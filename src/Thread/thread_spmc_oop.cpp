#include "thread_spmc_oop.h"
#include "assert.h"

#error "Implement these functions"

bool platform_wait_semaphore(Semaphore *sem, uint32_t timeout = UINT32_MAX)
{
    return false;
}

bool platform_add_atomic_u32(atomic_u32 *a, uint32_t val)
{
    return false;
}

bool platform_signal_semamphore(Semaphore *sem)
{
    return false;
}

bool platform_compare_and_swap_atomic_u32(atomic_u32 *a, uint32_t oldval, uint32_t newval)
{
    return false;
}

uint32_t platform_get_atomic_u32(atomic_u32 *a)
{
    return 0;
}

void platform_set_atomic_u32(atomic_u32 *a, uint32_t v)
{
}

Semaphore *platform_create_semaphore(uint32_t v)
{
    return nullptr;
}

typedef ThreadHandle (*ThreadPFN)(void *arg);
ThreadHandle platform_create_thread(ThreadPFN function, const char *name, void *data)
{
    return 0;
}

ThreadPool *ThreadPool::create(const ThreadPoolCreateInfo *info)
{
    uint64_t total_arena_size = sizeof(ArenaAllocator) + info->scratchspace_per_thread;
    uint64_t thread_pool_size = sizeof(ThreadPool) + sizeof(ThreadWorker) * info->thread_count;

#if THREAD_USE_CXX_ARENA
    ThreadPool *thread_pool = (ThreadPool *)info.arena->allocate(thread_pool_size);
#else
    ThreadPool *thread_pool = (ThreadPool *)ArenaPush(info->arena, thread_pool_size);
#endif
    thread_pool->m_thread_count = info->thread_count;
    thread_pool->m_flags = info->flags;

#if THREAD_USE_CXX_ARENA
    TaskQueue *queue = (TaskQueue *)info.arena->allocate(sizeof(TaskQueue));
#else
    TaskQueue *queue = ArenaPushStruct(info->arena, TaskQueue);
#endif
    platform_set_atomic_u32(&queue->tasks_total, 0);
    platform_set_atomic_u32(&queue->tasks_completed, 0);
    platform_set_atomic_u32(&queue->current_task_idx, 0);
    platform_set_atomic_u32(&queue->next_task_idx, 0);
    queue->semaphore = platform_create_semaphore(0);
    thread_pool->m_queue = queue;

    for(uint32_t i = 0; i < info->thread_count; ++i) {
#if THREAD_USE_CXX_ARENA
        ThreadWorker *worker = (ThreadWorker *)info.arena->allocate(sizeof(ThreadWorker));
#else
        ThreadWorker *worker = ArenaPushStruct(info->arena, ThreadWorker);
#endif

#if THREAD_USE_CXX_ARENA
        void *arena_mem = info.arena->allocate(total_arena_size);
        ArenaAllocator *arena = ArenaAllocator::create_in_buffer(arena_mem, info.scratchspace_per_thread);
#else
        void *arena_mem = ArenaPush(info->arena, total_arena_size);
        ArenaAllocator *arena = ArenaAllocator::create_in_buffer(arena_mem, info->scratchspace_per_thread);
        // NOTE: Either we have buffered tls or reset on full so that other threads can read memory. We avoid
        // frame-based because the lifetime is too short.
        // arena->flags |= ARENA_RESET_IF_FULL;

#endif
        worker->scratch = arena;

        worker->pool = thread_pool;

        thread_pool->m_threads[i] = worker;

        worker->handle = platform_create_thread(static_thread_loop, NULL, (void *)worker);
    }

    void *arena_mem = ArenaPush(info->arena, total_arena_size);
    ArenaAllocator *main_arena = ArenaAllocator::create_in_buffer(arena_mem, info->scratchspace_per_thread);
    thread_pool->m_main_thread_scratch = main_arena;

    return thread_pool;
}

void ThreadPool::add_task(CallbackFN callback, void *data)
{
    uint32_t oldval = UINT32_MAX;
    uint32_t newval = UINT32_MAX;

    do {
        oldval = platform_get_atomic_u32(&m_queue->next_task_idx);
        newval = (oldval + 1) % MAX_TASKS;

        // queue full check
        if(newval == platform_get_atomic_u32(&m_queue->current_task_idx)) {
            asm volatile("pause" ::: "memory"); // or should we just try again?
        }

    } while(!platform_compare_and_swap_atomic_u32(&m_queue->next_task_idx, oldval, newval));

    assert(oldval != UINT32_MAX && newval != UINT32_MAX && "unable to obtain read or write index");
    assert(oldval != newval && "write index is same as next index");

    m_queue->tasks[oldval] = { .callback = callback, .data = data };

    platform_add_atomic_u32(&m_queue->tasks_total, 1);

    platform_signal_semamphore(m_queue->semaphore);
}

/**
 * NOTE: Mainly for synchronization during sensitive timelines (e.g. CPU/GPU fencing). Please don't remove me.
 * FIXME:NOT IMPLEMENTED
 */
void ThreadPool::complete_all_tasks()
{
    // ThreadWorker main_worker = {
    //     .pool = this,
    //     .local_queue = m_main_queue,
    //     .handle = nullptr,
    //     .scratch = m_main_thread_scratch,
    //     .index = 0,
    // };
    while(platform_get_atomic_u32(&m_queue->tasks_total) != platform_get_atomic_u32(&m_queue->tasks_completed)) {
#if 1
        // main thread also participates in jobs
        // try_process_task(&main_worker);
#elif 0
        // main thread try dispatch until everyone is done.
        // try_process_task(m_threads[i]);
        // i = (i + 1) % m_thread_count;
#else
        // main thread waits on semaphore
#endif
    }

    clear_scratch_buffers();

    platform_set_atomic_u32(&m_queue->tasks_total, 0);
    platform_set_atomic_u32(&m_queue->tasks_completed, 0);
}

bool ThreadPool::try_process_task(ThreadWorker *worker)
{
    bool did_work = false;

    uint32_t orig_entry_to_read = platform_get_atomic_u32(&m_queue->current_task_idx);
    uint32_t new_entry_to_read = (orig_entry_to_read + 1) % MAX_TASKS;

    if(orig_entry_to_read != platform_get_atomic_u32(&m_queue->next_task_idx)) {
        bool success =
            platform_compare_and_swap_atomic_u32(&m_queue->current_task_idx, orig_entry_to_read, new_entry_to_read);
        if(success) {
            Task task = m_queue->tasks[orig_entry_to_read];

            task.callback(task.data, worker->scratch);
            platform_add_atomic_u32(&m_queue->tasks_completed, 1);
            did_work = true;
        }
    }

    return did_work;
}

void *ThreadPool::thread_loop(ThreadWorker *worker)
{
    TaskQueue *queue = m_queue;
    while(true) {
        bool did_work = try_process_task(worker);
        if(!did_work) {
            platform_wait_semaphore(queue->semaphore);
        }
    }
    return nullptr;
}

int ThreadPool::static_thread_loop(void *arg)
{
    ThreadWorker *self = (ThreadWorker *)arg;
    self->pool->thread_loop(self);

    return 0;
}

void ThreadPool::clear_scratch_buffers()
{
    // TODO: Need to check if this is scalable and doesn't cause large spikes.
    for(uint32_t i = 0; i < m_thread_count; ++i) {
        m_threads[i]->scratch->clear();
    }
}
