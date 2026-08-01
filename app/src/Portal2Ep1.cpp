#include "Portal2Ep1.hpp"
#include "GUIController.hpp"
#include "MainController.hpp"
#include <engine/core/App.hpp>
#include <engine/core/Controller.hpp>
#include <engine/graphics/LightController.hpp>
#include <engine/graphics/PostProcessController.hpp>
#include <engine/graphics/SceneController.hpp>
#include <memory>

namespace app {

void Portal2Ep1::app_setup() {
    auto scene = register_controller<engine::graphics::SceneController>();
    auto lights = register_controller<engine::graphics::LightController>();
    auto gui = register_controller<GUIController>();
    auto main_controller = register_controller<MainController>();
    auto post = register_controller<engine::graphics::PostProcessController>();

    scene->after(engine::core::Controller::get<engine::core::EngineControllersEnd>());
    scene->before(lights);
    lights->before(gui);
    gui->before(post);
    post->before(main_controller);
}

}// namespace app

int main(int argc, char **argv) {
    auto p = std::make_unique<app::Portal2Ep1>();
    return p->run(argc, argv);
}
