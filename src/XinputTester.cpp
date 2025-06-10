#define SOKOL_IMPL
#define SOKOL_NO_ENTRY
#define SOKOL_GLCORE
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_log.h"
#include "sokol_glue.h"
#define SOKOL_DEBUGTEXT_IMPL
#include "sokol_debugtext.h"

#include <windows.h>
#include <xinput.h>
#pragma comment(lib, "Xinput.lib")

#define NUM_FONTS  (3)
#define FONT_KC854 (0)
#define FONT_C64   (1)
#define FONT_ORIC  (2)
#define FONT_KC853 (3)
#define FONT_Z1013 (4)
#define FONT_CPC   (5)

typedef struct {
    uint8_t r, g, b;
} color_t;

static struct {
    sg_pass_action pass_action;
    color_t palette[NUM_FONTS];
    XINPUT_STATE controllerState[4];
    bool controllerActive[4];
} state;

static void init(void) {
    state.pass_action.colors[0] = {
            .load_action = SG_LOADACTION_CLEAR,
            .clear_value = { 0.0f, 0.1f, 0.2f, 1.0f }
    };
    state.palette[0] = { 0xf4, 0x43, 0x36 };
    state.palette[1] = { 0x4c, 0xaf, 0x50 };
    state.palette[2] = { 0x21, 0x96, 0xf3 };

    sg_desc _sg_desc = {};
    _sg_desc.environment = sglue_environment();
    _sg_desc.logger.func = slog_func;
    sg_setup(&_sg_desc);

    sdtx_desc_t _sdtx_desc{};
    _sdtx_desc.fonts[FONT_KC854] = sdtx_font_kc854();
    _sdtx_desc.fonts[FONT_C64]   = sdtx_font_c64();
    _sdtx_desc.fonts[FONT_ORIC]  = sdtx_font_oric();
    _sdtx_desc.fonts[FONT_KC853] = sdtx_font_kc853();
    _sdtx_desc.fonts[FONT_Z1013] = sdtx_font_z1013();
    _sdtx_desc.fonts[FONT_CPC]   = sdtx_font_cpc();
    _sdtx_desc.logger.func = slog_func;
    sdtx_setup(&_sdtx_desc);

    for(int i=0; i<4; i++) {
        state.controllerActive[i] = false;
    }
}

static void my_printf_wrapper(const char* fmt, ...) {
    va_list args;
            va_start(args, fmt);
    sdtx_vprintf(fmt, args);
            va_end(args);
}

static void frame(void) {
    for(int i=0; i<4; i++) {
        // Zero out the state structure prior to retrieving the new state for it.
        ZeroMemory(&state.controllerState[i], sizeof(XINPUT_STATE));

        // Get the state of the controller.
        DWORD result = XInputGetState(i, &state.controllerState[i]);

        // Store whether the controller is currently connected or not.
        if(result == ERROR_SUCCESS) {
            state.controllerActive[i] = true;
        } else {
            state.controllerActive[i] = false;
        }
    }

    uint32_t frame_count = (uint32_t)sapp_frame_count();
    double frame_time = sapp_frame_duration() * 1000.0;

    sdtx_canvas(sapp_width() * 0.5f, sapp_height() * 0.5f);
    sdtx_origin(3.0f, 3.0f);
    {
        sdtx_font(FONT_ORIC);
        for (int i = 0; i < 4; i++) {
            if (state.controllerActive[i]) {
                color_t color = state.palette[1];
                sdtx_color3b(color.r, color.g, color.b);
                sdtx_printf("Controller Active: Yes\n");
            } else {
                color_t color = state.palette[0];
                sdtx_color3b(color.r, color.g, color.b);
                sdtx_printf("Controller Active: No\n");
            }
        }

        int first_active_controller_index = -1;
        for (int i = 0; i < 4; i++) {
            if (state.controllerActive[i]) {
                first_active_controller_index = i;
                break;
            }
        }

        if (first_active_controller_index >= 0) {
            color_t color = state.palette[1];
            sdtx_color3b(color.r, color.g, color.b);
            sdtx_printf("First active controller: %d\n", first_active_controller_index);
        } else {
            color_t color = state.palette[0];
            sdtx_color3b(color.r, color.g, color.b);
            sdtx_printf("NO active controller!!!\n");
        }
        sdtx_printf("\n");

        if (first_active_controller_index >= 0) {
            int i = first_active_controller_index;
            sdtx_color3b(0xFF, 0xFF, 0xFF);

            // button
            bool buttonA = state.controllerState[i].Gamepad.wButtons & XINPUT_GAMEPAD_A;
            bool buttonB = state.controllerState[i].Gamepad.wButtons & XINPUT_GAMEPAD_B;
            bool buttonX = state.controllerState[i].Gamepad.wButtons & XINPUT_GAMEPAD_X;
            bool buttonY = state.controllerState[i].Gamepad.wButtons & XINPUT_GAMEPAD_Y;
            sdtx_printf("A Button Down: %s\n", buttonA ? "Yes" : "No");
            sdtx_printf("B Button Down: %s\n", buttonB ? "Yes" : "No");
            sdtx_printf("X Button Down: %s\n", buttonX ? "Yes" : "No");
            sdtx_printf("Y Button Down: %s\n", buttonY ? "Yes" : "No");
            sdtx_printf("\n");

            // trigger
            BYTE triggerLeftValue = state.controllerState[i].Gamepad.bLeftTrigger;
            BYTE triggerRightValue = state.controllerState[i].Gamepad.bRightTrigger;
            float leftValue,rightValue;
            if(triggerLeftValue < XINPUT_GAMEPAD_TRIGGER_THRESHOLD) {
                leftValue = 0.0f;
            } else {
                leftValue = (float)triggerLeftValue / 255.0f;
            }
            if(triggerRightValue < XINPUT_GAMEPAD_TRIGGER_THRESHOLD) {
                rightValue = 0.0f;
            } else {
                rightValue = (float)triggerRightValue / 255.0f;
            }
            sdtx_printf("Left Trigger: %f\n", leftValue);
            sdtx_printf("Right Trigger: %f\n", rightValue);
            sdtx_printf("\n");

            // left thumb
            int thumbLeftX = (int)state.controllerState[i].Gamepad.sThumbLX;
            int thumbLeftY = (int)state.controllerState[i].Gamepad.sThumbLY;
            int magnitude = (int)sqrt((thumbLeftX * thumbLeftX) + (thumbLeftY * thumbLeftY));
            // Check if the controller is inside a circular dead zone.
            if(magnitude < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) {
                thumbLeftX = 0;
                thumbLeftY = 0;
            }
            sdtx_printf("Left Thumb X: %d\n", thumbLeftX);
            sdtx_printf("Right Thumb Y: %d\n", thumbLeftY);
        }
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
