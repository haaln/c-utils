#ifndef EVENT_QUEUE_H
#define EVENT_QUEUE_H

#define MAX_EVENTS 32

#include <stdint.h>

struct arena_t;

// NOTE: Define events inside Events.def as X-macros
typedef enum {
#define X(event) event,
#include "Events.def"
#undef X
} EventType;

typedef struct Event {
    EventType type;
    uint32_t param0;
    union {

        struct {
            uint32_t param1;
            uint32_t param2;
        } DATA;

        void *ptr;
    };
} Event;

typedef struct EventList_t {
    Event events[MAX_EVENTS];
    unsigned short head;
    unsigned short tail;
    unsigned short count;
} EventList_t;

EventList_t *create_event_list(arena_t *arena);
Event *event_poll(EventList_t *list);
Event event_pop(EventList_t *list);
void event_clear(EventList_t *list);
void event_push(EventList_t *list, Event event);
void event_debug_print(Event *event);

#include "string.h"
static inline bool event_id_from_string(const char *s, EventType *out)
{
#define X(name)                                                                                                        \
    if(strcmp(s, #name) == 0) {                                                                                        \
        *out = EventType::name;                                                                                        \
        return true;                                                                                                   \
    }
#include "Events.def"
#undef X
    return false;
}

static inline const char *enum_to_str(EventType event)
{
#define X(name)                                                                                                        \
    if(name == event) return #name;
#include "Events.def"
#undef X
    return "";
}

#endif // EVENT_QUEUE_H
