#ifndef UTIL_GLFW_PLATFORM
#define UTIL_GLFW_PLATFORM

#include <GLFW/glfw3.h>

#define GLFW_KEY_COUNT 348 // 1024?

struct GLFWwindow;

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
    int keyboard_key;
    int mods;
    int scancode;
    bool keys[GLFW_KEY_COUNT];
} keyboard_t;

typedef struct input_t {
    mouse_t mouse{};
    keyboard_t keyboard{};
} input_t;

typedef struct metadata_t {
    const char *identifier = "";
    const char *version = "1.0";
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
    } flags{};

    int32_t width = 800;
    int32_t height = 600;
    int32_t refresh_rate;
} display_settings_t;

typedef struct frame_metrics_t {
    double frame_time;
    double average_frame_time;
    uint32_t frame_count;
    double fps;
} frame_metrics_t;

typedef struct platform_context_t {
    GLFWwindow *Window;

    input_t input;
    metadata_t MetaData;
    display_settings_t DisplaySettings;
    frame_metrics_t FrameMetrics;
} platform_context_t;

/**
 *
 */
int platform_init(platform_context_t *Context);

/**
 * Zero initialization
 */
void platform_init_context(platform_context_t *Context);

/**
 * Updates platform context, including frame timing
 */
void platform_update(platform_context_t *Context);

#endif // UTIL_GLFW_PLATFORM
