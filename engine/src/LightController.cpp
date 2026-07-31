#include <engine/graphics/GraphicsController.hpp>
#include <engine/graphics/LightController.hpp>
#include <engine/graphics/OpenGL.hpp>
#include <engine/graphics/SceneController.hpp>
#include <engine/resources/ResourcesController.hpp>
#include <format>
#include <glm/gtc/matrix_transform.hpp>

namespace engine::graphics {

PointLight &LightController::add_point_light() {
    return m_point_lights.emplace_back();
}

SpotLight &LightController::add_spot_light() {
    return m_spot_lights.emplace_back();
}

void LightController::setup_shadow_maps() {
    int needed = 0;
    for (auto &l: m_point_lights) {
        if (l.casts_shadows)
            ++needed;
    }

    while (static_cast<int>(m_shadow_maps.size()) < needed) {
        ShadowMap sm;
        sm.cubemap = OpenGL::create_depth_cubemap(m_shadow_resolution);
        sm.fbo = OpenGL::create_depth_cubemap_fbo(sm.cubemap);
        m_shadow_maps.push_back(sm);
    }
}

void LightController::setup_spot_shadow_maps() {
    int needed = 0;
    for (auto &l: m_spot_lights) {
        if (l.casts_shadows)
            ++needed;
    }

    while (static_cast<int>(m_spot_shadow_maps.size()) < needed) {
        SpotShadowMap sm;
        sm.texture = OpenGL::create_depth_texture(m_shadow_resolution);
        sm.fbo = OpenGL::create_depth_texture_fbo(sm.texture);
        m_spot_shadow_maps.push_back(sm);
    }
}

void LightController::begin_draw() {
    setup_shadow_maps();
    setup_spot_shadow_maps();

    auto scene = engine::core::Controller::get<SceneController>();
    auto resources = engine::core::Controller::get<engine::resources::ResourcesController>();

    uint32_t prev_fbo = OpenGL::current_framebuffer();
    int prev_viewport[4];
    OpenGL::current_viewport(prev_viewport);

    OpenGL::cull_front_faces();

    auto depth_shader = resources->shader("depth");
    float aspect = 1.0f;
    float near = 0.1f;

    int shadow_idx = 0;
    for (auto &light: m_point_lights) {
        if (!light.casts_shadows)
            continue;

        auto &sm = m_shadow_maps[shadow_idx++];
        glm::mat4 shadow_proj = glm::perspective(glm::radians(90.0f), aspect, near, light.shadow_far);
        glm::vec3 pos = light.position;

        glm::mat4 shadow_transforms[6] = {
                shadow_proj * glm::lookAt(pos, pos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
                shadow_proj * glm::lookAt(pos, pos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
                shadow_proj * glm::lookAt(pos, pos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
                shadow_proj * glm::lookAt(pos, pos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
                shadow_proj * glm::lookAt(pos, pos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
                shadow_proj * glm::lookAt(pos, pos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
        };

        OpenGL::set_viewport(0, 0, m_shadow_resolution, m_shadow_resolution);
        OpenGL::bind_framebuffer(sm.fbo);
        OpenGL::clear_depth_buffer();

        depth_shader->use();
        for (int i = 0; i < 6; ++i) {
            depth_shader->set_mat4(std::format("shadowMatrices[{}]", i), shadow_transforms[i]);
        }
        depth_shader->set_vec3("lightPos", pos);
        depth_shader->set_float("far_plane", light.shadow_far);

        scene->render_all(depth_shader);
    }

    auto depth_spot_shader = resources->shader("depth_spot");
    int spot_shadow_idx = 0;
    for (auto &light: m_spot_lights) {
        if (!light.casts_shadows)
            continue;

        auto &sm = m_spot_shadow_maps[spot_shadow_idx++];

        float fov = 2.0f * glm::acos(light.outerCutOff);
        glm::mat4 light_proj = glm::perspective(fov, 1.0f, 0.1f, light.shadow_far);

        glm::vec3 up = glm::abs(glm::dot(light.direction, glm::vec3(0, 1, 0))) > 0.99f
                               ? glm::vec3(0, 0, 1)
                               : glm::vec3(0, 1, 0);
        glm::mat4 light_view = glm::lookAt(light.position, light.position + light.direction, up);
        sm.light_space_matrix = light_proj * light_view;

        OpenGL::set_viewport(0, 0, m_shadow_resolution, m_shadow_resolution);
        OpenGL::bind_framebuffer(sm.fbo);
        OpenGL::clear_depth_buffer();

        depth_spot_shader->use();
        depth_spot_shader->set_mat4("lightSpaceMatrix", sm.light_space_matrix);

        scene->render_all(depth_spot_shader);
    }

    OpenGL::cull_back_faces();

    OpenGL::bind_framebuffer(prev_fbo);
    OpenGL::set_viewport(prev_viewport[0], prev_viewport[1], prev_viewport[2], prev_viewport[3]);
}

void LightController::draw() {
    if (!m_draw_debug) return;

    auto graphics = engine::core::Controller::get<GraphicsController>();
    auto resources = engine::core::Controller::get<engine::resources::ResourcesController>();
    auto shader = resources->shader("light_debug");
    auto cube = resources->model("cube");

    shader->use();
    shader->set_mat4("projection", graphics->projection_matrix());
    shader->set_mat4("view", graphics->camera()->view_matrix());

    for (auto &pl: m_point_lights) {
        auto model = glm::scale(glm::translate(glm::mat4(1.0f), pl.position), glm::vec3(0.1f));
        shader->set_mat4("model", model);
        shader->set_vec3("lightColor", pl.color);
        cube->draw(shader);
    }
    for (auto &sl: m_spot_lights) {
        auto model = glm::scale(glm::translate(glm::mat4(1.0f), sl.position), glm::vec3(0.1f));
        shader->set_mat4("model", model);
        shader->set_vec3("lightColor", sl.color);
        cube->draw(shader);
    }
}

void LightController::apply(const engine::resources::Shader *shader) const {
    for (int i = 0; i < 8; ++i) {
        shader->set_int(std::format("pointShadowMaps[{}]", i), SHADOW_MAP_BASE_UNIT + i);
    }
    for (int i = 0; i < 4; ++i) {
        shader->set_int(std::format("spotShadowMaps[{}]", i), SPOT_SHADOW_MAP_BASE_UNIT + i);
    }

    shader->set_int("numPointLights", static_cast<int>(m_point_lights.size()));
    int shadow_idx = 0;
    for (int i = 0; i < static_cast<int>(m_point_lights.size()); ++i) {
        auto prefix = std::format("pointLights[{}].", i);
        const auto &l = m_point_lights[i];
        shader->set_vec3(prefix + "position", l.position);
        shader->set_vec3(prefix + "color", l.color);
        shader->set_float(prefix + "constant", l.constant);
        shader->set_float(prefix + "linear", l.linear);
        shader->set_float(prefix + "quadratic", l.quadratic);
        shader->set_int(prefix + "castsShadows", l.casts_shadows ? 1 : 0);
        shader->set_float(prefix + "shadowFar", l.shadow_far);

        if (l.casts_shadows && shadow_idx < static_cast<int>(m_shadow_maps.size())) {
            OpenGL::bind_texture_cube_map(SHADOW_MAP_BASE_UNIT + i, m_shadow_maps[shadow_idx].cubemap);
            ++shadow_idx;
        }
    }

    shader->set_int("numSpotLights", static_cast<int>(m_spot_lights.size()));
    int spot_shadow_idx = 0;
    for (int i = 0; i < static_cast<int>(m_spot_lights.size()); ++i) {
        auto prefix = std::format("spotLights[{}].", i);
        const auto &l = m_spot_lights[i];
        shader->set_vec3(prefix + "position", l.position);
        shader->set_vec3(prefix + "direction", l.direction);
        shader->set_vec3(prefix + "color", l.color);
        shader->set_float(prefix + "cutOff", l.cutOff);
        shader->set_float(prefix + "outerCutOff", l.outerCutOff);
        shader->set_float(prefix + "constant", l.constant);
        shader->set_float(prefix + "linear", l.linear);
        shader->set_float(prefix + "quadratic", l.quadratic);
        shader->set_int(prefix + "castsShadows", l.casts_shadows ? 1 : 0);

        if (l.casts_shadows && spot_shadow_idx < static_cast<int>(m_spot_shadow_maps.size())) {
            OpenGL::bind_texture_2d(SPOT_SHADOW_MAP_BASE_UNIT + i,
                                    m_spot_shadow_maps[spot_shadow_idx].texture);
            shader->set_mat4(prefix + "lightSpaceMatrix", m_spot_shadow_maps[spot_shadow_idx].light_space_matrix);
            ++spot_shadow_idx;
        }
    }
}

}// namespace engine::graphics
