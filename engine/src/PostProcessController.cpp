#include <engine/graphics/OpenGL.hpp>
#include <engine/graphics/PostProcessController.hpp>
#include <engine/platform/PlatformController.hpp>
#include <engine/resources/ResourcesController.hpp>

namespace engine::graphics {

void PostProcessController::initialize() {
    m_quad_vao = OpenGL::create_screen_quad();
}

void PostProcessController::begin_draw() {
    auto platform = engine::core::Controller::get<engine::platform::PlatformController>();
    int width = platform->window()->width();
    int height = platform->window()->height();

    if (width != m_fb_width || height != m_fb_height) {
        OpenGL::destroy_hdr_framebuffer(m_hdr_fb);
        OpenGL::destroy_ping_pong_buffers(m_ping_pong);
        m_hdr_fb = OpenGL::create_hdr_framebuffer(width, height);
        m_ping_pong = OpenGL::create_ping_pong_buffers(width, height);
        m_fb_width = width;
        m_fb_height = height;
    }

    OpenGL::bind_framebuffer(m_hdr_fb.fbo);
    OpenGL::clear_buffers();
}

void PostProcessController::end_draw() {
    if (m_bloom_enabled) {
        render_bloom();
    }

    OpenGL::bind_framebuffer(0);
    OpenGL::clear_buffers();

    auto resources = engine::core::Controller::get<engine::resources::ResourcesController>();
    auto bloom_shader = resources->shader("bloom_final");
    bloom_shader->use();
    OpenGL::bind_texture_2d(0, m_hdr_fb.color_buffers[0]);
    OpenGL::bind_texture_2d(1, m_ping_pong.textures[0]);
    bloom_shader->set_int("scene", 0);
    bloom_shader->set_int("bloomBlur", 1);
    bloom_shader->set_int("bloom", m_bloom_enabled ? 1 : 0);
    bloom_shader->set_float("exposure", m_exposure);
    OpenGL::draw_screen_quad(m_quad_vao);
}

void PostProcessController::render_bloom() {
    auto resources = engine::core::Controller::get<engine::resources::ResourcesController>();
    auto blur_shader = resources->shader("blur");

    int horizontal = 1;
    bool first_iteration = true;
    int amount = 10;
    blur_shader->use();
    blur_shader->set_int("image", 0);
    for (int i = 0; i < amount; i++) {
        OpenGL::bind_framebuffer(m_ping_pong.fbo[horizontal]);
        blur_shader->set_int("horizontal", horizontal);
        OpenGL::bind_texture_2d(0,
                                first_iteration ? m_hdr_fb.color_buffers[1] : m_ping_pong.textures[1 - horizontal]);
        OpenGL::draw_screen_quad(m_quad_vao);
        horizontal = 1 - horizontal;
        first_iteration = false;
    }
}

}// namespace engine::graphics
