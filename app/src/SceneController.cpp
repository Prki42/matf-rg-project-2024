#include "SceneController.hpp"

namespace app {

Renderable &SceneController::add_renderable(engine::resources::Model *model, const glm::mat4 &transform) {
    return m_renderables.emplace_back(Renderable{model, transform});
}

void SceneController::render_all(const engine::resources::Shader *shader) {
    for (auto &r : m_renderables) {
        shader->set_mat4("model", r.transform);
        r.model->draw(shader);
    }
}

}// namespace app
