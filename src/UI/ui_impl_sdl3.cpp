#ifndef UI_IMPL
#define UI_IMPL

#include <SDL3/SDL.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>

static bool render_texture(void *Renderer, void *texture, vec2_t position)
{
    SDL_FRect dst = { .x = position.x,
                      .y = position.y,
                      .w = (float)((SDL_Texture *)texture)->w,
                      .h = (float)((SDL_Texture *)texture)->h };
    return SDL_RenderTexture((SDL_Renderer *)Renderer, (SDL_Texture *)texture, NULL, &dst);
}
#else
#error "Only one implementation file allowed"
#endif // UI_IMPL
