#include "ui.h"
#include "../hash.h"
#include "../math.h"
#include <assert.h>

bool UI::button(const char *id_str, void *texture, void *hover_texture, void *clicked_texture)
{
    uint32_t id = hash_string(id_str);
    Layout *layout = top_layout();
    vec2_t position = layout->space_available();
    render_texture(m_renderer, texture, position);
    return false;
}

vec2_t UI::Layout::space_available()
{
    vec2_t result = { 0, 0 };
    return result;
}

UI::UI(void *renderer)
{
    m_renderer = renderer;
}

void UI::push_layout(Layout layout)
{
    m_layouts[m_layout_count++] = layout;
}

UI::Layout UI::pop_layout()
{
    return m_layouts[--m_layout_count];
}

UI::Layout *UI::top_layout()
{
    if(m_layout_count > 0) {
        return &m_layouts[m_layout_count - 1];
    }
    return nullptr;
}

void UI::Layout::push_ui_element(vec2_t element_size)
{
    switch(type) {
    case Type::HORIZONTAL:
        size.x = size.x + element_size.x + padding;
        size.y = MAX(size.y, element_size.y);
        break;
    case Type::VERTICAL:
        size.y = size.y + element_size.y + padding;
        size.x = MAX(size.x, element_size.x);
        break;
    }
}

void UI::begin(vec2_t position, float padding)
{
    Layout layout = {};
    layout.type = Layout::Type::HORIZONTAL;
    layout.position = position;
    layout.size = {};
    layout.padding = padding;
    push_layout(layout);
}

void UI::begin_layout(Layout::Type type, float padding)
{
    auto prev = top_layout();
    assert(prev != nullptr);

    Layout next = {};
    next.type = type;
    next.position = prev->space_available();
    next.size = {};
    next.padding = padding;
    push_layout(next);
}
