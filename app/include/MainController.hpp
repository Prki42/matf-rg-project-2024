#ifndef MAINCONTROLLER_HPP
#define MAINCONTROLLER_HPP

#include <engine/core/Controller.hpp>
#include <engine/platform/PlatformController.hpp>
#include <glm/glm.hpp>
#include <string_view>
#include <vector>

namespace app {
class MainController final : public engine::core::Controller {
public:
    std::string_view name() const override {
        return "app::MainController";
    }

private:
    void initialize() override;

    bool loop() override;

    void poll_events() override;

    void update() override;

    void draw() override;

    void begin_draw() override;

    void end_draw() override;

    void update_camera();

    int m_wheatley_light_index{-1};
    int m_wheatley_renderable_index{-1};
    bool m_wheatley_light_on{true};

    int m_temple_renderable_index{-1};
    int m_point3_index{-1};

    enum class EventState {
        IDLE,
        TEMPLE_WAIT,
        DIM_WAIT,
        RESTORE_WAIT
    };

    EventState m_event_state{EventState::IDLE};

    float m_event_timer{0.0f};
    std::vector<glm::vec3> m_saved_point_colors;
    std::vector<glm::vec3> m_saved_spot_colors;
};
}// namespace app

#endif
