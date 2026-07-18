#include "SceneController.hpp"

namespace app {

Renderable &SceneController::add_renderable(engine::resources::Model *model, const glm::mat4 &transform) {
    return m_renderables.emplace_back(Renderable{model, transform});
}

}// namespace app
