#include "MainController.hpp"
#include "GUIController.hpp"
#include "LightController.hpp"
#include "PostProcessController.hpp"
#include "SceneController.hpp"
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

    auto post = engine::core::Controller::get<PostProcessController>();
    post->set_bloom(true);
    post->set_exposure(1.0f);

    auto lights = engine::core::Controller::get<LightController>();
    auto &ceiling = lights->add_point_light();
    ceiling.position = {0.0f, 4.0f, 0.0f};
    ceiling.color = {0.6f, 0.6f, 1.0f};
    ceiling.linear = 0.009f;
    ceiling.casts_shadows = true;

    auto &warm = lights->add_point_light();
    warm.position = {3.0f, 2.0f, 7.0f};
    warm.color = {1.0f, 0.5f, 0.2f};
    warm.linear = 0.009f;
    warm.casts_shadows = true;

    auto &spot = lights->add_spot_light();
    spot.position = {0.4f, 1.0f, 0.0f};
    spot.direction = {0.0f, 0.0f, 1.0f};
    spot.color = {1.0f, 1.0f, 1.0f};
    spot.cutOff = glm::cos(glm::radians(15.0f));
    spot.outerCutOff = glm::cos(glm::radians(20.0f));
    spot.linear = 0.09f;
    spot.casts_shadows = true;

    auto scene = engine::core::Controller::get<SceneController>();
    auto resources = engine::core::Controller::get<engine::resources::ResourcesController>();
    scene->add_renderable(resources->model("room"));
    scene->add_renderable(resources->model("cube"),
                          glm::translate(glm::mat4(1.0f), {1.0f, 0.7f, 2.0f}));
    scene->add_renderable(resources->model("cube"),
                          glm::translate(glm::mat4(1.0f), {1.0f, 0.7f, 4.0f}));
    scene->add_renderable(resources->model("wheatley"),
                          glm::scale(glm::translate(glm::mat4(1.0f), {1.0f, 0.7f, 4.0f}), glm::vec3(0.05f)));
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
    auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();
    auto shader = engine::core::Controller::get<engine::resources::ResourcesController>()->shader("basic");
    auto scene = engine::core::Controller::get<SceneController>();

    shader->use();
    shader->set_mat4("projection", graphics->projection_matrix());
    shader->set_mat4("view", graphics->camera()->view_matrix());
    shader->set_vec3("viewPos", graphics->camera()->Position);
    engine::core::Controller::get<LightController>()->apply(shader);

    scene->render_all(shader);
}

void MainController::update_camera() {
    if (engine::core::Controller::get<GUIController>()->is_enabled()) {
        return;
    }
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
