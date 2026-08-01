#include "engine/resources/Shader.hpp"
#include <engine/graphics/SceneController.hpp>
#include <memory>

namespace engine::graphics {

Renderable *SceneController::add_renderable(engine::resources::Model *model, const glm::mat4 &transform, engine::resources::Shader *shader) {
    return m_renderables.emplace_back(std::make_unique<Renderable>(model, transform, shader)).get();
}

void SceneController::render_all(const engine::resources::Shader *fallback_shader) {
    for (auto &r: m_renderables) {
        if (!r->visible) {
            continue;
        }
        auto *shader = r->shader ? r->shader : fallback_shader;
        shader->use();
        shader->set_mat4("model", r->transform);
        r->model->draw(shader);
    }
}

}// namespace engine::graphics
