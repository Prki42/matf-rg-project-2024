#include "GUIController.hpp"
#include <engine/core/Engine.hpp>
#include <engine/graphics/GraphicsController.hpp>
#include <engine/graphics/LightController.hpp>
#include <engine/graphics/PostProcessController.hpp>
#include <imgui.h>

namespace app {

void GUIController::initialize() {
    set_enable(false);
}

void GUIController::poll_events() {
    const auto platform = engine::core::Controller::get<engine::platform::PlatformController>();
    if (platform->key(engine::platform::KeyId::KEY_F2).state() == engine::platform::Key::State::JustPressed) {
        set_enable(!is_enabled());
        platform->set_enable_cursor(is_enabled());
    }
}

void GUIController::end_draw() {
    auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();
    auto camera = graphics->camera();
    auto post = engine::core::Controller::get<engine::graphics::PostProcessController>();
    auto lights = engine::core::Controller::get<engine::graphics::LightController>();

    graphics->begin_gui();

    ImGui::Begin("Scene Controls");

    if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("Position: (%.2f, %.2f, %.2f)", camera->Position.x, camera->Position.y, camera->Position.z);
        ImGui::Text("Yaw: %.1f  Pitch: %.1f", camera->Yaw, camera->Pitch);
    }

    if (ImGui::CollapsingHeader("Post Processing", ImGuiTreeNodeFlags_DefaultOpen)) {
        bool bloom = post->bloom();
        if (ImGui::Checkbox("Bloom", &bloom)) {
            post->set_bloom(bloom);
        }
        float exposure = post->exposure();
        if (ImGui::SliderFloat("Exposure", &exposure, 0.1f, 5.0f)) {
            post->set_exposure(exposure);
        }
    }

    if (ImGui::CollapsingHeader("Point Lights", ImGuiTreeNodeFlags_DefaultOpen)) {
        auto &point_lights = lights->point_lights();
        for (int i = 0; i < static_cast<int>(point_lights.size()); ++i) {
            ImGui::PushID(i);
            if (ImGui::TreeNode("", "Point Light %d", i)) {
                ImGui::DragFloat3("Position", &point_lights[i].position.x, 0.1f);
                ImGui::ColorEdit3("Color", &point_lights[i].color.x);
                ImGui::DragFloat("Linear", &point_lights[i].linear, 0.001f, 0.0f, 1.0f);
                ImGui::DragFloat("Quadratic", &point_lights[i].quadratic, 0.001f, 0.0f, 1.0f);
                ImGui::Checkbox("Shadows", &point_lights[i].casts_shadows);
                ImGui::TreePop();
            }
            ImGui::PopID();
        }
    }

    if (ImGui::CollapsingHeader("Spot Lights", ImGuiTreeNodeFlags_DefaultOpen)) {
        auto &spot_lights = lights->spot_lights();
        for (int i = 0; i < static_cast<int>(spot_lights.size()); ++i) {
            ImGui::PushID(100 + i);
            if (ImGui::TreeNode("", "Spot Light %d", i)) {
                ImGui::DragFloat3("Position", &spot_lights[i].position.x, 0.1f);
                ImGui::DragFloat3("Direction", &spot_lights[i].direction.x, 0.01f, -1.0f, 1.0f);
                ImGui::ColorEdit3("Color", &spot_lights[i].color.x);
                ImGui::DragFloat("Linear", &spot_lights[i].linear, 0.001f, 0.0f, 1.0f);
                ImGui::DragFloat("Quadratic", &spot_lights[i].quadratic, 0.001f, 0.0f, 1.0f);
                ImGui::Checkbox("Shadows", &spot_lights[i].casts_shadows);
                ImGui::TreePop();
            }
            ImGui::PopID();
        }
    }

    if (ImGui::CollapsingHeader("Debug", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("Show Light Positions", &lights->draw_debug());
    }

    ImGui::End();
    graphics->end_gui();
}

}// namespace app
