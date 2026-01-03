// pool_tests.cpp
#define CATCH_CONFIG_MAIN
// #include <catch2/catch.hpp>
#include <catch2/catch_all.hpp>

extern "C" {
// #include "../mem.h"
#include "../pool.h"

#include <assert.h>
#include <cstddef>
#include <cstdint>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
}

// Helper: align forward (you probably already have these)
static inline uintptr_t align_forward_uintptr(uintptr_t ptr, size_t align)
{
    return (ptr + align - 1) & ~(align - 1);
}
static inline size_t align_forward_size(size_t size, size_t align)
{
    return (size + align - 1) & ~(align - 1);
}

TEST_CASE("pool_init rejects invalid parameters", "[pool_init]")
{

    REQUIRE_THROWS_WITH(pool_init(0, 10), "Chunk size is zero");
    REQUIRE_THROWS_WITH(pool_init(4, 0), "chunk_count is zero");
    // REQUIRE_THAT(pool_init(4, 10), Catch::Matchers::NotNull()); // valid
}

TEST_CASE("pool_init with user buffer works and respects alignment", "[pool_init][user_buffer]")
{
    alignas(64) char buffer[2048]{};
    pool_t *pool = pool_init(16, 32, buffer, sizeof(buffer), 64);

    REQUIRE(pool != nullptr);
    REQUIRE(pool->is_heap_based == false);
    REQUIRE(pool->chunk_size == 16);
    REQUIRE(pool->chunk_count == 32);
    REQUIRE(pool->capacity == 16 * 32);
    REQUIRE(pool->chunks_in_use == 0);
    REQUIRE(pool->head != nullptr);

    // pool header must be inside the user buffer and properly aligned
    uintptr_t pool_addr = (uintptr_t)pool;
    REQUIRE(pool_addr >= (uintptr_t)buffer);
    REQUIRE(pool_addr < (uintptr_t)(buffer + sizeof(buffer)));
    REQUIRE((pool_addr & 63) == 0); // 64-byte aligned

    pool_destroy(pool);
}

TEST_CASE("pool_init dynamic allocation works", "[pool_init][heap]")
{
    pool_t *pool = pool_init(32, 100);
    REQUIRE(pool != nullptr);
    REQUIRE(pool->is_heap_based == true);
    REQUIRE(pool->chunk_count == 100);
    REQUIRE(pool->chunks_in_use == 0);
    REQUIRE(pool->head != nullptr);

    pool_destroy(pool);
}

TEST_CASE("pool_alloc / pool_free basic round-trip", "[alloc][free]")
{
    pool_t *pool = pool_init(64, 8);

    void *p1 = pool_alloc(pool);
    void *p2 = pool_alloc(pool);
    REQUIRE(p1 != nullptr);
    REQUIRE(p2 != nullptr);
    REQUIRE(p1 != p2);

    REQUIRE(pool->chunks_in_use == 2);

    pool_free(pool, p1);
    pool_free(pool, p2);

    REQUIRE(pool->chunks_in_use == 0);
    REQUIRE(pool->head != nullptr);

    pool_destroy(pool);
}

TEST_CASE("pool_alloc returns nullptr when exhausted", "[alloc]")
{
    pool_t *pool = pool_init(16, 3);

    void *a = pool_alloc(pool);
    void *b = pool_alloc(pool);
    void *c = pool_alloc(pool);
    REQUIRE(a != nullptr);
    REQUIRE(b != nullptr);
    REQUIRE(c != nullptr);

    void *d = pool_alloc(pool);
    REQUIRE(d == nullptr); // exhausted

    REQUIRE(pool->chunks_in_use == 3);

    pool_free(pool, b);
    void *e = pool_alloc(pool);
    REQUIRE(e == b); // should reuse b

    pool_destroy(pool);
}

TEST_CASE("pool_free rejects invalid pointers (bounds + alignment)", "[free][validation]")
{
    pool_t *pool = pool_init(32, 10);

    void *valid = pool_alloc(pool);

    // 1. nullptr
    REQUIRE_NOTHROW(pool_free(pool, nullptr));

    // 2. Pointer outside pool
    char external[100];
    REQUIRE_NOTHROW(pool_free(pool, external)); // should just return (assert in debug)

    // 3. Pointer inside pool but not at chunk boundary
    REQUIRE_NOTHROW(pool_free(pool, (char *)valid + 8));

    // 4. Double free
    pool_free(pool, valid);
    REQUIRE_NOTHROW(pool_free(pool, valid)); // should not crash, just ignore or assert

    pool_destroy(pool);
}

TEST_CASE("pool_free_all resets everything", "[free_all]")
{
    pool_t *pool = pool_init(48, 5);

    void *ptrs[5];
    for(int i = 0; i < 5; ++i)
        ptrs[i] = pool_alloc(pool);

    REQUIRE(pool->chunks_in_use == 5);

    pool_free_all(pool);

    REQUIRE(pool->chunks_in_use == 0);
    REQUIRE(pool->head != nullptr);

    // Can allocate all again
    for(int i = 0; i < 5; ++i) {
        void *p = pool_alloc(pool);
        REQUIRE(p != nullptr);
    }
    REQUIRE(pool_alloc(pool) == nullptr); // exhausted again

    pool_destroy(pool);
}

TEST_CASE("pool_destroy detects leaks in debug", "[destroy][leak]")
{
    pool_t *pool = pool_init(16, 4);
    pool_alloc(pool);
    pool_alloc(pool);

    // In debug builds: should assert on destroy due to leak
#ifndef NDEBUG
    REQUIRE_THROWS_WITH(pool_destroy(pool), "memory leak detected");
#else
    REQUIRE_NOTHROW(pool_destroy(pool));
#endif
}

TEST_CASE("LIFO order: recently freed block is reused first", "[lifo]")
{
    pool_t *pool = pool_init(32, 3);

    void *a = pool_alloc(pool);
    void *b = pool_alloc(pool);
    void *c = pool_alloc(pool);

    pool_free(pool, b);
    void *reused = pool_alloc(pool);
    REQUIRE(reused == b); // LIFO!

    pool_destroy(pool);
}

TEST_CASE("Mixed stack and heap pools don't interfere", "[mixed]")
{
    alignas(32) char buf[1024];
    pool_t *stack_pool = pool_init(64, 8, buf, sizeof(buf));
    pool_t *heap_pool = pool_init(64, 8);

    void *s1 = pool_alloc(stack_pool);
    void *h1 = pool_alloc(heap_pool);

    REQUIRE(s1 != h1);

    // Free into wrong pool → should be rejected (no crash)
    REQUIRE_NOTHROW(pool_free(stack_pool, h1));
    REQUIRE_NOTHROW(pool_free(heap_pool, s1));

    pool_free(stack_pool, s1);
    pool_free(heap_pool, h1);

    pool_destroy(stack_pool);
    pool_destroy(heap_pool);
}

TEST_CASE("Large number of allocations (stress)", "[stress]")
{
    pool_t *pool = pool_init(128, 1000);

    std::vector<void *> ptrs;
    for(int i = 0; i < 1000; ++i) {
        void *p = pool_alloc(pool);
        REQUIRE(p != nullptr);
        ptrs.push_back(p);
    }
    REQUIRE(pool_alloc(pool) == nullptr);

    for(void *p : ptrs)
        pool_free(pool, p);

    REQUIRE(pool->chunks_in_use == 0);

    pool_destroy(pool);
}
