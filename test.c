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
    for (U64 i = 0; i < (1<<10); ++i) {
    //for (U64 i = 0; i < /1; ++i) {
        U32 r = prng_next(&p);

        Usize size = r & 1023;
        Usize align = size-1;
        align |= align >> 1;
        align |= align >> 2;
        align |= align >> 4;
        align |= align >> 8;
        align |= align >> 16;
        align += 1;

        if (size == 0) { continue; }
        //U8* ptr = malloc(align);
        U8* ptr = bump_list_alloc(&b, size, align);
        if (prev != NULL) {
            //assert(prev-ptr >= (I64)size);
            //printf("%lu %lu in %lu\n", size, align, prev-ptr);
        }
        //assert(((Usize)ptr & (align-1)) == 0);

        *ptr = (U8)size;
        prev = ptr;
    }
    bump_list_clear(&b);

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

int test_arena(void) {
    Timer t = timer_start();
    Arena ar = arena_create();
    U64* arena_elements = vm_alloc(ARENA_MAX_ELEMENTS * sizeof(*arena_elements));
    printf("%f\n", timer_elapsed_us(&t));

    U64 n = 256;

    for (U64 i = 0; i < n; ++i) {
        U64 idx = arena_insert(&ar);
        arena_elements[idx] = i;
        //printf("insert at %u\n", arena_insert(&ar));
    }

    for (U64 i = 0; i < n; i += 2) {
        arena_remove(&ar,(ArenaIdx)i);
        //printf("remove at %u\n", i);
    }

    for (U64 i = 0; i < n; i += 4) {
        U64 idx = arena_insert(&ar);
        arena_elements[idx] = i;
        //printf("insert at %u\n", arena_insert(&ar));
    }

    for (U64 i = 1; i < n; i += 8) {
        arena_remove(&ar, (ArenaIdx)i);
        //printf("remove at %u\n", i);
    }

    for (U64 i = 0; i < n/2; i++) {
        U64 idx = arena_insert(&ar);
        arena_elements[idx] = i;
        //printf("insert at %u\n", arena_insert(&ar));
    }
    printf("%f\n", timer_elapsed_us(&t));

    ArenaIter iter = arena_iter(&ar);

    while (true) {
        ArenaIdx idx = arena_iter_next(&iter);
        if (idx == ARENA_INVALID_IDX) { break; }

        printf("%lu at %u\n", arena_elements[idx], idx);
    }

    return 0;
}

int test_vec(void) {
    Vec_2 a = {{ 1.0, 1.0 }};
    Vec_2 b = {{ 2.0, 3.0 }};
    Vec_4 c = {{ 1.0, 2.0, 3.0, 4.0 }};
    Vec_3 s = vec_add_3(c.xyz, c.yzw);
    printf("%f, %f, %f\n", s.x, s.y, s.z);
    return 0;
}

int main(void) {
    return test_vec();
}
