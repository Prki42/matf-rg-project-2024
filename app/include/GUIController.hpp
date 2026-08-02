#ifndef GUICONTROLLER_HPP
#define GUICONTROLLER_HPP

#include <engine/core/Controller.hpp>

namespace app {
class GUIController final : public engine::core::Controller {
public:
    std::string_view name() const override {
        return "GUIController";
    }

private:
    void initialize() override;
    void poll_events() override;
    void end_draw() override;
};
}// namespace app
#endif//GUICONTROLLER_HPP
