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
sg_buffer ibuf{};
sg_pipeline pip{};
sg_bindings bind{};
uint32_t pixel_buffer[NES_WIDTH * NES_HEIGHT];

NES* nes;

void init() {
    sg_desc desc = {};
    desc.context = sapp_sgcontext();
    desc.logger.func = slog_func;
    sg_setup(&desc);

    const float vertices[] = {
            // positions     uv
            -1.0, -1.0, 0.0, 0.0, 1.0,
            1.0,  -1.0, 0.0, 1.0, 1.0,
            1.0,  1.0,  0.0, 1.0, 0.0,
            -1.0, 1.0,  0.0, 0.0, 0.0,
    };
    sg_buffer_desc vb_desc = {};
    vb_desc.data = SG_RANGE(vertices);
    vbuf = sg_make_buffer(&vb_desc);

    const int indices[] = { 0, 1, 2, 0, 2, 3, };
    sg_buffer_desc ib_desc = {};
    ib_desc.type = SG_BUFFERTYPE_INDEXBUFFER;
    ib_desc.data = SG_RANGE(indices);
    ibuf = sg_make_buffer(&ib_desc);

    sg_shader_desc shd_desc = {};
    shd_desc.attrs[0].name = "position";
    shd_desc.attrs[1].name = "texcoord0";
    shd_desc.vs.source = R"(
#version 330
layout(location=0) in vec3 position;
layout(location=1) in vec2 texcoord0;
out vec4 color;
out vec2 uv;
void main() {
  gl_Position = vec4(position, 1.0f);
  uv = texcoord0;
  color = vec4(uv, 0.0f, 1.0f);
}
)";
    shd_desc.fs.images[0].name = "tex";
    shd_desc.fs.images[0].image_type = SG_IMAGETYPE_2D;
    shd_desc.fs.images[0].sampler_type = SG_SAMPLERTYPE_FLOAT;
    shd_desc.fs.source = R"(
#version 330
uniform sampler2D tex;
in vec4 color;
in vec2 uv;
out vec4 frag_color;
void main() {
  frag_color = texture(tex, uv);
}
)";

    sg_image_desc img_desc = {};
    img_desc.width = NES_WIDTH;
    img_desc.height = NES_HEIGHT;
    img_desc.label = "nes-texture";
    img_desc.pixel_format = SG_PIXELFORMAT_RGBA8;
    img_desc.usage = SG_USAGE_STREAM;

    sg_shader shd = sg_make_shader(&shd_desc);

    sg_pipeline_desc pip_desc = {};
    pip_desc.shader = shd;
    pip_desc.layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT3;
    pip_desc.layout.attrs[1].format = SG_VERTEXFORMAT_FLOAT2;
    pip_desc.index_type = SG_INDEXTYPE_UINT32;
    pip = sg_make_pipeline(&pip_desc);

    bind.vertex_buffers[0] = vbuf;
    bind.index_buffer = ibuf;
    bind.fs_images[0] = sg_make_image(&img_desc);

    pass_action.colors[0] = { .action=SG_ACTION_CLEAR, .value={0.5f, 0.0f, 0.0f, 1.0f} };
}

void frame() {
    const double dt = sapp_frame_duration();
    // processe input
//    nes->controller1->buttons = controller1;
    nes->controller1->buttons = 0;
    nes->controller2->buttons = 0;

    // step the NES state forward by 'dt' seconds, or more if in fast-forward
    emulate(nes, dt);

    for (int y = 0; y < NES_HEIGHT; y++) {
        for (int x = 0; x < NES_WIDTH; x++) {
            pixel_buffer[y * NES_WIDTH + x] = nes->ppu->front[y * NES_WIDTH +  x];
        }
    }

    sg_image_data image_data{};
    image_data.subimage[0][0] = SG_RANGE(pixel_buffer);
    sg_update_image(bind.fs_images[0], image_data);

    sg_begin_default_pass(&pass_action, sapp_width(), sapp_height());
    sg_apply_pipeline(pip);
    sg_apply_bindings(&bind);
    sg_draw(0, 6, 1);
    sg_end_pass();
    sg_commit();
}

void cleanup() {
    sg_shutdown();
}

void input(const sapp_event* event) {

}

int main(int argc, const char* argv[]) {
    if (argc < 2) {
        std::cerr << "Please pass ROM path as first parameter.\n";
        return EXIT_FAILURE;
    }

    char* SRAM_path = new char[strlen(argv[1]) + 1];
    strcpy(SRAM_path, argv[1]);
    strcat(SRAM_path, ".srm");

    std::cout << "Initializing NES..." << std::endl;
    nes = new NES(argv[1], SRAM_path);
    if (!nes->initialized) return EXIT_FAILURE;

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

    // save SRAM back to file
    if (nes->cartridge->battery_present) {
        std::cout << std::endl << "Writing SRAM..." << std::endl;
        FILE* fp = fopen(SRAM_path, "wb");
        if (fp == nullptr || (fwrite(nes->cartridge->SRAM, 8192, 1, fp) != 1)) {
            std::cout << "WARN: failed to save SRAM file!" << std::endl;
        }
        else {
            fclose(fp);
        }
    }

    return 0;
}
