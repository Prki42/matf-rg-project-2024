/**
 * @file Mesh.hpp
 * @brief Defines the Mesh class that serves as the interface for mesh rendering and storing processed assimp scenes.
 */

#ifndef MATF_RG_PROJECT_MESH_HPP
#define MATF_RG_PROJECT_MESH_HPP

#include <engine/resources/Texture.hpp>
#include <glm/glm.hpp>
#include <vector>

namespace engine::resources {
/**
* @struct Vertex
* @brief Represents a vertex in the mesh.
*/
struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec2 TexCoords2;

    glm::vec3 Tangent;
    glm::vec3 Bitangent;
};

/**
* @struct MeshTexture
* @brief Represents a texture used for the mesh
*/
struct MeshTexture {
    Texture *texture;
    uint32_t uv_index;
};

/**
* @class Mesh
* @brief Represents a mesh in the model in the OpenGL context.
*/
class Mesh {
    friend class AssimpSceneProcessor;

public:
    /**
    * @brief Draws the mesh using a given shader. Called by the @ref Model::draw function to draw all the meshes in the model.
    * @param shader The shader to use for drawing.
    */
    void draw(const Shader *shader);

    bool is_transparent() const { return m_opacity < 1.0f; }

    /**
    * @brief Destroys the mesh in the OpenGL context.
    */
    void destroy();

private:
    /**
    * @brief Constructs a Mesh object.
    * @param vertices The vertices in the mesh.
    * @param indices The indices in the mesh.
    * @param textures The textures in the mesh.
    * @param emissive_factor The emissive factor for the mesh.
    * @param shininess The shininess of the mesh.
    */
    Mesh(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices,
         std::vector<MeshTexture> textures, glm::vec3 emissive_factor = glm::vec3(0.0f), float shininess = 32.0f,
         float opacity = 1.0f, glm::vec3 diffuse_factor = glm::vec3(1.0f));

    uint32_t m_vao{0};
    uint32_t m_num_indices{0};
    std::vector<MeshTexture> m_textures;
    glm::vec3 m_emissive_factor{0.0f};
    float m_shininess{32.0f};
    float m_opacity{1.0f};
    glm::vec3 m_diffuse_factor{1.0f};
};
}// namespace engine::resources

#endif//MATF_RG_PROJECT_MESH_HPP
