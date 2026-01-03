#include "EventQueue.h"
#include "arena.h"
#include <string.h>

EventList_t *create_event_list(arena_t *arena)
{
    EventList_t *list = (EventList_t *)arena_alloc(arena, sizeof(EventList_t));
    event_clear(list);
    return list;
}

void event_push(EventList_t *list, Event event)
{
    if(list->count < MAX_EVENTS) {
        list->events[list->count] = event;
        list->count++;
    }
}

int event_poll(EventList_t *list, Event *event)
{
    if(list->count > 0) {
        list->count--;
        event = &list->events[list->head];
        list->head++;
        return 1;
    }
    event_clear(list);
    return 0;
}

Event *event_poll(EventList_t *list)
{
    if(list->count > list->head) {
        Event *result = &list->events[list->head];
        list->head++;
        return result;
    }
    event_clear(list);
    return NULL;
}

Event event_pop(EventList_t *list)
{
    if(list->count > 0) {
        Event event = list->events[list->head];
        list->head = (list->head + 1) % MAX_EVENTS;
        list->count--;
        return event;
    }
    Event empty = { EVENT_INVALID, {} };
    return empty;
}

void event_clear(EventList_t *list)
{
    memset(list, 0, sizeof(EventList_t));
}

// TODO: Enum to string
void event_debug_print(Event *event)
{
    switch(event->type) {
    default: break;
    }
}

#endif
