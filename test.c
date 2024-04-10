#include "tools.h"

#define STACK_TYPE U32
#include "stack.h"

#define HASH_MAP_TYPE U32
#include "map.h"

int test_bump(void) {
    BumpList b = bump_list_create();
    Prng p = prng_create(0);

    U8* prev = NULL;

    Timer t = timer_start();
    for (U64 i = 0; i < (1<<14); ++i) {
        U32 r = prng_next(&p);

        Usize size = r & 1023;
        if (size == 0) { continue; }
        Usize align = size-1;
        align |= align >> 1;
        align |= align >> 2;
        align |= align >> 4;
        align |= align >> 8;
        align |= align >> 16;
        align += 1;

        //U8* ptr = malloc(align);
        U8* ptr = bump_list_alloc(&b, size, align);
        if (prev != NULL) {
            //assert(prev-ptr >= (I64)size);
            //printf("%lu %lu in %lu\n", size, align, prev-ptr);
        }
        assert(((Usize)ptr & (align-1)) == 0);

        *ptr = (U8)size;
        prev = ptr;
    }
    bump_list_dealloc(&b);

    printf("%fus\n", timer_elapsed_us(&t));

    return 0;
}

int test_set(void) {
    Timer timer = timer_start();
    Set set = set_create(4096);

    Prng p = prng_create(0);

    for (U64 i = 0; i < 256; ++i) {
        U32 r = prng_next(&p);
        set_insert(&set, HASH(r));
    }

    p = prng_create(0);

    for (U64 i = 0; i < 256; ++i) {
        U32 r = prng_next(&p);
        assert((set_lookup(&set, HASH(r)) & 1) == 1);
    }

    for (U64 i = 0; i < 256; ++i) {
        U32 r = prng_next(&p);
        assert((set_lookup(&set, HASH(r)) & 1) == 0);
    }

    p = prng_create(0);

    for (U64 i = 0; i < 256; ++i) {
        U32 r = prng_next(&p);
        assert((set_remove(&set, HASH(r)) & 1) == 1);
    }

    for (U64 i = 0; i < 256; ++i) {
        U32 r = prng_next(&p);
        assert((set_lookup(&set, HASH(r)) & 1) == 0);
    }

    set_dealloc(&set);

    printf("%fus\n", timer_elapsed_us(&timer));

    return 0;
}

int test_map(void) {
    Timer timer = timer_start();
    Map_U32 map = map_create_U32(4096);

    Prng p = prng_create(0);

    for (U64 i = 0; i < 256; ++i) {
        U32 r = prng_next(&p);
        map_insert_U32(&map, HASH(r), r);
    }

    p = prng_create(0);

    for (U64 i = 0; i < 256; ++i) {
        U32 r = prng_next(&p);
        assert(*map_lookup_U32(&map, HASH(r)) == r);
    }

    for (U64 i = 0; i < 256; ++i) {
        U32 r = prng_next(&p);
        assert(map_lookup_U32(&map, HASH(r)) == NULL);
    }

    p = prng_create(0);

    for (U64 i = 0; i < 256; ++i) {
        U32 r = prng_next(&p);
        assert(*map_remove_U32(&map, HASH(r)) == r);
    }

    for (U64 i = 0; i < 256; ++i) {
        U32 r = prng_next(&p);
        assert(map_lookup_U32(&map, HASH(r)) == NULL);
    }

    map_dealloc_U32(&map);

    printf("%fus\n", timer_elapsed_us(&timer));

    return 0;
}

int test_stack(void) {
    Stack_U32 s = stack_create_U32(0);
    Prng p = prng_create(0);

    Timer t = timer_start();

    for (U64 i = 0; i < 1024; ++i) {
        U32 r = prng_next(&p);
        stack_push_U32(&s, r);
    }

    p = prng_create(0);

    for (U64 i = 0; i < 1024; ++i) {
        U32 r = prng_next(&p);
        assert(s.objects[i] == r);
    }

    for (U64 i = 0; i < 1024; ++i) {
        stack_pop_U32(&s);
    }

    assert(s.len == 0);

    printf("%f\n", timer_elapsed_us(&t));

    return 0;
}

int main(void) {
    return test_map();
}
