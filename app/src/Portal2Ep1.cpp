#include "Portal2Ep1.hpp"
#include "MainController.hpp"
#include <engine/core/App.hpp>
#include <engine/core/Controller.hpp>
#include <memory>

namespace app {

void Portal2Ep1::app_setup() {
    auto main_controller = register_controller<MainController>();
    main_controller->after(engine::core::Controller::get<engine::core::EngineControllersEnd>());
}

}// namespace app

int main(int argc, char **argv) {
    auto p = std::make_unique<app::Portal2Ep1>();
    return p->run(argc, argv);
}
