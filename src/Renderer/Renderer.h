#ifndef RENDERER_H__
#define RENDERER_H__

class Renderer
{
  public:
    Renderer();
    virtual ~Renderer() = 0;

    virtual void begin_frame() = 0;
    virtual void end_frame() = 0;
    virtual void init() = 0;

  private:
};

#endif // !RENDERER_H__
