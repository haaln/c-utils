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
 * - thread-local storage.
 * - CPU pinning.
 */

#include <stdint.h>

#if (__linux__)
#include <pthread.h>
#include <semaphore.h>
using semaphore_t = sem_t;
using thread_handle_t = pthread_t;
using atomic_u32 = _Atomic uint32_t;
#else
#include <semaphore> // C++20
#include <threads.h>
#error "Platform not supported"
using atomic_uint32_t = _Atomic uint32_t;
using Semaphore = counting_semaphore;
using ThreadHandle = thrd_t;
#endif

#if (_WIN32)
using semaphore_t = HANDLE;
#endif

#ifdef __STDC_NO_THREADS__
#error "threads are required for this program to run"
#endif

#define MAX_TASKS 256

struct task_queue_t;

#define THREAD_CALLBACK(function_name) void function_name(void *arg)

typedef void(callback_pfn)(void *data);

typedef struct task_t {
    callback_pfn *callback;
    void *data;
} task_t;

typedef struct task_queue_t {
    uint32_t volatile tasks_total;
    uint32_t volatile tasks_completed;
    uint32_t volatile current_task_idx;
    uint32_t volatile next_task_idx;
    semaphore_t semaphore_handle;
    task_t tasks[MAX_TASKS];
} task_queue_t;

typedef struct thread_startup_info_t {
    task_queue_t *queue;
    uint64_t handle;
    uint64_t index;
} thread_startup_info_t;

int32_t thread_get_current_id();
uint32_t thread_get_id();

uint32_t atomic_compare_exchange_u32(uint32_t volatile *value, uint32_t new_value, uint32_t expected);
uint64_t atomic_compare_and_swap_u64(uint64_t volatile *value, uint64_t new_value, uint64_t expected);
bool atomic_compare_and_swap_u64_bool(uint64_t volatile *value, uint64_t new_value, uint64_t expected);
uint64_t atomic_exchange_u64(uint64_t volatile *value, uint64_t new_value);
uint64_t atomic_add_u64(uint64_t volatile *value, uint64_t number);

void thread_semaphore_release(task_queue_t *queue);
void thread_semaphore_wait(task_queue_t *queue);
void thread_semaphore_create(task_queue_t *queue);

void thread_add_task(task_queue_t *queue, callback_pfn *callback, void *data);
void thread_complete_all_tasks(task_queue_t *queue);
bool thread_process_next_task(task_queue_t *queue);
void thread_create_task_queue(task_queue_t *queue, int64_t thread_count, thread_startup_info_t *startups);
void *thread_process(void *arg);

#endif // UTIL_THREADS
