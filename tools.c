#ifndef TOOLS_C
#define TOOLS_C

#include "tools.h"

// round to next power of 2
U32 round_pow_2(U32 n) {
    U32 m = n-1;
    m |= m >> 1;
    m |= m >> 2;
    m |= m >> 4;
    m |= m >> 8;
    m |= m >> 16;
    return m+1;
}

U8 lowest_bit_idx(U64 n) {
    U8 idx = 0;

    n = n & (~n + 1);
    if ((n & 0x00000000FFFFFFFF) == 0) { idx += 32; }
    if ((n & 0x0000FFFF0000FFFF) == 0) { idx += 16; }
    if ((n & 0x00FF00FF00FF00FF) == 0) { idx += 8; }
    if ((n & 0x0F0F0F0F0F0F0F0F) == 0) { idx += 4; }
    if ((n & 0x3333333333333333) == 0) { idx += 2; }
    if ((n & 0x5555555555555555) == 0) { idx += 1; }

    return idx;
}

// STRING ----------------------------------------------------------------------

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
    while (cstr[i] != '\0') {
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


Bytes read_file(const char* path) {
    FILE *f = fopen(path, "rb");
    fseek(f, 0, SEEK_END);
    I64 ret = ftell(f);
    fseek(f, 0, SEEK_SET);
    U64 fsize = (U64)ret;

    U8* c = malloc(fsize + 1);
    fread(c, fsize, 1, f);
    fclose(f);

    return (Bytes) {
        .ptr = c,
        .len = fsize,
    };
}

// PARSING -----------------------------------------------------------------------------

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

// HASHING -----------------------------------------------------------

HashKey murmur_32_scramble(U32 k) {
    k *= 0xcc9e2d51;
    k = (k << 15) | (k >> 17);
    k *= 0x1b873593;
    return k;
}

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

// allows for at least size elements
Set set_create(U32 size) {
    U32 mask = round_pow_2(size)-1;

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

static _Thread_local Usize page_size_global = 0;
Usize page_size(void) {
    if (page_size_global == 0) {
        page_size_global = (Usize)sysconf(_SC_PAGESIZE);
    }
    return page_size_global;
}

// always zero initialized
void* vm_alloc(Usize size) {
    return mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}

int vm_dealloc(void* ptr, Usize size) {
    return munmap(ptr, size);
}

// bump -----------------------------------------------

BumpList bump_list_create(void) {
    return (BumpList) {
        .alloc_start = (U8*)~((Usize) 0),
        .pos         = (U8*)~((Usize) 0),
    };
}

void bump_list_new_page(BumpList* bump) {
    Usize alloc_size = page_size() * BUMP_PAGE_ALLOC_COUNT;
    U8* new_alloc_start = vm_alloc(alloc_size);

    // linked list of ptrs
    U8** prev_page_pos = (U8**)(new_alloc_start + alloc_size - sizeof(U8*));
    *((U8**) prev_page_pos) = bump->alloc_start;

    *bump = (BumpList) {
        .alloc_start = new_alloc_start,
        .pos = (U8*)prev_page_pos,
    };
}

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

void bump_list_clear(BumpList* bump) {
    Usize alloc_size = page_size() * BUMP_PAGE_ALLOC_COUNT;
    U8* page = bump->alloc_start;

    // nothing allocated
    if (~((Usize)page) == 0) { return; }

    while (true) {
        U8* next_page = *((U8**)(page + alloc_size - sizeof(U8*)));
        if (~((Usize)next_page) == 0) { break; }
        vm_dealloc(page, alloc_size);
        page = next_page;
    }

    bump->alloc_start = page;
    bump->pos = (U8*)(page + alloc_size - sizeof(U8*));
}

void bump_list_dealloc(BumpList* bump) {
    Usize alloc_size = page_size() * BUMP_PAGE_ALLOC_COUNT;
    U8* page = bump->alloc_start;
    
    while (~((Usize)page) != 0) {
        U8* next_page = *((U8**)(page + alloc_size - sizeof(U8*)));
        vm_dealloc(page, alloc_size);
        page = next_page;
    }
}

// arena --------------------------------------------------------

Arena arena_create(void) {
    return (Arena) {
        .free = vm_alloc(ARENA_MAX_ELEMENTS / 8),
        .element_num = 0,
    };
}

// returns ARENA_INVALID_IDX on fail
static ArenaIdx find_next_unused(U64* free, U64* end) {
    ArenaIdx idx = 0;
    while (true) {
        U64 mask = *free;
        if (mask != 0) {
            idx += lowest_bit_idx(mask);
            return idx;
        }

        if (free == end) { return ARENA_INVALID_IDX; }
        free += 1;
        idx += 64;
    }
}

ArenaIdx arena_insert(Arena* ar) {
    ArenaIdx idx = find_next_unused(ar->free, &ar->free[ar->element_num / 64]);
    if (idx == ARENA_INVALID_IDX) {
        idx = ar->element_num;
        assert(idx != ARENA_INVALID_IDX);
        ar->element_num += 1;
    } else {
        ar->free[idx / 64] &= ~(1ul << (idx % 64));
    }

    return idx;
}

void arena_remove(Arena* ar, ArenaIdx idx) {
    ar->free[idx / 64] |= (1ul << (idx % 64));
}

void arena_dealloc(Arena* ar) {
    vm_dealloc(ar->free, ARENA_MAX_ELEMENTS / 8);
}

ArenaIter arena_iter(Arena* ar) {
    return (ArenaIter) {
        .ar = ar,
        .idx = 0,
    };
}

ArenaIdx arena_iter_next(ArenaIter* iter) {
    ArenaIdx idx = iter->idx;
    Arena* ar = iter->ar;
    ArenaIdx element_num = ar->element_num;
    U64* free = ar->free;
    while (true) {
        if (idx == element_num) { return ARENA_INVALID_IDX; }

        U64 mask = free[idx / 64];
        if ((mask & (1ul << (idx % 64))) == 0) {
            iter->idx = idx+1;
            return idx;
        }

        idx += 1;
    }
}

#endif
