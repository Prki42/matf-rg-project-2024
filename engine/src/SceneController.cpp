#include <engine/graphics/SceneController.hpp>
#include <memory>

namespace engine::graphics {

Renderable *SceneController::add_renderable(engine::resources::Model *model, const glm::mat4 &transform) {
    return m_renderables.emplace_back(std::make_unique<Renderable>(model, transform)).get();
}

void SceneController::render_all(const engine::resources::Shader *shader) {
    for (auto &r: m_renderables) {
        if (!r->visible) {
            continue;
        }
        shader->set_mat4("model", r->transform);
        r->model->draw(shader);
    }
}

}// namespace engine::graphics
