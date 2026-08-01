#include "engine/resources/Texture.hpp"
#include <algorithm>
#include <assimp/Importer.hpp>
#include <assimp/material.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <engine/graphics/OpenGL.hpp>
#include <engine/resources/ResourcesController.hpp>
#include <engine/resources/ShaderCompiler.hpp>
#include <engine/util/Configuration.hpp>
#include <engine/util/Errors.hpp>
#include <spdlog/spdlog.h>
#include <utility>

namespace engine::resources {

void ResourcesController::initialize() {
    load_shaders();
    load_models();
    load_textures();
    load_skyboxes();
}

void ResourcesController::terminate() {
    for (auto &[name, resource]: m_models) {
        resource->destroy();
    }
    for (auto &[name, resource]: m_shaders) {
        resource->destroy();
    }
    for (auto &[name, resource]: m_textures) {
        resource->destroy();
    }
    for (auto &[name, resource]: m_sky_boxes) {
        resource->destroy();
    }
}


void ResourcesController::load_shaders() {
    if (!exists(m_shaders_path)) {
        spdlog::info("[ResourcesController]: no {} found to load the shaders from", m_shaders_path.string());
        return;
    }
    for (const auto &shader_path: std::filesystem::directory_iterator(m_shaders_path)) {
        const auto name = shader_path.path().stem().string();
        shader(name, shader_path);
    }
}

void ResourcesController::load_models() {
    if (!exists(m_models_path)) {
        spdlog::info("[ResourcesController]: no {} found to load the models from", m_models_path.string());
        return;
    }
    const auto &config = util::Configuration::config();
    if (!config.contains("resources") || !config["resources"].contains("models")) {
        std::string msg = "No configuration for models in the config.json, please provide the resources config. See the example in the README.md";
        throw util::EngineError(util::EngineError::Type::ConfigurationError, msg);
    }
    for (const auto &model_entry: config["resources"]["models"].items()) {
        model(model_entry.key());
    }
}

void ResourcesController::load_textures() {
    if (!exists(m_textures_path)) {
        spdlog::info("[ResourcesController]: no {} found to load the textures from", m_textures_path.string());
        return;
    }
    for (const auto &texture_entry: std::filesystem::directory_iterator(m_textures_path)) {
        texture(texture_entry.path().stem().string(), texture_entry.path());
    }
}

void ResourcesController::load_skyboxes() {
    if (!exists(m_skyboxes_path)) {
        spdlog::info("[ResourcesController]: no {} found to load the skyboxes from", m_skyboxes_path.string());
        return;
    }
    for (const auto &sky_boxes_entry: std::filesystem::directory_iterator(m_skyboxes_path)) {
        skybox(sky_boxes_entry.path().stem().string(), sky_boxes_entry.path());
    }
}

/**
 * @class AssimpSceneProcessor
 * @brief Processes the meshes in an Assimp scene.
 */
class AssimpSceneProcessor {
public:
    /**
     * @brief Processes the meshes in the scene.
     * @returns The meshes in the scene.
     */
    std::vector<Mesh> process_meshes();

    explicit AssimpSceneProcessor(ResourcesController *resources_controller, const aiScene *scene, std::filesystem::path model_path)
        : m_scene(scene)
        , m_model_path(std::move(model_path))
        , m_resources_controller(resources_controller) {
    }

private:
    void process_node(const aiNode *node, const aiMatrix4x4 &parent_transform);

    void process_mesh(aiMesh *mesh, const aiMatrix4x4 &transform);

    std::vector<MeshTexture> process_materials(const aiMaterial *material);

    void process_material_type(std::vector<MeshTexture> &textures, const aiMaterial *material, aiTextureType type);

    static TextureType assimp_texture_type_to_engine(aiTextureType type);

    std::vector<Mesh> m_meshes;
    const aiScene *m_scene;
    std::filesystem::path m_model_path;
    ResourcesController *m_resources_controller;
};

Model *ResourcesController::model(const std::string &name) {
    auto &result = m_models[name];
    if (!result) {
        auto &config = util::Configuration::config();
        if (!config["resources"]["models"].contains(name)) {
            std::string msg = std::format("No model ({}) specify in config.json. Please add the model to the config.json.", name);
            throw util::EngineError(util::EngineError::Type::ConfigurationError, msg);
        }
        std::filesystem::path model_path = m_models_path / std::filesystem::path(config["resources"]["models"][name]["path"].get<std::string>());
        Assimp::Importer importer;
        int flags = aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace;
        if (config["resources"]["models"][name].value<bool>("flip_uvs", false)) {
            flags |= aiProcess_FlipUVs;
        }

        spdlog::info("load_model(name={}, path={})", name, model_path.string());
        const aiScene *scene = importer.ReadFile(model_path, flags);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            std::string msg = std::format("Assimp error while reading model: {} from path {}.", model_path.string(), name);
            throw util::EngineError(util::EngineError::Type::AssetLoadingError, msg);
        }
        AssimpSceneProcessor scene_processor(this, scene, model_path);
        std::vector<Mesh> meshes = scene_processor.process_meshes();
        result = std::make_unique<Model>(Model(std::move(meshes), model_path, name));
    }
    return result.get();
}

Texture *ResourcesController::texture(const std::string &name, const std::filesystem::path &path, TextureType type, bool flip_uvs, bool srgb) {
    std::string cache_key = std::format("{}_{}{}", name, Texture::uniform_name_convention(type), srgb ? "_srgb" : "");
    auto &result = m_textures[cache_key];
    if (!result) {
        spdlog::info("'{}' = texture(path={}, srgb={})", path.stem().string(), path.string(), srgb);
        auto texture = graphics::OpenGL::generate_texture(path, flip_uvs, srgb);
        result = std::make_unique<Texture>(Texture(texture, type, path, path.stem()));
    }
    return result.get();
}

Texture *ResourcesController::normal_from_height(const std::string &name, const std::filesystem::path &path, bool flip_uvs, float strength) {
    auto &result = m_textures[name];
    if (!result) {
        spdlog::info("'{}' = load_height_to_normal(path={}, strength={})", path.stem().string(), path.string(), strength);
        auto texture_id = graphics::OpenGL::generate_normal_from_height(path, flip_uvs, strength);
        result = std::make_unique<Texture>(Texture(texture_id, TextureType::Normal, path, path.stem()));
    }
    return result.get();
}

Texture *ResourcesController::color_texture(const std::string &name, uint8_t r, uint8_t g, uint8_t b, TextureType type) {
    auto &result = m_textures[name];
    if (!result) {
        spdlog::info("'{}' = color_texture({}, {}, {})", name, r, g, b);
        auto texture_id = graphics::OpenGL::generate_color_texture(r, g, b);
        result = std::make_unique<Texture>(Texture(texture_id, type, "", name));
    }
    return result.get();
}

Skybox *ResourcesController::skybox(const std::string &name, const std::filesystem::path &path, bool flip_uvs) {
    auto &result = m_sky_boxes[name];
    if (!result) {
        spdlog::info("load_skybox(path={})", path.string());
        auto skybox = graphics::OpenGL::init_skybox_cube();
        auto textures = graphics::OpenGL::load_skybox_textures(path, flip_uvs);
        result = std::make_unique<Skybox>(Skybox(skybox, textures, path, name));
    }
    return result.get();
}

Shader *ResourcesController::shader(const std::string &name, const std::filesystem::path &path) {
    auto &result = m_shaders[name];
    if (!result) {
        spdlog::info("load_shader(path={})", path.string());
        result = std::make_unique<Shader>(ShaderCompiler::compile_from_file(name, path));
    }
    return result.get();
}

std::vector<Mesh> AssimpSceneProcessor::process_meshes() {
    m_meshes.clear();
    process_node(m_scene->mRootNode, aiMatrix4x4());
    return std::move(m_meshes);
}

void AssimpSceneProcessor::process_node(const aiNode *node, const aiMatrix4x4 &parent_transform) {
    aiMatrix4x4 transform = parent_transform * node->mTransformation;
    for (uint32_t i = 0; i < node->mNumMeshes; ++i) {
        auto mesh = m_scene->mMeshes[node->mMeshes[i]];
        process_mesh(mesh, transform);
    }
    for (uint32_t i = 0; i < node->mNumChildren; ++i) {
        process_node(node->mChildren[i], transform);
    }
}

void AssimpSceneProcessor::process_mesh(aiMesh *mesh, const aiMatrix4x4 &transform) {
    aiMatrix3x3 normal_matrix(transform);
    normal_matrix.Inverse().Transpose();

    std::vector<Vertex> vertices;
    vertices.reserve(mesh->mNumVertices);
    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
        Vertex vertex{};
        aiVector3D pos = transform * mesh->mVertices[i];
        vertex.Position.x = pos.x;
        vertex.Position.y = pos.y;
        vertex.Position.z = pos.z;

        if (mesh->HasNormals()) {
            aiVector3D norm = normal_matrix * mesh->mNormals[i];
            norm.Normalize();
            vertex.Normal.x = norm.x;
            vertex.Normal.y = norm.y;
            vertex.Normal.z = norm.z;
        }

        if (mesh->mTextureCoords[0]) {
            vertex.TexCoords.x = mesh->mTextureCoords[0][i].x;
            vertex.TexCoords.y = mesh->mTextureCoords[0][i].y;

            aiVector3D tan = normal_matrix * mesh->mTangents[i];
            tan.Normalize();
            vertex.Tangent.x = tan.x;
            vertex.Tangent.y = tan.y;
            vertex.Tangent.z = tan.z;

            aiVector3D bitan = normal_matrix * mesh->mBitangents[i];
            bitan.Normalize();
            vertex.Bitangent.x = bitan.x;
            vertex.Bitangent.y = bitan.y;
            vertex.Bitangent.z = bitan.z;
        } else if (mesh->HasNormals()) {
            glm::vec3 n = glm::normalize(glm::vec3(vertex.Normal.x, vertex.Normal.y, vertex.Normal.z));
            glm::vec3 up = std::abs(n.y) < 0.999f ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0);
            glm::vec3 t = glm::normalize(glm::cross(up, n));
            glm::vec3 b = glm::cross(n, t);
            vertex.Tangent = {t.x, t.y, t.z};
            vertex.Bitangent = {b.x, b.y, b.z};
        }

        if (mesh->mTextureCoords[1]) {
            vertex.TexCoords2.x = mesh->mTextureCoords[1][i].x;
            vertex.TexCoords2.y = mesh->mTextureCoords[1][i].y;
        } else if (mesh->mTextureCoords[0]) {
            vertex.TexCoords2 = vertex.TexCoords;
        }
        vertices.push_back(vertex);
    }

    std::vector<uint32_t> indices;
    for (uint32_t i = 0; i < mesh->mNumFaces; ++i) {
        aiFace face = mesh->mFaces[i];

        for (uint32_t j = 0; j < face.mNumIndices; ++j) {
            indices.push_back(face.mIndices[j]);
        }
    }

    auto material = m_scene->mMaterials[mesh->mMaterialIndex];
    std::vector<MeshTexture> textures = process_materials(material);

    aiColor3D emissive_color(0.0f, 0.0f, 0.0f);
    material->Get(AI_MATKEY_COLOR_EMISSIVE, emissive_color);
    glm::vec3 emissive_factor(emissive_color.r, emissive_color.g, emissive_color.b);

    bool has_emissive_texture = material->GetTextureCount(aiTextureType_EMISSIVE) > 0;
    if (has_emissive_texture && emissive_factor == glm::vec3(0.0f)) {
        emissive_factor = glm::vec3(1.0f);
    }

    float shininess = 32.0f;
    material->Get(AI_MATKEY_SHININESS, shininess);

    float opacity = 1.0f;
    material->Get(AI_MATKEY_OPACITY, opacity);

    aiColor3D diffuse_color(1.0f, 1.0f, 1.0f);
    material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse_color);
    glm::vec3 diffuse_factor(diffuse_color.r, diffuse_color.g, diffuse_color.b);

    aiString mat_name;
    material->Get(AI_MATKEY_NAME, mat_name);
    spdlog::info("Material '{}': diffuse=({},{},{}), opacity={}, emissive=({},{},{})",
                 mat_name.C_Str(),
                 diffuse_color.r, diffuse_color.g, diffuse_color.b,
                 opacity,
                 emissive_color.r, emissive_color.g, emissive_color.b);

    m_meshes.emplace_back(Mesh(vertices, indices, std::move(textures), emissive_factor, shininess, opacity, diffuse_factor));
}

std::vector<MeshTexture> AssimpSceneProcessor::process_materials(const aiMaterial *material) {
    std::vector<MeshTexture> textures;
    auto ai_texture_types = {
            aiTextureType_DIFFUSE,
            aiTextureType_SPECULAR,
            aiTextureType_NORMALS,
            aiTextureType_HEIGHT,
            aiTextureType_EMISSIVE,
    };

    for (auto ai_texture_type: ai_texture_types) {
        process_material_type(textures, material, ai_texture_type);
    }

    bool has_diffuse = std::ranges::any_of(textures, [](const auto &mt) {
        return mt.texture->type() == TextureType::Diffuse;
    });

    if (!has_diffuse) {
        Texture *texture = m_resources_controller->color_texture("_white_diffuse", 255, 255, 255);
        textures.insert(textures.begin(), {texture, 0});
    }

    bool has_normal = material->GetTextureCount(aiTextureType_NORMALS) > 0 ||
                      material->GetTextureCount(aiTextureType_HEIGHT) > 0;
    bool has_specular = material->GetTextureCount(aiTextureType_SPECULAR) > 0;
    bool has_emissive = material->GetTextureCount(aiTextureType_EMISSIVE) > 0;
    if (!has_normal) {
        textures.push_back({m_resources_controller->color_texture(
                                    "_flat_normal", 128, 128, 255, TextureType::Normal),
                            0});
    }
    if (!has_specular) {
        aiColor3D spec_color(0.0f, 0.0f, 0.0f);
        material->Get(AI_MATKEY_COLOR_SPECULAR, spec_color);
        aiString mat_name;
        material->Get(AI_MATKEY_NAME, mat_name);
        std::string name = std::string("_generated_specular_") + mat_name.C_Str();
        textures.push_back({m_resources_controller->color_texture(name,
                                                                  static_cast<uint8_t>(spec_color.r * 255),
                                                                  static_cast<uint8_t>(spec_color.g * 255),
                                                                  static_cast<uint8_t>(spec_color.b * 255), TextureType::Specular),
                            0});
    }
    if (!has_emissive) {
        textures.push_back({m_resources_controller->color_texture(
                                    "_no_emission", 0, 0, 0, TextureType::Emissive),
                            0});
    }

    for (auto &mt: textures) {
        spdlog::info("set texture '{}' with index {} for '{}' as '{}'", mt.texture->name(), mt.uv_index, material->GetName().C_Str(), Texture::uniform_name_convention(mt.texture->type()));
    }

    return textures;
}

void AssimpSceneProcessor::process_material_type(std::vector<MeshTexture> &textures, const aiMaterial *material, aiTextureType type) {
    auto material_count = material->GetTextureCount(type);
    if (material_count > 0) {
        aiString mat_name;
        material->Get(AI_MATKEY_NAME, mat_name);
    }
    for (uint32_t i = 0; i < material_count; ++i) {
        aiString ai_texture_path_string;
        unsigned int uv_index = 0;
        material->GetTexture(type, i, &ai_texture_path_string, nullptr, &uv_index);
        std::filesystem::path texture_path = m_model_path.parent_path() / ai_texture_path_string.C_Str();
        Texture *texture;
        if (type == aiTextureType_HEIGHT) {
            float bump_strength = 4.0f;
            material->Get(AI_MATKEY_BUMPSCALING, bump_strength);
            texture = m_resources_controller->normal_from_height(texture_path.string(), texture_path, false, bump_strength);
        } else {
            bool srgb = (type == aiTextureType_DIFFUSE || type == aiTextureType_EMISSIVE);
            texture = m_resources_controller->texture(texture_path.string(), texture_path, assimp_texture_type_to_engine(type), false, srgb);
        }
        textures.push_back({texture, uv_index});
    }
}

TextureType AssimpSceneProcessor::assimp_texture_type_to_engine(aiTextureType type) {
    switch (type) {
        case aiTextureType_DIFFUSE: return TextureType::Diffuse;
        case aiTextureType_SPECULAR: return TextureType::Specular;
        // height maps get converted to normals
        case aiTextureType_HEIGHT: return TextureType::Normal;
        case aiTextureType_NORMALS: return TextureType::Normal;
        case aiTextureType_EMISSIVE: return TextureType::Emissive;
        default: RG_SHOULD_NOT_REACH_HERE("Engine currently doesn't support the aiTextureType: {}", static_cast<int>(type));
    }
}

}// namespace engine::resources
