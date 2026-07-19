#ifndef SCENECONTROLLER_HPP
#define SCENECONTROLLER_HPP

#include <engine/core/Controller.hpp>
#include <engine/resources/Model.hpp>
#include <engine/resources/Shader.hpp>
#include <glm/glm.hpp>
#include <vector>

namespace app {

struct Renderable {
    engine::resources::Model *model;
    glm::mat4 transform{1.0f};
};

class SceneController final : public engine::core::Controller {
public:
    std::string_view name() const override {
        return "SceneController";
    }

    Renderable &add_renderable(engine::resources::Model *model, const glm::mat4 &transform = glm::mat4(1.0f));

    void render_all(const engine::resources::Shader *shader);

    std::vector<Renderable> &renderables() { return m_renderables; }
    const std::vector<Renderable> &renderables() const { return m_renderables; }

private:
    std::vector<Renderable> m_renderables;
};

}// namespace app
#endif//SCENECONTROLLER_HPP
