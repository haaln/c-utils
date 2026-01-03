#include "thread_spmc_pod.h"

#include "stdio.h"
#include <assert.h>

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))
#endif

int get_current_thread_id()
{
#if (_WIN32)
    return GetCurrentThreadId();
#elif (__linux__)
    return (uint64_t)pthread_self();
#elif (__x86_64__)
    asm("mov %%fs:0x10,%0" : "=r"(thread_id));
#else
#error "Platform not supported"
#endif
}

uint32_t atomic_compare_exchange_u32(uint32_t volatile *value, uint32_t new_value, uint32_t expected)
{
#if _WIN32
    return InterlockedCompareExchange((volatile LONG *)value, (LONG)new_value, (LONG)expected);
#elif (__linux__)
    return (uint32_t)__sync_val_compare_and_swap(value, expected, new_value);
#else
#error "Platform not supported"
#endif
}

uint64_t atomic_compare_and_swap_u64(uint64_t volatile *value, uint64_t new_value, uint64_t expected)
{
#if _WIN32
    return InterlockedCompareExchange((volatile LONG *)value, (LONG)new_value, (LONG)expected);
#elif (__linux__)
    return __sync_bool_compare_and_swap(value, expected, new_value);
#else
#error "Platform not supported"
#endif
}

uint64_t atomic_exchange_u64(uint64_t volatile *value, uint64_t new_value)
{
#if _WIN32
    return InterlockedExchange64((__int64 volatile *)value, new_value);
#elif (__linux__)
    return __sync_lock_test_and_set(value, new_value);
#else
#error "Platform not supported"
#endif
}

uint64_t atomic_add_u64(uint64_t volatile *value, uint64_t number)
{
#if _WIN32
    return InterlockedExchangeAdd64((__int64 volatile *)value, number);
#elif (__linux__)
    return __sync_fetch_and_add(value, number);
#else
#error "Platform not supported"
#endif
}

void semaphore_release(task_queue_t *queue)
{
#if _WIN32
    ReleaseSemaphore(queue->semaphore_handle, 1, NULL);
#elif (__linux__)
    sem_post(&queue->semaphore_handle);
#else
#error "Platform not supported"
#endif
}

void thread_semaphore_wait(task_queue_t *queue)
{
#if _WIN32
    WaitForSingleObjectEx(queue->semaphore_handle, INFINITE, FALSE);
#elif (__linux__)
    sem_wait(&queue->semaphore_handle);
#else
#error "Platform not supported"
#endif
}

void semaphore_create(task_queue_t *queue)
{
#if _WIN32
    queue->semaphore_handle = CreateSemaphore(NULL, 0, THREAD_COUNT, NULL);
#elif (__linux__)
    if(sem_init(&queue->semaphore_handle, 0, 0) != 0) {
        perror("Failed to initialize semaphore\n");
    }
#else
#error "Platform not supported"
#endif
}

void queue_add_entry(task_queue_t *queue, callback_pfn *callback, void *data)
{
    uint32_t current_task_idx = (queue->current_task_idx + 1) % ARRAY_SIZE(queue->tasks);
    assert(current_task_idx != queue->next_task_idx);
    task_t *task = queue->tasks + queue->current_task_idx;
    task->callback = callback;
    task->data = data;
    ++queue->tasks_total;

    __atomic_thread_fence(__ATOMIC_SEQ_CST);

    queue->current_task_idx = current_task_idx;
    semaphore_release(queue);
}

bool thread_process_next_task(task_queue_t *queue)
{
    bool should_sleep = false;
    uint32_t orig_entry_to_read = queue->next_task_idx;
    uint32_t new_entry_to_read = (orig_entry_to_read + 1) % ARRAY_SIZE(queue->tasks);

    if(orig_entry_to_read != queue->current_task_idx) {
        uint32_t index = atomic_compare_exchange_u32(&queue->next_task_idx, new_entry_to_read, orig_entry_to_read);
        if(index == orig_entry_to_read) {
            task_t task = queue->tasks[index];

            task.callback(task.data);
            atomic_add_u64((volatile uint64_t *)&queue->tasks_completed, 1);
        }
    } else {
        should_sleep = true;
    }
    return should_sleep;
}

void *thread_process(void *arg)
{
    thread_startup_info_t *Thread = (thread_startup_info_t *)arg;
    task_queue_t *queue = Thread->queue;

    while(true) {
        if(thread_process_next_task(queue)) {
            thread_semaphore_wait(queue);
        }
    }
    return NULL;
}

void complete_all_work(task_queue_t *queue)
{
    while(queue->tasks_total != queue->tasks_completed) {
        thread_process_next_task(queue);
    }

    queue->tasks_total = 0;
    queue->tasks_completed = 0;
}

void thread_create_queue(task_queue_t *queue, int64_t thread_count, thread_startup_info_t *startups)
{
    queue->tasks_total = 0;
    queue->tasks_completed = 0;
    queue->current_task_idx = 0;
    queue->next_task_idx = 0;
    semaphore_create(queue);

    for(int64_t thread_index = 0; thread_index < thread_count; ++thread_index) {
#if _WIN32
        thread_startup_info_t *startup = startups + thread_index;
        DWORD thread_id;
        startup->queue = queue;
        HANDLE ThreadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)thread_process, startup, 0, &thread_id);
        if(!ThreadHandle) {
            perror("Failed to create thread");
            exit(EXIT_FAILURE);
        }
        CloseHandle(ThreadHandle);
#elif (__linux__)
        pthread_t thread_id;
        thread_startup_info_t *startup = startups + thread_index;
        startup->queue = queue;
        if(pthread_create(&thread_id, NULL, thread_process, startup) != 0) {
            perror("Failed to create thread");
        }
#else
#error "Platform not supported"
#endif
    }
}
