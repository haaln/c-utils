#ifndef SDL3_RENDERER_H__
#define SDL3_RENDERER_H__

#ifdef CXX_CLASS_IMPL
#include "Renderer.h"
class SDL_Renderer final : public Renderer
{
  public:
    SDL_Renderer();
    ~SDL_Renderer();

    void begin_frame() override;
    void end_frame() override;
    void init() override;

  private:
}
#else

struct SDL_Window;
struct SDL_Renderer;

typedef struct renderer_t {
    SDL_Renderer *Renderer;
} renderer_t;

void render_begin(renderer_t *renderer);
void render_end(renderer_t *renderer);
void render_draw_2d(renderer_t *renderer);
int render_init(renderer_t *Renderer, SDL_Window *Window);

#endif

#endif // SDL3_RENDERER_H__
