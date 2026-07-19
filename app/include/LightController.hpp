#ifndef LIGHTCONTROLLER_HPP
#define LIGHTCONTROLLER_HPP

#include <cstdint>
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
    bool casts_shadows = false;
    float shadow_far = 25.0f;
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
    bool casts_shadows = false;
    float shadow_far = 25.0f;
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

    void set_shadow_resolution(int resolution) { m_shadow_resolution = resolution; }

    static constexpr int SHADOW_MAP_BASE_UNIT = 8;
    static constexpr int SPOT_SHADOW_MAP_BASE_UNIT = 16;

private:
    void begin_draw() override;

    void setup_shadow_maps();
    void setup_spot_shadow_maps();

    struct ShadowMap {
        uint32_t fbo = 0;
        uint32_t cubemap = 0;
    };

    struct SpotShadowMap {
        uint32_t fbo = 0;
        uint32_t texture = 0;
        glm::mat4 light_space_matrix{1.0f};
    };

    std::vector<PointLight> m_point_lights;
    std::vector<SpotLight> m_spot_lights;
    std::vector<ShadowMap> m_shadow_maps;
    std::vector<SpotShadowMap> m_spot_shadow_maps;
    int m_shadow_resolution = 1024;
};

}// namespace app
#endif//LIGHTCONTROLLER_HPP
