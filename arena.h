#ifndef TOOLS_H
#error "please include tools.h before including arena.h"
#else

#ifndef ARENA_TYPE
#error "ARENA_TYPE must be defined before including arena.h"
#else

#define NAME(a) CAT2(a, ARENA_TYPE)

#define ARENA NAME(Arena)

typedef struct {
    ArenaTracking tracking;
    ARENA_TYPE* backing;
} ARENA;

ARENA NAME(arena_create)() {
    return (ARENA) {
        .tracking = arena_tracking_create(),
        .backing = vm_alloc(ARENA_MAX_ELEMENTS * sizeof(ARENA_TYPE)),
    };
}

ArenaKey NAME(arena_insert)(ARENA* ar, ARENA_TYPE e) {
    ArenaKey k = arena_tracking_insert(&ar->tracking);
    ar->backing[k.idx] = e;
    return k;
}

void NAME(arena_remove)(ARENA* ar, ArenaKey k) {
    arena_tracking_remove(&ar->tracking, k);
}

ARENA_TYPE* NAME(arena_lookup)(ARENA* ar, ArenaKey k) {
    if (!arena_tracking_key_valid(&ar->tracking, k)) return NULL;
    return &ar->backing[k.idx];
}

void NAME(arena_dealloc)(ARENA* ar) {
    vm_dealloc(ar->backing, ARENA_MAX_ELEMENTS * sizeof(ARENA_TYPE));
    arena_tracking_dealloc(&ar->tracking);
}

#undef ARENA_TYPE
#undef NAME
#undef MAP

#endif
#endif
