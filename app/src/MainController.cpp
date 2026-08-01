#include "MainController.hpp"
#include "GUIController.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/fwd.hpp"
#include "glm/trigonometric.hpp"
#include <engine/core/Engine.hpp>
#include <engine/graphics/GraphicsController.hpp>
#include <engine/graphics/LightController.hpp>
#include <engine/graphics/OpenGL.hpp>
#include <engine/graphics/PostProcessController.hpp>
#include <engine/graphics/SceneController.hpp>
#include <engine/platform/PlatformController.hpp>
#include <memory>
#include <spdlog/spdlog.h>


namespace app {

void MainPlatformEventObserver::on_key(engine::platform::Key key) {
    // spdlog::info("Keyboard event: key={}, state={}", key.name(), key.state_str());
}

void MainController::initialize() {
    engine::graphics::OpenGL::enable_depth_testing();

    auto platform = engine::core::Controller::get<engine::platform::PlatformController>();
    auto observer = std::make_unique<MainPlatformEventObserver>();
    auto camera = engine::core::Controller::get<engine::graphics::GraphicsController>()->camera();
    platform->register_platform_event_observer(std::move(observer));
    platform->set_enable_cursor(false);

    auto post = engine::core::Controller::get<engine::graphics::PostProcessController>();
    post->set_bloom(true);
    post->set_exposure(1.0f);

    camera->Position = {2, 1, 6};

    auto lights = engine::core::Controller::get<engine::graphics::LightController>();

    auto &point1 = lights->add_point_light();
    point1.position = {-5.6f, 3.0f, 6.4f};
    point1.color = {0.8f, 0.8f, 1.0f};
    point1.linear = 0.009f;
    point1.quadratic = 0.2f;
    point1.casts_shadows = true;

    auto &point2 = lights->add_point_light();
    point2.position = {2.5f, 3.0f, -0.5f};
    point2.color = {1.0f, 1.0f, 1.0f};
    point2.linear = 0.009f;
    point2.quadratic = 0.2f;
    point2.casts_shadows = true;

    auto &point3 = lights->add_point_light();
    point3.position = {2.3f, 1.4f, 15.6f};
    point3.color = {0.0f, 0.0f, 0.0f};
    point3.linear = 0.009f;
    point3.quadratic = 0.2f;
    point3.casts_shadows = true;
    m_point3_index = static_cast<int>(lights->point_lights().size()) - 1;

    auto &wheatley_light = lights->add_spot_light();
    wheatley_light.position = {1.7f, 2.0f, 4.0f};
    wheatley_light.direction = {1.0f, 0.0f, 0.0f};
    wheatley_light.color = {1.0f, 1.0f, 1.0f};
    wheatley_light.cutoff = glm::cos(glm::radians(10.0f));
    wheatley_light.outer_cutoff = glm::cos(glm::radians(15.0f));
    wheatley_light.linear = 0.027f;
    wheatley_light.casts_shadows = true;
    m_wheatley_light_index = static_cast<int>(lights->spot_lights().size()) - 1;

    auto scene = engine::core::Controller::get<engine::graphics::SceneController>();
    auto resources = engine::core::Controller::get<engine::resources::ResourcesController>();

    auto temple_model = glm::mat4(1.0);
    temple_model = glm::translate(temple_model, {-1.0f, 1.0f, 15.0f});
    temple_model = glm::rotate(temple_model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    temple_model = glm::scale(temple_model, glm::vec3{0.004f});

    auto turret_model = glm::mat4(1.0);
    turret_model = glm::translate(turret_model, {-5.0f, 0.0f, 8.0f});
    turret_model = glm::rotate(turret_model, glm::radians(70.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    turret_model = glm::scale(turret_model, glm::vec3{0.05f});

    scene->add_renderable(resources->model("room"));

    auto &temple_renderable = scene->add_renderable(resources->model("temple"), temple_model);
    temple_renderable.visible = false;
    m_temple_renderable_index = static_cast<int>(scene->renderables().size()) - 1;

    scene->add_renderable(resources->model("turret"), turret_model);
    scene->add_renderable(resources->model("cube"),
                          glm::translate(glm::mat4(1.0f), {1.0f, 0.7f, 6.0f}));

    auto moron_model = glm::mat4(1.0f);
    moron_model = glm::translate(moron_model, {5.0f, 0.7f, 5.0f});
    moron_model = glm::rotate(moron_model, glm::radians(200.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    moron_model = glm::rotate(moron_model, glm::radians(-20.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    moron_model = glm::scale(moron_model, glm::vec3(0.05f));
    scene->add_renderable(resources->model("wheatley"),
                          moron_model);
    m_wheatley_renderable_index = static_cast<int>(scene->renderables().size()) - 1;
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
    if (platform->key(engine::platform::KEY_F).state() == engine::platform::Key::State::JustPressed) {
        auto lights = engine::core::Controller::get<engine::graphics::LightController>();
        auto &wl = lights->spot_lights()[m_wheatley_light_index];
        m_wheatley_light_on = !m_wheatley_light_on;
        wl.color = m_wheatley_light_on ? glm::vec3(1.0f, 1.0f, 1.0f) : glm::vec3(0.0f);
    }
    if (platform->key(engine::platform::KEY_E).state() == engine::platform::Key::State::JustPressed) {
        if (m_event_state == EventState::IDLE) {
            m_event_state = EventState::TEMPLE_WAIT;
            m_event_timer = 0.0f;
        }
    }
}

void MainController::update() {
    auto platform = engine::core::Controller::get<engine::platform::PlatformController>();
    auto scene = engine::core::Controller::get<engine::graphics::SceneController>();
    auto lights = engine::core::Controller::get<engine::graphics::LightController>();
    auto &transform = scene->renderables()[m_wheatley_renderable_index].transform;
    auto &wl = lights->spot_lights()[m_wheatley_light_index];

    glm::vec3 eye_local(14.0f, 26.0f, 0.0f);
    glm::vec3 dir_local(1.0f, 0.0f, 0.0f);
    wl.position = glm::vec3(transform * glm::vec4(eye_local, 1.0f));
    wl.direction = glm::normalize(glm::mat3(transform) * dir_local);

    if (m_event_state != EventState::IDLE) {
        m_event_timer += platform->dt();

        if (m_event_state == EventState::TEMPLE_WAIT && m_event_timer >= 1.0f) {
            scene->renderables()[m_temple_renderable_index].visible = true;
            m_event_timer = 0.0f;
            m_event_state = EventState::DIM_WAIT;
        } else if (m_event_state == EventState::DIM_WAIT && m_event_timer >= 2.0f) {
            auto &points = lights->point_lights();
            m_saved_point_colors.clear();
            for (auto &p: points) {
                m_saved_point_colors.push_back(p.color);
            }
            points[0].color = {1.0f, 0.0f, 0.0f};
            points[1].color = {1.0f, 0.0f, 0.0f};
            points[m_point3_index].color = {1.0f, 0.0f, 0.0f};
            m_event_timer = 0.0f;
            m_event_state = EventState::RESTORE_WAIT;
        } else if (m_event_state == EventState::RESTORE_WAIT && m_event_timer >= 3.0f) {
            auto &points = lights->point_lights();
            for (int i = 0; i < static_cast<int>(m_saved_point_colors.size()); ++i) {
                points[i].color = m_saved_point_colors[i];
            }
            scene->renderables()[m_temple_renderable_index].visible = false;
            m_event_state = EventState::IDLE;
        }
    }

    update_camera();
}

void MainController::draw() {
    auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();
    auto shader = engine::core::Controller::get<engine::resources::ResourcesController>()->shader("basic");
    auto scene = engine::core::Controller::get<engine::graphics::SceneController>();

    shader->use();
    shader->set_mat4("projection", graphics->projection_matrix());
    shader->set_mat4("view", graphics->camera()->view_matrix());
    shader->set_vec3("viewPos", graphics->camera()->Position);
    engine::core::Controller::get<engine::graphics::LightController>()->apply(shader);

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
    if (!engine::core::Controller::get<GUIController>()->is_enabled()) {
        camera->rotate_camera(mouse.dx, mouse.dy);
    }
    camera->zoom(mouse.scroll);
}
}// namespace app
