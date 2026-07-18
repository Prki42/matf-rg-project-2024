#include "LightController.hpp"
#include <format>

namespace app {

PointLight &LightController::add_point_light() {
    return m_point_lights.emplace_back();
}

SpotLight &LightController::add_spot_light() {
    return m_spot_lights.emplace_back();
}

void LightController::apply(const engine::resources::Shader *shader) const {
    shader->set_int("numPointLights", static_cast<int>(m_point_lights.size()));
    for (int i = 0; i < static_cast<int>(m_point_lights.size()); ++i) {
        auto prefix = std::format("pointLights[{}].", i);
        const auto &l = m_point_lights[i];
        shader->set_vec3(prefix + "position", l.position);
        shader->set_vec3(prefix + "color", l.color);
        shader->set_float(prefix + "constant", l.constant);
        shader->set_float(prefix + "linear", l.linear);
        shader->set_float(prefix + "quadratic", l.quadratic);
    }

    shader->set_int("numSpotLights", static_cast<int>(m_spot_lights.size()));
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
    }
}

}// namespace app
