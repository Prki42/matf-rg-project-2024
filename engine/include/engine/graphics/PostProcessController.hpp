#ifndef ENGINE_POSTPROCESSCONTROLLER_HPP
#define ENGINE_POSTPROCESSCONTROLLER_HPP

#include <engine/core/Controller.hpp>
#include <engine/graphics/OpenGL.hpp>

namespace engine::graphics {

class PostProcessController final : public engine::core::Controller {
public:
    std::string_view name() const override {
        return "PostProcessController";
    }

    void set_bloom(bool enabled) { m_bloom_enabled = enabled; }
    bool bloom() const { return m_bloom_enabled; }
    void set_exposure(float exposure) { m_exposure = exposure; }
    float exposure() const { return m_exposure; }

private:
    void initialize() override;
    void terminate() override;
    void begin_draw() override;
    void end_draw() override;

    void render_bloom();

    bool m_bloom_enabled = false;
    float m_exposure = 1.0f;

    OpenGL::HdrFramebuffer m_hdr_fb;
    OpenGL::PingPongBuffers m_ping_pong;
    uint32_t m_quad_vao = 0;
    int m_fb_width = 0;
    int m_fb_height = 0;
};

}// namespace engine::graphics
#endif//ENGINE_POSTPROCESSCONTROLLER_HPP
