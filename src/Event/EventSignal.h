#ifndef EVENT_SIGNAL_H
#define EVENT_SIGNAL_H

#define MAX_SUBSCRIBERS 32

#ifdef CXX_CLASS_IMPL
#else

struct Arena_t;

typedef enum {
    MSG_COUNT,
} MessageType;

typedef struct Message {
    MessageType type;
    // TODO: Add union for message data.
    union {
        void *data;
    } data;
} Message;

typedef void (*MessageHandler)(Message message, void *user_data);

typedef struct {
    MessageHandler handler;
    void *user_data; // optional state/context
} Subscriber;

typedef struct MessageChannel {
    MessageType type;
    Subscriber subscribers[MAX_SUBSCRIBERS];
    int count;
} MessageChannel;

// MessageChannel channels[MSG_COUNT];

// TODO: generic allocator for C-style
MessageChannel *create_message_channels(Arena_t *arena);
void subscribe(MessageType type, MessageHandler handler, void *user_data);
void publish(Message message);

#endif // CXX_CLASS_IMPL
#endif // EVENT_SIGNAL_H
