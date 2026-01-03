#include "EventSignal.h"

#ifdef CXX_CLASS_IMPL

#else

void subscribe(MessageChannel *channels, MessageType type, MessageHandler handler, void *user_data)
{
    MessageChannel *ch = &channels[type];
    if(ch->count < MAX_SUBSCRIBERS) {
        ch->subscribers[ch->count++] = (Subscriber){ handler, user_data };
    }
}

void publish(MessageChannel *channels, Message message)
{
    MessageChannel *ch = &channels[message.type];
    for(int i = 0; i < ch->count; ++i) {
        ch->subscribers[i].handler(message, ch->subscribers[i].user_data);
    }
}

// TODO: generic allocator
MessageChannel *create_message_channels(Arena_t *arena)
{
    MessageChannel *channels = nullptr;
    // channels = (MessageChannel *)arena_alloc(arena, sizeof(MessageChannel *) * MSG_COUNT);
    // memset(channels, 0, sizeof(MessageChannel) * MSG_COUNT);
    //
    // for(int i = 0; i < MSG_COUNT; ++i) {
    //     channels[i].type = (MessageType)i;
    // }
    return channels;
}
#endif // CXX_CLASS_IMPL
