#ifndef LIGHTCONTROLLER_HPP
#define LIGHTCONTROLLER_HPP

#include <engine/core/Controller.hpp>
#include <engine/resources/Shader.hpp>
#include <glm/glm.hpp>
#include <vector>

namespace app {

struct PointLight {
    glm::vec3 position{0.0f};
    glm::vec3 color{1.0f};
    float constant = 1.0f;
    float linear = 0.09f;
    float quadratic = 0.032f;
};

struct SpotLight {
    glm::vec3 position{0.0f};
    glm::vec3 direction{0.0f, -1.0f, 0.0f};
    glm::vec3 color{1.0f};
    float cutOff = glm::cos(glm::radians(12.5f));
    float outerCutOff = glm::cos(glm::radians(15.0f));
    float constant = 1.0f;
    float linear = 0.09f;
    float quadratic = 0.032f;
};

class LightController final : public engine::core::Controller {
public:
    std::string_view name() const override {
        return "LightController";
    }

    PointLight &add_point_light();
    SpotLight &add_spot_light();

    std::vector<PointLight> &point_lights() { return m_point_lights; }
    std::vector<SpotLight> &spot_lights() { return m_spot_lights; }

    void apply(const engine::resources::Shader *shader) const;

private:
    std::vector<PointLight> m_point_lights;
    std::vector<SpotLight> m_spot_lights;
};

}// namespace app
#endif//LIGHTCONTROLLER_HPP
