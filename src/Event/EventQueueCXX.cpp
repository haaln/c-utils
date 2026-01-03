

struct UICardClicked {
    u8 slot;
    CardIntent intent;
};

struct GameTurnEnded {
    PlayerId whose_turn;
};

struct CardPlayed {
    PlayerId player;
    CardId card;
    u8 from_slot;
};

using GameEvent = std::variant<
    UICardClicked,
    GameTurnEnded,
    CardPlayed,
    // ...
    >;

class EventBus
{
    struct Queue {
        std::vector<GameEvent> events;
        i32 priority = 0;
    };
    std::vector<Queue> phased_queues[PhaseCount];

  public:
    void Push(GameEvent &&e, Phase phase = Phase::Default, i32 priority = 0);
    void ProcessAll(); // called once per tick in deterministic order
    void Register(GameEvent);

    // event_bus.Register<UICardClicked>([](const UICardClicked &e) {
    //     if(!GameState::CanAct(CurrentPlayer)) return;
    //     CommandBuffer::Push<PlayCardCommand>({ .slot = e.slot, .intent = e.intent });
    // });
    //
    // event_bus.Register<CardPlayed>([&](const CardPlayed &e) {
    //     FXSystem::Spawn(e.card, e.player);
    //     AudioSystem::Play("card_slam");
    // });
};
enum Phase {
    PHASE_INPUT,      // raw UI events go here
    PHASE_GAME_LOGIC, // card resolution, turn changes
    PHASE_ANIMATION,  // triggers VFX/SFX after state settled
    PHASE_UI_UPDATE,  // UI reacts to new game state
    PHASE_END_OF_FRAME
};

class EventQueue
{
  public:
    EventQueue();
    ~EventQueue();

    void clear();
    void push(Event *event);
    Event pop();
    Event *poll();

  private:
    Events events[MAX_EVENTS];
    unsigned short m_head;
    unsigned short m_tail;
    unsigned short m_count;
}
