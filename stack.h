#ifndef TOOLS_H
#error "please include tools.h before including stack.h"
#else

#ifndef STACK_TYPE
#error "STACK_TYPE must be defined before including stack.h"
#else

#define CAT(a, b) a##_##b
#define CAT2(a, b) CAT(a, b)
#define NAME(a) CAT2(a, STACK_TYPE)

#define STACK NAME(Map)

typedef struct {
    STACK_TYPE* ptr;
    U64 len;
} STACK;

MAP NAME(map_create)(U32 size) {
    Set set = set_create(size);
    return (MAP) {
        .set = set,
        .objects = malloc(((U64)set.mask + 1) * sizeof(HASH_MAP_TYPE))
    };
}

void NAME(map_insert)(MAP* map, HashKey key, HASH_MAP_TYPE val) {
    U32 idx = set_insert(&map->set, key);
    map->objects[idx] = val;
}

// returns NULL if not found
HASH_MAP_TYPE* NAME(map_lookup)(MAP* map, HashKey key) {
    U64 ret = set_lookup(&map->set, key);
    if ((ret & 1) == 0) { return NULL; }

    U64 idx = ret >> 32;
    return &map->objects[idx];
}

// returns NULL if not found
HASH_MAP_TYPE* NAME(map_remove)(MAP* map, HashKey key) {
    U64 ret = set_remove(&map->set, key);
    if ((ret & 1) == 0) { return NULL; }

    U64 idx = ret >> 32;
    return &map->objects[idx];
}

// returns NULL if not found
void NAME(map_dealloc)(MAP* map) {
    set_dealloc(&map->set);
    free(map->objects)
}

#undef HASH_MAP_TYPE
#undef CAT2
#undef CAT
#undef NAME
#undef MAP

#endif
#endif
