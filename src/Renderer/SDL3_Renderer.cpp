#include "SDL3_Renderer.h"
#include <SDL3/SDL_render.h>
#include <stddef.h>
#include <stdio.h>

#ifdef CXX_CLASS_IMPL

#else

int render_init(renderer_t *Renderer, SDL_Window *Window)
{
    SDL_Renderer *renderer = SDL_CreateRenderer(Window, NULL);
    if(renderer) {
        Renderer->Renderer = renderer;
        SDL_SetRenderVSync(renderer, 1);
        return 0;
    }
    printf("[CRITICAL] Could not initialize SDL Renderer\n");
    return -1;
}

#endif
