#ifndef TOOLS_H
#define TOOLS_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <unistd.h>
#include <sys/mman.h>
#include <time.h>

typedef uint64_t U64;
typedef uint32_t U32;
typedef uint16_t U16;
typedef uint8_t  U8;
typedef int64_t I64;
typedef int32_t I32;
typedef int16_t I16;
typedef int8_t  I8;
typedef size_t Usize;

typedef double F64;
typedef float F32;

#define IF_ERR(expr, message) if (expr) { fprintf(stderr, "%s\n", message); return 1; }

// STRING ----------------------------------------------------------------------

typedef struct {
    char* ptr;
    U64 len;
} String;

void string_print(String s) {
    fwrite(s.ptr, 1, s.len, stdout);
}

void string_print_n(String s) {
    string_print(s);
    printf("\n");
}

bool string_equals(String a, String b) {
    if (a.len != b.len) { return false; }

    for (U64 i = 0; i < a.len; ++i) {
        if (a.ptr[i] != b.ptr[i]) { return false; }
    }

    return true;
}

String string_create(char* cstr) {
    U64 i = 0;
    while (1) {
        if (cstr[i] == '\0') { break; }
        i += 1;
    }

    String s = { .ptr = cstr, .len = i };
    return s;
}

String string_slice(String s, U64 start, U64 end) {
    String ret = {
        .ptr = s.ptr + start,
        .len = end - start,
    };
    return ret;
}

String string_slice_start(String s, U64 start) {
    String ret = {
        .ptr = s.ptr + start,
        .len = s.len - start,
    };
    return ret;
}

int string_copy(String source, String dest) {
    if (source.len > dest.len) { return 1; }
    for (U64 i = 0; i < source.len; ++i) {
        dest.ptr[i] = source.ptr[i];
    }

    return 0;
}

String string_trim_start(String s) {
    while (s.len > 0 && s.ptr[0] == ' ') {
        s.len -= 1;
        s.ptr += 1;
    }
    return s;
}

String string_trim_end(String s) {
    while (s.len > 0 && s.ptr[s.len-1] == ' ') {
        s.len -= 1;
    }
    return s;
}

String string_trim(String s) {
    return string_trim_end(string_trim_start(s));
}

int parse_unsigned(U64* out, String s) {
    *out = 0;

    U64 mul = 1;
    U64 i = s.len-1;

    while (1) {
        char c = s.ptr[i];
        if ('9' < c || c < '0') { return 1; }
        *out += (U64)(c - '0') * mul;
        if (i == 0) { break; }
        mul *= 10;
        i -= 1;
    }

    return 0;
}

String read_input(void) {
    String result = {NULL, 0};

    FILE* file = fopen("input", "rb");
    if (file != NULL) {
        fseek(file, 0, SEEK_END);
        I64 ret = ftell(file);
        assert(ret >= 0);
        U64 file_size = (U64)ret;
        fseek(file, 0, SEEK_SET);

        result.ptr = (char*)malloc(file_size + 1); // +1 for null terminator
        if (result.ptr != NULL) {
            fread(result.ptr, 1, file_size, file);
            result.ptr[file_size] = '\0'; // Null-terminate the string
            result.len = file_size;
        }

        fclose(file);
    }

    return result;
}

// PARSING -----------------------------------------------------------------------------

typedef struct {
    String segment;     // includes separator, if it exists
    String token;       // does not include separator
    String original;
} StringSegment;

StringSegment segment_create(String s) {
    StringSegment segments = {.segment = {s.ptr, 0}, .token = {s.ptr, 0}, .original = s};
    return segments;
}

int segment_next(StringSegment* l, char separator) {
    U64 orig_len = l->original.len;
    char* seg_start = l->segment.ptr + l->segment.len;
    U64 dist_from_start = (Usize)seg_start - (Usize)l->original.ptr;
    if (dist_from_start >= orig_len) {
        return 0;
    }

    U64 segment_len = 1;
    U64 token_len = 1;
    while (dist_from_start + segment_len < orig_len) {
        char c = seg_start[segment_len];
        if (c == separator) { 
            ++segment_len; 
            break; 
        } else {
            ++segment_len;
            ++token_len;
        }
    }

    String new_seg = { .ptr = seg_start, .len = segment_len };
    l->segment = new_seg;
    String new_tok = { .ptr = seg_start, .len = token_len };
    l->token = new_tok;

    return 1;
}

// GRID --------------------------------------------------------------------------------

typedef struct {
    char* grid;
    I64 width;
    I64 height;
} Grid;

int grid_parse(Grid* out, String input) {
    StringSegment lines = segment_create(input);
    segment_next(&lines, '\n');
    U64 line_len = lines.token.len;
    U64 line_count = 1;

    while (segment_next(&lines, '\n')) {
        line_count += 1;
    }

    char* data = malloc(line_len * line_count);

    lines = segment_create(input);
    U64 l = 0;
    while (segment_next(&lines, '\n')) {
        if (lines.token.len != line_len) { return 1; }

        String s = { .ptr = &data[line_len * l], .len = line_len };
        if (string_copy(lines.token, s)) { return 2; }
        l += 1;
    }

    Grid g = { .grid = data, .width = (I64)line_len, .height = (I64)line_count };
    *out = g;
    return 0;
}

void grid_dealloc(Grid g) {
    free(g.grid);
}

void grid_print(Grid g) {
    for (I64 y = 0; y < g.height; ++y) {
        fwrite(&g.grid[y * g.width], 1, (Usize)g.width, stdout);
        printf("\n");
    }
}

bool grid_valid_idx(Grid g, I64 x, I64 y) {
    return 0 <= x && x < g.width && 0 <= y && y < g.height;
}

// returns null byte on error
char grid_index(Grid g, I64 x, I64 y) {
    if (!grid_valid_idx(g, x, y)) { return 0; }
    return g.grid[y * g.width + x];
}

// exclusive ranges
int grid_sub(Grid* child, Grid parent, I64 x1, I64 x2, I64 y1, I64 y2) {
    if (!grid_valid_idx(parent, x1, y1)) { return 1; }
    if (!grid_valid_idx(parent, x2, y2)) { return 2; }

    if (x2 < x1) { return 3; }
    if (y2 < y1) { return 4; }

    I64 out_width = x2 - x1;
    I64 out_height = y2 - y1;
    child->width = out_width;
    child->height = out_height;
    child->grid = malloc((Usize)(out_width * out_height) * sizeof(char));

    for (I64 y = 0; y < out_height; ++y) {
        for (I64 x = 0; x < out_width; ++x) {
            I64 in_idx = (y1+y)*parent.width + x1+x;
            I64 out_idx = y*out_width+x;
            child->grid[out_idx] = parent.grid[in_idx];
        }
    }

    return 0;
}

// HASHING -----------------------------------------------------------

typedef U32 HashKey;

HashKey murmur_32_scramble(U32 k) {
    k *= 0xcc9e2d51;
    k = (k << 15) | (k >> 17);
    k *= 0x1b873593;
    return k;
}

#define HASH(t) hash_bytes((U8*)&t, sizeof(t))

// murmur 3
HashKey hash_bytes(const U8* key, U64 len) {
    U32 h = 0x1b873593;
    U32 k;
    for (U64 i = len >> 2; i; i--) {
        memcpy(&k, key, sizeof(U32));
        key += sizeof(U32);
        h ^= murmur_32_scramble(k);
        h = (h << 13) | (h >> 19);
        h = h * 5 + 0xe6546b64;
    }
    k = 0;
    for (U64 i = len & 3; i; i--) {
        k <<= 8;
        k |= key[i - 1];
    }
    h ^= murmur_32_scramble(k);
    h ^= (U32)len;
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;
    return h;
}

// hash set -------------------------------------------------------------------

typedef struct {
    HashKey* keys;
    U32 mask;
} Set;

// allows for at least size elements
Set set_create(U32 size) {
    // round up to pow2
    U32 mask = size-1;
    mask |= mask >> 1;
    mask |= mask >> 2;
    mask |= mask >> 4;
    mask |= mask >> 8;
    mask |= mask >> 16;

    return (Set) {
        .keys = calloc(mask+1, sizeof(HashKey)),
        .mask = mask,
    };
}

// TODO: prevent infinite lookup loops when all elements in keys are nonzero

// high half is key index, or first available index
// low half is 1 if found, 0 if not found
U64 set_lookup(Set* set, HashKey key) {
    U32 mask = set->mask;
    HashKey* keys = set->keys;

    U32 idx = key & mask;
    HashKey ele = keys[idx];

    // don't stop on removed elements (ele == 1)
    while (ele != 0 && ele != key) {
        idx += 1;

        // wrap on overflow size
        idx &= mask;
        ele = keys[idx];
    }

    return ((U64)idx << 32) ^ (U64)(ele == key);
}

// returns index in backing array
U32 set_insert(Set* set, HashKey key) {
    assert(key > 1);

    U32 mask = set->mask;
    HashKey* keys = set->keys;

    U32 idx = key & mask;
    HashKey ele = keys[idx];
    while (ele > 1 && ele != key) {
        idx += 1;

        // wrap on overflow size
        idx &= mask;
        ele = keys[idx];
    }

    keys[idx] = key;
    return idx;
}

// high half is key index, or first available index
// low half is 1 if found, 0 if not found
U64 set_remove(Set* set, HashKey key) {
    U64 ret = set_lookup(set, key);
    if ((ret & 1) == 0) { return ret; }
    
    // Replace with 1. This prevents not finding inserted keys in some cases.
    // Replacing with 0 causes premature stops
    set->keys[ret >> 32] = 1;
    return ret;
}

void set_dealloc(Set* set) {
    free(set->keys);
}

// PRNG -----------------------------------------------------------------------

U64 PRNG_SEEDS[256] = {
#include "prng_seeds.h"
};

typedef struct {
    U64 x, w, s;
} Prng;

Prng prng_create(U64 seed) {
    U64 s = PRNG_SEEDS[seed & (255)];
    return (Prng) {
        .x = 0,
        .w = 0,
        .s = s,
    };
}

U32 prng_next(Prng* prng) {
    U64 x = prng->x;
    U64 w = prng->w;

    w += prng->s;
    x = x*x + w;
    x = (x >> 32) ^ (x << 32);

    prng->x = x;
    prng->w = w;

    return (U32) x;
}

// TIMERS --------------------------------------------------------------------

typedef struct timespec TimeSpec;

double time_s(TimeSpec t) {
    return (double)t.tv_sec + (double)t.tv_nsec / 1'000'000'000.0;
}

double time_ms(TimeSpec t) {
    return (double)t.tv_sec * 1'000.0 + (double)t.tv_nsec / 1'000'000.0;
}

double time_us(TimeSpec t) {
    return (double)t.tv_sec * 1'000'000.0 + (double)t.tv_nsec / 1'000.0;
}

double time_ns(TimeSpec t) {
    return (double)t.tv_sec * 1'000'000'000.0 + (double)t.tv_nsec;
}


typedef struct {
    TimeSpec start;
} Timer;

Timer timer_start(void) {
    Timer t;
    clock_gettime(CLOCK_MONOTONIC, &t.start);
    return t;
}

TimeSpec timer_elapsed(Timer* timer) {
    TimeSpec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    t.tv_sec -= timer->start.tv_sec;
    t.tv_nsec -= timer->start.tv_nsec;
    return t;
}

TimeSpec timer_lap(Timer* timer) {
    TimeSpec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    TimeSpec diff = now;
    diff.tv_sec -= timer->start.tv_sec;
    diff.tv_nsec -= timer->start.tv_nsec;
    timer->start = now;
    return diff;
}

double timer_elapsed_s(Timer* timer) { return time_s(timer_elapsed(timer)); }
double timer_elapsed_ms(Timer* timer) { return time_ms(timer_elapsed(timer)); }
double timer_elapsed_us(Timer* timer) { return time_us(timer_elapsed(timer)); }
double timer_elapsed_ns(Timer* timer) { return time_ns(timer_elapsed(timer)); }
double timer_lap_s(Timer* timer) { return time_s(timer_lap(timer)); }
double timer_lap_ms(Timer* timer) { return time_ms(timer_lap(timer)); }
double timer_lap_us(Timer* timer) { return time_us(timer_lap(timer)); }
double timer_lap_ns(Timer* timer) { return time_ns(timer_lap(timer)); }

// ALLOCATORS ----------------------------------------------------------------

Usize page_size(void) {
    return (Usize)sysconf(_SC_PAGESIZE);
}

void* vm_alloc(Usize size) {
    return mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}

int vm_dealloc(void* ptr, Usize size) {
    return munmap(ptr, size);
}

// common arena things -----------------------------------------------------------

typedef struct {
    U16 generation;
    U16 index;
} ArenaKey;

// bump -----------------------------------------------

// Memory not contiguous
// resetting may deallocate
typedef struct {
    U8* alloc_start;
    U8* pos;  
} BumpList;

BumpList bump_list_create(void) {
    return (BumpList) {
        .alloc_start = (U8*)~((Usize) 0),
        .pos         = (U8*)~((Usize) 0),
    };
}

#define BUMP_PAGE_ALLOC_COUNT 32

void bump_list_new_page(BumpList* bump) {
    Usize alloc_size = page_size() * BUMP_PAGE_ALLOC_COUNT;
    U8* new_alloc_start = vm_alloc(alloc_size);
    //printf("allocating %lu\n", new_alloc_start);

    // linked list of ptrs
    U8** prev_page_pos = (U8**)((Usize)new_alloc_start + alloc_size - sizeof(U8*));
    *((U8**) prev_page_pos) = bump->alloc_start;

    *bump = (BumpList) {
        .alloc_start = new_alloc_start,
        .pos = (U8*)prev_page_pos,
    };
}

#define ALIGN_TO(p, align) (void*)((Usize)(p) & ~((align)-1))

#define BUMP_LIST_ALLOC(bump, type) ((type*) bump_list_alloc(bump, sizeof(type), alignof(type)))
#define BUMP_LIST_ALLOC_ARRAY(bump, type, size) ((type*) bump_list_alloc(bump, sizeof(type)*size, alignof(type)))

// alignment must be a power of 2
void* bump_list_alloc(BumpList* bump, Usize size, Usize align) {
    if (size == 0) { return NULL; }

    U8* aligned = ALIGN_TO(bump->pos - size, align);

    if (aligned < bump->alloc_start) {
        bump_list_new_page(bump);

        aligned = ALIGN_TO(bump->pos - size, align);
        assert(aligned >= bump->alloc_start);
    }

    bump->pos = aligned;
    return (void*) aligned;
}

void bump_list_dealloc(BumpList* bump) {
    Usize alloc_size = page_size() * BUMP_PAGE_ALLOC_COUNT;
    U8* page = bump->alloc_start;
    
    while (~((Usize)page) != 0) {
        U8* next_page = *((U8**)((Usize)page + alloc_size - sizeof(U8*)));
        vm_dealloc(page, alloc_size);
        //printf("freeing %lu\n", page);
        page = next_page;
    }
}

#endif
