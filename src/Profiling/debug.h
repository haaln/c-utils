#ifndef UTIL_DEBUG_H
#define UTIL_DEBUG_H

#ifndef __cplusplus
#error "C++ must be enabled"
#endif

#include <stdio.h>
#include <x86intrin.h>

#include "thread.h"

#include "types.h"

typedef struct debug_info_t {

} debug_info_t;

typedef enum DEBUG_TYPE {
  DEBUG_TYPE_UNKNOWN,

  DEBUG_TYPE_NAME,

  DEBUG_TYPE_FRAME_MARKER,
  DEBUG_TYPE_BEGIN_BLOCK,
  DEBUG_TYPE_END_BLOCK,

  DEBUG_TYPE_OpenDataBlock,
  DEBUG_TYPE_CloseDataBlock,
  DEBUG_TYPE_SetHUD,

  DEBUG_TYPE_U8,
  DEBUG_TYPE_U16,
  DEBUG_TYPE_U32,
  DEBUG_TYPE_U64,
  DEBUG_TYPE_S8,
  DEBUG_TYPE_S16,
  DEBUG_TYPE_S32,
  DEBUG_TYPE_S64,
  DEBUG_TYPE_F32,
  DEBUG_TYPE_F64,
  DEBUG_TYPE_SIZE_T,
  DEBUG_TYPE_STR,

  DEBUG_TYPE_BITMAP_ID,

  DEBUG_TYPE_ThreadIntervalGraph,
  DEBUG_TYPE_FrameBarGraph,
  DEBUG_TYPE_LastFrameInfo,
  DEBUG_TYPE_FrameSlider,
  DEBUG_TYPE_TopClocksList,
  DEBUG_TYPE_FunctionSummary,
  DEBUG_TYPE_MemoryByArena,
  DEBUG_TYPE_MemoryByFrame,
  DEBUG_TYPE_MemoryBySize,

  DEBUG_TYPE_COUNT
} DEBUG_TYPE;

struct debug_event {
  unsigned long Clock;
  char *GUID;
  unsigned short ThreadID;
  unsigned short CoreIndex;
  char Type;
  char *Name;

  union {
    debug_event *Value_debug_event;

    char *Value_string_ptr;
    unsigned int Value_b32;
    int Value_s32;
    unsigned int Value_u32;
  };
};

struct debug_table_t {
  debug_event EditEvent;
  unsigned int RecordIncrement;

  // TODO(casey): No attempt is currently made to ensure that the final
  // debug records being written to the event array actually complete
  // their output prior to the swap of the event array index.
  unsigned int CurrentEventArrayIndex;
  // TODO(casey): This could actually be a u32 atomic now, since we
  // only need 1 bit to store which array we're using...
  unsigned long volatile EventArrayIndex_EventIndex;
  debug_event Events[2][16 * 65536];
};

class TimedBlock {
  TimedBlock(char *GUID, char *Name);
  ~TimedBlock();
};

class debug_data_block {
  debug_data_block(char *GUID, char *Name);
  ~debug_data_block(void);
};

extern debug_table_t *GlobalDebugTable;

#define DEBUG_NAME__(A, B, C) A "(" #B ")" #C
#define DEBUG_NAME_(A, B, C) DEBUG_NAME__(A, B, C)
#define DEBUG_NAME(Name) DEBUG_NAME_(__FILE__, __LINE__, __COUNTER__)

#define _RECORD_DEBUG_EVENT(EventType, GUIDInit, NameInit)                     \
  unsigned long ArrayIndex_EventIndex =                                        \
      __sync_fetch_and_add(&GlobalDebugTable->EventArrayIndex_EventIndex,      \
                           GlobalDebugTable->RecordIncrement);                 \
  u32 EventIndex = ArrayIndex_EventIndex & 0xFFFFFFFF;                         \
  Assert(EventIndex < ARRAY_SIZE(GlobalDebugTable->Events[0]));                \
  debug_event *Event =                                                         \
      GlobalDebugTable->Events[ArrayIndex_EventIndex >> 32] + EventIndex;      \
  Event->Clock = __rdtsc();                                                    \
  Event->Type = (char)EventType;                                               \
  Event->CoreIndex = 0;                                                        \
  Event->ThreadID = (unsigned short)get_thread_id();                           \
  Event->GUID = GUIDInit;                                                      \
  Event->Name = NameInit

//
#define FRAME_MARKER(SecondsElapsedInit)                                       \
  {                                                                            \
    _RECORD_DEBUG_EVENT(DEBUG_TYPE_FRAME_MARKER, DEBUG_NAME("Frame Marker"),   \
                        "Frame Marker");                                       \
    Event->Value_r32 = SecondsElapsedInit;                                     \
  }

#define TIMED_BLOCK__(GUID, Name) TimedBlock TimedBlock_##Number(GUID, Name)
#define TIMED_BLOCK_(GUID, Name) TIMED_BLOCK__(GUID, Name)
#define TIMED_BLOCK(Name) TIMED_BLOCK_(DEBUG_NAME(Name), Name)
#define TIMED_FUNCTION(...)                                                    \
  TIMED_BLOCK_(DEBUG_NAME(__FUNCTION__), (char *)__FUNCTION__)

#define BEGIN_BLOCK_(GUID, Name)                                               \
  {                                                                            \
    _RECORD_DEBUG_EVENT(DEBUG_TYPE_BEGIN_BLOCK, GUID, Name);                   \
  }
#define END_BLOCK_(GUID, Name)                                                 \
  {                                                                            \
    _RECORD_DEBUG_EVENT(DEBUG_TYPE_END_BLOCK, GUID, Name);                     \
  }

#define BEGIN_BLOCK(Name) BEGIN_BLOCK_(DEBUG_NAME(Name), Name)
#define END_BLOCK() END_BLOCK_(DEBUG_NAME("END_BLOCK_"), "END_BLOCK_")

#define DEBUG_VALUE(Value)                                                     \
  {                                                                            \
    _RECORD_DEBUG_EVENT(DEBUG_TYPE_UNKNOWN, DEBUG_NAME(#Value), #Value);       \
    DEBUGValueSetEventData(Event, Value, (void *)&(Value));                    \
  }

#define DEBUG_DATA_BLOCK(Name)                                                 \
  debug_data_block DataBlock__(DEBUG_NAME(Name), Name)
#define DEBUG_BEGIN_DATA_BLOCK(Name)                                           \
  RecordDebugEvent(DebugType_OpenDataBlock, DEBUG_NAME(Name), Name)
#define DEBUG_END_DATA_BLOCK(Name)                                             \
  RecordDebugEvent(DebugType_CloseDataBlock, DEBUG_NAME("End Data Block"),     \
                   "End Data Block")

#endif // UTIL_DEBUG_H

#define UTIL_DEBUG_IMPLEMENTATION

#ifdef UTIL_DEBUG_IMPLEMENTATION
#undef UTIL_DEBUG_IMPLEMENTATION

TimedBlock::TimedBlock(char *GUID, char *Name) { BEGIN_BLOCK_(GUID, Name); }
TimedBlock::~TimedBlock() { END_BLOCK(); }
debug_data_block::debug_data_block(char *GUID, char *Name) {
  _RECORD_DEBUG_EVENT(DEBUG_TYPE_OpenDataBlock, GUID, Name);
  // Event->DebugID = ID;
};

debug_data_block::~debug_data_block(void) {
  _RECORD_DEBUG_EVENT(DEBUG_TYPE_CloseDataBlock, DEBUG_NAME("End Data Block"),
                      "End Data Block");
};

#endif // UTIL_DEBUG_IMPLEMENTATION
