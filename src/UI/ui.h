#ifndef UI_H
#define UI_H

#include "../types.h"

class UI
{
    struct Layout {
        enum class Type { HORIZONTAL, VERTICAL };
        Type type;
        vec2_t position;
        vec2_t size;
        float padding;

        vec2_t space_available();
        void push_ui_element(vec2_t element_size);
    };

    const static size_t MAX_LAYOUT = 256;

    // NOTE: The hot and active variables are there to be able to defer the action until after all the UI elements have
    // been rendered. it generally makes things easier to reason about the UI state. deferring the actions until later
    // is also a consequence of compression oriented programming https://guide.handmadehero.org/code/day196/
    uint32_t m_hot_id;
    uint32_t m_tool_bar_button_id;
    Layout m_layouts[MAX_LAYOUT];
    uint32_t m_layout_count;

    float m_button_scale;
    float m_text_scale;
    float m_tooltip_scale;

    void *m_renderer;

    bool m_mouse_pressed;

    enum class State {
        INACTIVE,
        HOVER,
        ACTIVE,
        CLICKED,
    };

    Layout pop_layout();
    Layout *top_layout();

    void push_layout(Layout layout);

  public:
    UI(void *renderer);

    State update_state(uint32_t id, aabb_t rect);

    void begin(vec2_t position, float padding);
    void begin_layout(Layout::Type kind, float padding);

    void begin_toolbar(uint32_t tool_current, float padding);
    void toolbar_button(vec2_t size, uint32_t id);
    void end_toolbar();

    bool button(const char *id, void *texture, void *hover_texture = nullptr, void *clicked_texture = nullptr);
    void text(const char *text, float scale);
    void tooltip(const char *text);

    void end_layout();
    void end();
};

#endif // UI_H
