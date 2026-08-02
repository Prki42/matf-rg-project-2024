#ifndef ENGINE_SCENECONTROLLER_HPP
#define ENGINE_SCENECONTROLLER_HPP

#include <engine/core/Controller.hpp>
#include <engine/resources/Model.hpp>
#include <engine/resources/Shader.hpp>
#include <glm/glm.hpp>
#include <vector>

namespace engine::graphics {

struct Renderable {
    engine::resources::Model *model;
    glm::mat4 transform{1.0f};
    engine::resources::Shader *shader{nullptr};
    bool visible{true};
};

class SceneController final : public engine::core::Controller {
public:
    std::string_view name() const override {
        return "SceneController";
    }

    Renderable *add_renderable(engine::resources::Model *model, const glm::mat4 &transform = glm::mat4(1.0f), engine::resources::Shader *shader = nullptr);

    void render_all(const engine::resources::Shader *fallback_shader, bool allow_custom_shader = true);

    std::vector<std::unique_ptr<Renderable>> &renderables() { return m_renderables; }
    const std::vector<std::unique_ptr<Renderable>> &renderables() const { return m_renderables; }

private:
    std::vector<std::unique_ptr<Renderable>> m_renderables;
};

}// namespace engine::graphics
#endif//ENGINE_SCENECONTROLLER_HPP
