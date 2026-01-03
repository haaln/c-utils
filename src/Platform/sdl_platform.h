#ifndef SDL_PLATFORM_H
#define SDL_PLATFORM_H

struct SDL_Window;

#include "SDL3/SDL_scancode.h"

typedef struct mouse_t {
    int mouse_action;
    int mouse_button;
    int mouse_mod;

    float mouse_x;
    float mouse_y;
    float mouse_delta_x;
    float mouse_delta_y;
    bool mouse_left_pressed;
    bool mouse_left_released;
    bool mouse_right_pressed;
    bool mouse_right_released;
} mouse_t;

typedef struct keyboard_t {
    const bool *key;
    bool key_state[SDL_SCANCODE_COUNT];
    bool prev_key_state[SDL_SCANCODE_COUNT];
    bool key_presssed[SDL_SCANCODE_COUNT];
    bool key_released[SDL_SCANCODE_COUNT];
} keyboard_t;

typedef struct input_t {
    mouse_t mouse;
    keyboard_t keyboard;
} input_t;

typedef struct metadata_t {
    const char *identifier = "";
    const char *version = "";
    const char *title = "";
} metadata_t;

typedef struct display_settings_t {
    struct {
        uint8_t vsync_enabled : 1;
        uint8_t fullscreen_enabled : 1;
        uint8_t windowed_fullscreen_enabled : 1;
        uint8_t borderless_enabled : 1;
        uint8_t window_resizable_enabled : 1;
        uint8_t reserved : 3;
    } flags;

    int32_t width = 800;
    int32_t height = 600;
    uint32_t refresh_rate;
} display_settings_t;

typedef struct frame_metrics_t {
    double frame_time;
    double average_frame_time;
    uint32_t frame_count;
    double fps;
} frame_metrics_t;

typedef struct platform_context_t {
    SDL_Window *Window;
    input_t Input;
    metadata_t MetaData;
    display_settings_t DisplaySettings;
    frame_metrics_t FrameMetrics;
} platform_context_t;

/**
 *
 */
int platform_init(platform_context_t *ctx);

/**
 * Updates platform context, including frame timing
 */
void platform_update(platform_context_t *ctx);

/**
 *
 */
int platform_init_context(platform_context_t *ctx);

/**
 * equivalent of SDL_Delay();
 */
void platform_delay(platform_context_t *ctx, unsigned int delay);

#endif // !SDL_PLATFORM_H
