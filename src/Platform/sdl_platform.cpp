#include "sdl_platform.h"
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_video.h"
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_timer.h>

#ifdef SDL_PLATFORM_IMGUI
#include "imgui_impl_sdl3.h"
#endif

#ifdef __AVX2__
#include "immintrin.h"
#endif

// Forward declarations
static double get_frame_time();
static void handle_keyboard_events(const SDL_Event &Event);
static void handle_mouse_events(const SDL_Event &Event);
static void handle_window_events(platform_context_t *ctx);
static void handle_all_events(platform_context_t *ctx);
//

static SDL_Event event;
static double last_frame;

void platform_delay(platform_context_t *ctx, unsigned int delay)
{
    SDL_Delay(delay);
    return;
}

static void handle_keyboard_events(const SDL_Event &Event)
{
    if(Event.key.scancode == SDL_SCANCODE_ESCAPE) {
    }
}

static void handle_mouse_events(const SDL_Event &Event)
{
    switch(Event.type) {
    case SDL_EVENT_MOUSE_MOTION:      break;
    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_MOUSE_BUTTON_DOWN: break;
    default:                          break;
    }
}

static void handle_window_events(platform_context_t *ctx)
{
    // Resize window
    SDL_GetWindowSize(ctx->Window, &ctx->DisplaySettings.width, &ctx->DisplaySettings.height);
    SDL_UpdateWindowSurface(ctx->Window);
}

static void handle_all_events(platform_context_t *ctx)
{
    while(SDL_PollEvent(&event)) {
        // ImGui_ImplSDL3_ProcessEvent(&event);
        switch(event.type) {
        case SDL_EVENT_QUIT:              break;
        case SDL_EVENT_WINDOW_RESIZED:    handle_window_events(ctx); break;
        case SDL_EVENT_KEY_DOWN:          /* FALLTHROUGH */
        case SDL_EVENT_KEY_UP:            handle_keyboard_events(event); break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN: /* FALLTHROUGH */
        case SDL_EVENT_MOUSE_BUTTON_UP:   /* FALLTHROUGH */
        case SDL_EVENT_MOUSE_MOTION:      handle_mouse_events(event); break;
        default:                          break;
        }
    }
}

int platform_init(platform_context_t *ctx)
{
    SDL_SetAppMetadata(ctx->MetaData.title, ctx->MetaData.version, ctx->MetaData.identifier);
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    SDL_WindowFlags flags = 0;
    SDL_Window *window =
        SDL_CreateWindow(ctx->MetaData.title, ctx->DisplaySettings.width, ctx->DisplaySettings.height, flags);
    if(window) {
        ctx->Window = window;
        return 0;
    }
    return -1;
}

#ifdef __AVX2__
static inline void update_input_simd256(input_t *Input)
{
    const bool *key = SDL_GetKeyboardState(NULL);

    const size_t chunks = SDL_SCANCODE_COUNT / 32;
    for(size_t i = 0; i < chunks; ++i) {
        // Load 32 bytes of current and previous state
        __m256i current = _mm256_loadu_si256((__m256i *)(Input->keyboard.key_state + i * 32));
        // __m256i previous = _mm256_loadu_si256((__m256i *)(Input->keyboard.prev_key_state + i * 32));

        // Store current as next frame's previous
        _mm256_storeu_si256((__m256i *)(Input->keyboard.prev_key_state + i * 32), current);
    }
}
#endif // __AVX2__

static inline void update_input_simd64(input_t *Input)
{
    const bool *key = SDL_GetKeyboardState(NULL);

    const uint64_t *current = (const uint64_t *)key;
    uint64_t *previous = (uint64_t *)Input->keyboard.prev_key_state;
    uint64_t *pressed = (uint64_t *)Input->keyboard.key_presssed;
    uint64_t *released = (uint64_t *)Input->keyboard.key_released;

    const int chunks = (SDL_SCANCODE_COUNT + 7) / 8;

    for(int i = 0; i < chunks; ++i) {
        uint64_t curr = current[i];
        uint64_t prev = previous[i];

        pressed[i] = curr & ~prev;
        released[i] = ~curr & prev;
        previous[i] = curr;
    }
}

int platform_init_context(platform_context_t *ctx)
{
    ctx->DisplaySettings = {
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
    ctx->MetaData = { .identifier = "", .version = "0.1", .title = "title" };
    return 0;
}

static double get_frame_time()
{
    double current_frame = (double)SDL_GetTicks() / 1000;
    last_frame = current_frame;
    return current_frame - last_frame;
}

void process_input(input_t *Input)
{
#ifdef __AVX2__
    update_input_simd256(Input);
    return;
#else
    update_input_simd64(Input);
    return;
#endif
}

void platform_update(platform_context_t *ctx)
{
    ctx->FrameMetrics.frame_time = get_frame_time();
    process_input(&ctx->Input);
    handle_all_events(ctx);
}
