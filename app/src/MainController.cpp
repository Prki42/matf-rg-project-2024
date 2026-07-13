#include "MainController.hpp"
#include <engine/core/Engine.hpp>
#include <engine/graphics/GraphicsController.hpp>
#include <engine/graphics/OpenGL.hpp>
#include <engine/platform/PlatformController.hpp>
#include <memory>
#include <spdlog/spdlog.h>


namespace app {

void MainPlatformEventObserver::on_key(engine::platform::Key key) {
    // spdlog::info("Keyboard event: key={}, state={}", key.name(), key.state_str());
}

void MainController::initialize() {
    engine::graphics::OpenGL::enable_depth_testing();

    auto observer = std::make_unique<MainPlatformEventObserver>();
    engine::core::Controller::get<engine::platform::PlatformController>()->register_platform_event_observer(
            std::move(observer));

    auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();
    graphics->set_bloom(true);
    graphics->set_exposure(1.0f);
}

bool MainController::loop() {
    const auto platform = engine::core::Controller::get<engine::platform::PlatformController>();
    if (platform->key(engine::platform::KeyId::KEY_Q).state() == engine::platform::Key::State::JustPressed) {
        return false;
    }
    return true;
}

void MainController::poll_events() {
    const auto platform = engine::core::Controller::get<engine::platform::PlatformController>();
    if (platform->key(engine::platform::KEY_F1).state() == engine::platform::Key::State::JustPressed) {
        m_cursor_enabled = !m_cursor_enabled;
        platform->set_enable_cursor(m_cursor_enabled);
    }
}

void MainController::update() {
    update_camera();
}

void MainController::draw() {
    draw_room();
}

void MainController::draw_room() {
    auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();
    auto shader = engine::core::Controller::get<engine::resources::ResourcesController>()->shader("basic");
    auto room = engine::core::Controller::get<engine::resources::ResourcesController>()->model("room");
    auto cube = engine::core::Controller::get<engine::resources::ResourcesController>()->model("cube");
    auto moron = engine::core::Controller::get<engine::resources::ResourcesController>()->model("wheatley");

    shader->use();

    auto model = glm::mat4(1.0f);

    shader->set_mat4("projection", graphics->projection_matrix());
    shader->set_mat4("view", graphics->camera()->view_matrix());
    shader->set_mat4("model", model);
    shader->set_vec3("viewPos", graphics->camera()->Position);

    shader->set_int("numPointLights", 2);
    shader->set_vec3("pointLights[0].position", glm::vec3(0.0f, 4.0f, 0.0f));
    shader->set_vec3("pointLights[0].color", glm::vec3(0.6f, 0.6f, 1.0f));
    shader->set_float("pointLights[0].constant", 1.0f);
    shader->set_float("pointLights[0].linear", 0.009f);
    shader->set_float("pointLights[0].quadratic", 0.032f);

    shader->set_vec3("pointLights[1].position", glm::vec3(3.0f, 2.0f, 10.0f));
    shader->set_vec3("pointLights[1].color", glm::vec3(1.0f, 0.5f, 0.2f));
    shader->set_float("pointLights[1].constant", 1.0f);
    shader->set_float("pointLights[1].linear", 0.009f);
    shader->set_float("pointLights[1].quadratic", 0.032f);

    shader->set_int("numSpotLights", 1);
    shader->set_vec3("spotLights[0].position", glm::vec3(0.0f, 5.0f, 0.0f));
    shader->set_vec3("spotLights[0].direction", glm::vec3(0.0f, -1.0f, 0.0f));
    shader->set_vec3("spotLights[0].color", glm::vec3(1.0f, 1.0f, 1.0f));
    shader->set_float("spotLights[0].cutOff", glm::cos(glm::radians(15.0f)));
    shader->set_float("spotLights[0].outerCutOff", glm::cos(glm::radians(20.0f)));
    shader->set_float("spotLights[0].constant", 1.0f);
    shader->set_float("spotLights[0].linear", 0.09f);
    shader->set_float("spotLights[0].quadratic", 0.032f);

    room->draw(shader);

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(1.0f, 0.7f, 2.0f));
    shader->set_mat4("model", model);
    cube->draw(shader);

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(1.0f, 0.7f, 4.0f));
    model = glm::scale(model, glm::vec3(0.05));
    shader->set_mat4("model", model);
    moron->draw(shader);
}

void MainController::update_camera() {
    auto platform = engine::core::Controller::get<engine::platform::PlatformController>();
    auto camera = engine::core::Controller::get<engine::graphics::GraphicsController>()->camera();
    float dt = platform->dt();
    if (platform->key(engine::platform::KEY_W).state() == engine::platform::Key::State::Pressed) {
        camera->move_camera(engine::graphics::Camera::Movement::FORWARD, dt);
    }
    if (platform->key(engine::platform::KEY_S).state() == engine::platform::Key::State::Pressed) {
        camera->move_camera(engine::graphics::Camera::Movement::BACKWARD, dt);
    }
    if (platform->key(engine::platform::KEY_A).state() == engine::platform::Key::State::Pressed) {
        camera->move_camera(engine::graphics::Camera::Movement::LEFT, dt);
    }
    if (platform->key(engine::platform::KEY_D).state() == engine::platform::Key::State::Pressed) {
        camera->move_camera(engine::graphics::Camera::Movement::RIGHT, dt);
    }
    if (platform->key(engine::platform::KEY_SPACE).state() == engine::platform::Key::State::Pressed) {
        camera->move_camera(engine::graphics::Camera::Movement::UP, dt);
    }
    if (platform->key(engine::platform::KEY_LEFT_SHIFT).state() == engine::platform::Key::State::Pressed) {
        camera->move_camera(engine::graphics::Camera::Movement::DOWN, dt);
    }
    auto mouse = platform->mouse();
    if (!m_cursor_enabled) {
        camera->rotate_camera(mouse.dx, mouse.dy);
    }
    camera->zoom(mouse.scroll);
}
}// namespace app
