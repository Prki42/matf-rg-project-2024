#include "Portal2Ep1.hpp"
#include "LightController.hpp"
#include "MainController.hpp"
#include "PostProcessController.hpp"
#include "SceneController.hpp"
#include <engine/core/App.hpp>
#include <engine/core/Controller.hpp>
#include <memory>

namespace app {

void Portal2Ep1::app_setup() {
    auto scene = register_controller<SceneController>();
    auto lights = register_controller<LightController>();
    auto post_process = register_controller<PostProcessController>();
    auto main_controller = register_controller<MainController>();
    scene->after(engine::core::Controller::get<engine::core::EngineControllersEnd>());
    lights->after(scene);
    post_process->after(lights);
    main_controller->after(post_process);
}

}// namespace app

int main(int argc, char **argv) {
    auto p = std::make_unique<app::Portal2Ep1>();
    return p->run(argc, argv);
}
