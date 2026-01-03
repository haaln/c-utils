#ifndef UTIL_THREADS
#define UTIL_THREADS

/**
 * Utility for multithreading, this is a lock-free single-producer multiple-consumer utility.
 * Highly experimental. This is a simple and general threading utility meant for general purpose computing, it is not
 * adapted for NUMA architectures, nor will it be. Please do not use this in production.
 *
 * Things that could be done:
 * - Create high priority queue and low priority queue mechanism.
 * - work-stealing.
 * - futexes.
 * - CPU pinning.
 */

#include <stdint.h>

#if (__linux__)
#include <pthread.h>
#include <semaphore.h>
using Semaphore = sem_t;
using ThreadHandle = pthread_t;
using atomic_u32 = _Atomic uint32_t;
#else
#include <semaphore> // C++20
#include <threads.h>
#error "Platform not supported"
using atomic_u32 = _Atomic uint32_t;
using Semaphore = counting_semaphore;
using ThreadHandle = thrd_t;
#endif

#ifdef __STDC_NO_THREADS__
#error "threads are required for this program to run"
#endif

#define MAX_TASKS 256

typedef enum ThreadScratchFlags : uint8_t {
    SCRATCH_FIXED = 0x1,        //
    SCRATCH_RESET_FRAME = 0x2,  // Either reset every frame or after task.
    SCRATCH_POWER_OF_TWO = 0x4, //
} ThreadScratchFlags;

struct ArenaAllocator;

struct ThreadPoolCreateInfo {
    ArenaAllocator *arena;
    uint64_t scratchspace_per_thread;
    uint32_t thread_count;
    uint8_t flags;
};

struct ThreadPool {
    struct ThreadWorker;
    static ThreadPool *create(const ThreadPoolCreateInfo *info);

    typedef void(CallbackFN)(void *data, ArenaAllocator *tl_allocator);
    void add_task(CallbackFN callback, void *data);
    void complete_all_tasks();
    bool try_process_task(ThreadWorker *worker);
    void *thread_loop(ThreadWorker *worker);
    void clear_scratch_buffers();

    struct Task {
        CallbackFN *callback;
        void *data;
    };

    struct TaskQueue {
        atomic_u32 tasks_total;
        atomic_u32 tasks_completed;
        atomic_u32 current_task_idx;
        atomic_u32 next_task_idx;
        Semaphore *semaphore;
        Task tasks[MAX_TASKS];
    };

    struct ThreadWorker {
        ThreadPool *pool;
        ThreadHandle *handle;
        ArenaAllocator *scratch;
        uint32_t index;
    };

  private:
    static int static_thread_loop(void *self);

    ArenaAllocator *m_main_thread_scratch;

    uint8_t m_flags;
    uint32_t m_thread_count;
    TaskQueue *m_queue = nullptr; // Global queue where workers come to get tasks.
    ThreadWorker *m_threads[];    // Flexible array of total worker threads.
};

#endif // UTIL_THREADS
