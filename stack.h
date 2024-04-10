#ifndef TOOLS_H
#error "please include tools.h before including stack.h"
#else

#ifndef STACK_TYPE
#error "STACK_TYPE must be defined before including stack.h"
#else

#define NAME(a) CAT2(a, STACK_TYPE)
#define STACK NAME(Stack)

typedef struct {
    STACK_TYPE* objects;
    U32 capacity;
    U32 len;
} STACK;

STACK NAME(stack_create)(U32 capacity) {
    STACK_TYPE* objects = NULL;
    if (capacity != 0) {
        objects = malloc(capacity * sizeof(STACK_TYPE));
    }
    
    return (STACK) {
        .objects = objects,
        .capacity = capacity,
        .len = 0,
    };
}

void NAME(stack_grow)(STACK* stack) {
    U32 capacity = stack->capacity;
    if (capacity < 8) {
        capacity = 16;
    } else {
        capacity *= 2;
    }

    stack->objects = realloc(stack->objects, capacity * sizeof(STACK_TYPE));
    stack->capacity = capacity;
}

void NAME(stack_push)(STACK* stack, STACK_TYPE item) {
    U32 len = stack->len;
    if (len == stack->capacity) {
        NAME(stack_grow)(stack);
    }

    stack->objects[len] = item;
    stack->len = len+1;
}

// returns NULL if not found
STACK_TYPE* NAME(stack_pop)(STACK* stack) {
    U32 len = stack->len;
    if (len == 0) { return NULL; }

    STACK_TYPE* ret = &stack->objects[len];
    stack->len = len-1;
    return ret;
}

// returns NULL if not found
void NAME(stack_dealloc)(STACK* stack) {
    free(stack->objects);
}

#undef STACK_TYPE
#undef NAME
#undef STACK

#endif
#endif
