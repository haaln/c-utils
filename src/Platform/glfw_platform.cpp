#include "glfw_platform.h"

#include <GLFW/glfw3.h>

// Forward declarations/
static void error_callback(int error, const char *msg);
static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
static void mouse_position_callback(GLFWwindow *window, double x, double y);
static void framebuffer_resize_callback(GLFWwindow *window, int width, int height);
static void scroll_callback(GLFWwindow *window, double offset_x, double offset_y);
static void cursor_enter_callback(GLFWwindow *window, int entered);
//

static double last_frame;

int platform_init(platform_context_t *Context)
{
    if(!glfwInit()) return 1;

    Context->Window = glfwCreateWindow(
        Context->DisplaySettings.width, Context->DisplaySettings.height, Context->MetaData.title, NULL, NULL);
    if(!Context->Window) {
        glfwTerminate();
        return 1;
    }

    glfwSetErrorCallback(error_callback);

    glfwSetWindowAspectRatio(Context->Window, 1, 1);

    glfwSetKeyCallback(Context->Window, key_callback);
    glfwSetMouseButtonCallback(Context->Window, mouse_button_callback);
    glfwSetCursorPosCallback(Context->Window, mouse_position_callback);
    glfwSetCursorEnterCallback(Context->Window, cursor_enter_callback);
    glfwSetScrollCallback(Context->Window, scroll_callback);

    glfwSetWindowUserPointer(Context->Window, (void *)Context);

    glfwSetFramebufferSizeCallback(Context->Window, framebuffer_resize_callback);
    glfwGetFramebufferSize(Context->Window, &Context->DisplaySettings.width, &Context->DisplaySettings.height);
    framebuffer_resize_callback(Context->Window, Context->DisplaySettings.width, Context->DisplaySettings.height);

    glfwSetInputMode(Context->Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwSetTime(0.0);

    return 0;
}

double platform_update_frame_time()
{
    double current_frame = (double)glfwGetTime();
    last_frame = current_frame;
    return current_frame - last_frame;
}

void platform_update(platform_context_t *Context)
{
    Context->FrameMetrics.frame_time = platform_update_frame_time();
}

static void cursor_enter_callback(GLFWwindow *window, int entered)
{
}

static void scroll_callback(GLFWwindow *window, double offset_x, double offset_y)
{
}

static void framebuffer_resize_callback(GLFWwindow *window, int width, int height)
{
    platform_context_t *Context = (platform_context_t *)glfwGetWindowUserPointer(window);
    Context->DisplaySettings.height = height;
    Context->DisplaySettings.width = width;
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    platform_context_t *Context = (platform_context_t *)glfwGetWindowUserPointer(window);
    if(action == GLFW_PRESS || action == GLFW_REPEAT) {
        Context->input.keyboard.keyboard_key = key;
        if(key == GLFW_KEY_ESCAPE) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
        if(key >= 0 && key < GLFW_KEY_COUNT) {
            Context->input.keyboard.keys[key] = true;
            Context->input.keyboard.mods = mods;
            Context->input.keyboard.scancode = scancode;
        }
    } else {
        Context->input.keyboard.keyboard_key = 0;
        Context->input.keyboard.mods = 0;
        Context->input.keyboard.scancode = 0;
        Context->input.keyboard.keys[key] = false;
    }
}

static void error_callback(int error, const char *msg)
{
    // Callback function here...

    // e.g.
    // fprintf(stderr, "[ERROR] %s (%d): %d %s\n", __FILE__, __LINE__, error, msg);
}

static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    platform_context_t *Context = (platform_context_t *)glfwGetWindowUserPointer(window);
    Context->input.mouse.mouse_button = button;
    Context->input.mouse.mouse_action = action;
    Context->input.mouse.mouse_mod = mods;
}

static void mouse_position_callback(GLFWwindow *window, double x, double y)
{
    platform_context_t *Context = (platform_context_t *)glfwGetWindowUserPointer(window);
    Context->input.mouse.mouse_x = x;
    Context->input.mouse.mouse_y = y;
}

void platform_init_context(platform_context_t *Context)
{
    Context->DisplaySettings = {
        .flags = { .vsync_enabled = 0,
                  .fullscreen_enabled = 0,
                  .windowed_fullscreen_enabled = 0,
                  .borderless_enabled = 0,
                  .window_resizable_enabled = 0,
                  .reserved = 0 },
        .width = 800,
        .height = 600,
        .refresh_rate = 0
    };
    Context->MetaData = { .identifier = "", .version = "0.1", .title = "title" };
}
