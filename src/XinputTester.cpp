#define SOKOL_IMPL
#define SOKOL_NO_ENTRY
#define SOKOL_GLCORE
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_log.h"
#include "sokol_glue.h"
#define SOKOL_DEBUGTEXT_IMPL
#include "sokol_debugtext.h"

#define NUM_FONTS  (3)
#define FONT_KC854 (0)
#define FONT_C64   (1)
#define FONT_ORIC  (2)

typedef struct {
    uint8_t r, g, b;
} color_t;

static struct {
    sg_pass_action pass_action;
    color_t palette[NUM_FONTS];
} state;

static void init(void) {
    state.pass_action.colors[0] = {
            .load_action = SG_LOADACTION_CLEAR,
            .clear_value = { 0.0f, 0.125f, 0.25f, 1.0f }
    };
    state.palette[0] = { 0xf4, 0x43, 0x36 };
    state.palette[1] = { 0x21, 0x96, 0xf3 };
    state.palette[2] = { 0x4c, 0xaf, 0x50 };

    sg_desc _sg_desc = {};
    _sg_desc.environment = sglue_environment();
    _sg_desc.logger.func = slog_func;
    sg_setup(&_sg_desc);

    sdtx_desc_t _sdtx_desc{};
    _sdtx_desc.fonts[FONT_KC854] = sdtx_font_kc854();
    _sdtx_desc.fonts[FONT_C64]   = sdtx_font_c64();
    _sdtx_desc.fonts[FONT_ORIC]  = sdtx_font_oric();
    _sdtx_desc.logger.func = slog_func;
    sdtx_setup(&_sdtx_desc);
}

static void my_printf_wrapper(const char* fmt, ...) {
    va_list args;
            va_start(args, fmt);
    sdtx_vprintf(fmt, args);
            va_end(args);
}

static void frame(void) {
    uint32_t frame_count = (uint32_t)sapp_frame_count();
    double frame_time = sapp_frame_duration() * 1000.0;

    sdtx_canvas(sapp_width() * 0.5f, sapp_height() * 0.5f);
    sdtx_origin(3.0f, 3.0f);
    for (int i = 0; i < NUM_FONTS; i++) {
        color_t color = state.palette[i];
        sdtx_font(i);
        sdtx_color3b(color.r, color.g, color.b);
        sdtx_printf("Hello '%s'!\n", (frame_count & (1<<7)) ? "Welt" : "World");
        sdtx_printf("\tFrame Time:\t\t%.3f\n", frame_time);
        my_printf_wrapper("\tFrame Count:\t%d\t0x%04X\n", frame_count, frame_count);
        sdtx_putr("Range Test 1(xyzbla)", 12);
        sdtx_putr("\nRange Test 2\n", 32);
        sdtx_move_y(2);
    }
    sg_begin_pass({ .action = state.pass_action, .swapchain = sglue_swapchain() });
    sdtx_draw();
    sg_end_pass();
    sg_commit();

}

static void cleanup(void) {
    sdtx_shutdown();
    sg_shutdown();
}

void input(const sapp_event* event) {}

int main(int argc, const char* argv[]) {
    (void)argc;
    (void)argv;

    sapp_desc desc = {};
    desc.init_cb = init;
    desc.frame_cb = frame;
    desc.cleanup_cb = cleanup,
    desc.event_cb = input,
    desc .width = 640,
    desc.height = 480,
    desc.window_title = "Xinput Tester",
    desc.icon.sokol_default = true,
    desc.logger.func = slog_func;
    sapp_run(desc);

    return 0;
}
