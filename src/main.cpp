#define SOKOL_IMPL
#define SOKOL_NO_ENTRY
#define SOKOL_GLCORE33
#include "sokol_app.h"
#include "sokol_audio.h"
#include "sokol_gfx.h"
#include "sokol_glue.h"
#include "sokol_log.h"
#include "NES.h"
#include <iostream>

constexpr auto NES_WIDTH  = 256;
constexpr auto NES_HEIGHT = 240;

sg_pass_action pass_action{};
sg_buffer vbuf{};
sg_pipeline pip{};
sg_bindings bind{};

NES* nes;

void init() {
    sg_desc desc = {};
    desc.context = sapp_sgcontext();
    desc.logger.func = slog_func;
    sg_setup(&desc);

    const float vertices[] = {
            // positions            // colors
            0.0f,  0.5f, 0.0f,     1.0f, 0.0f, 0.0f, 1.0f,
            0.5f, -0.5f, 0.0f,     0.0f, 1.0f, 0.0f, 1.0f,
            -0.5f, -0.5f, 0.0f,     0.0f, 0.0f, 1.0f, 1.0f
    };
    sg_buffer_desc vb_desc = {};
    vb_desc.data = SG_RANGE(vertices);
    vbuf = sg_make_buffer(&vb_desc);

    sg_shader_desc shd_desc = {};
    shd_desc.vs.source = R"(
#version 330
layout(location=0) in vec4 position;
layout(location=1) in vec4 color0;
out vec4 color;
void main() {
  gl_Position = position;
  color = color0;
}
)";
    shd_desc.fs.source = R"(
#version 330
in vec4 color;
out vec4 frag_color;
void main() {
  frag_color = color;
}
)";
    sg_shader shd = sg_make_shader(&shd_desc);

    sg_pipeline_desc pip_desc = {};
    pip_desc.shader = shd;
    pip_desc.layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT3;
    pip_desc.layout.attrs[1].format = SG_VERTEXFORMAT_FLOAT4;
    pip = sg_make_pipeline(&pip_desc);

    bind.vertex_buffers[0] = vbuf;

    pass_action.colors[0] = { .action=SG_ACTION_CLEAR, .value={1.0f, 0.0f, 0.0f, 1.0f} };
}

void frame() {
    sg_begin_default_pass(&pass_action, sapp_width(), sapp_height());
    sg_apply_pipeline(pip);
    sg_apply_bindings(&bind);
    sg_draw(0, 3, 1);
    sg_end_pass();
    sg_commit();
}

void cleanup() {
    sg_shutdown();
}

void input(const sapp_event* event) {

}

int main() {
    sapp_desc desc = {};
    desc.init_cb = init;
    desc.frame_cb = frame;
    desc.cleanup_cb = cleanup,
    desc.event_cb = input,
    desc .width = NES_WIDTH,
    desc.height = NES_HEIGHT,
    desc.window_title = "nesemu.cpp",
    desc.icon.sokol_default = true,
    desc.logger.func = slog_func;
    sapp_run(desc);
    return 0;
}
