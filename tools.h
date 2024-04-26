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

#define IF_ERR(expr, message) if (expr); fprintf(stderr, "%s\n", message); return 1; }

// used for generics
#define CAT(a, b) a##_##b
#define CAT2(a, b) CAT(a, b)

// round to next power of 2
U32 round_pow_2(U32 n);

// returns index of lowest set bit
// UB if n is zero
U8 lowest_bit_idx(U64 n);

// STRING ----------------------------------------------------------------------

typedef struct {
    char* ptr;
    U64 len;
} String;

void string_print(String s);
void string_print_n(String s);
bool string_equals(String a, String b);
String string_create(char* cstr);
String string_slice(String s, U64 start, U64 end);
String string_slice_start(String s, U64 start);
int string_copy(String source, String dest);
String string_trim_start(String s);
String string_trim_end(String s);
String string_trim(String s);
int parse_unsigned(U64* out, String s);

typedef struct {
    U8* ptr;
    U64 len;
} Bytes;

Bytes read_file(const char* path);

// PARSING -----------------------------------------------------------------------------

typedef struct {
    String segment;     // includes separator, if it exists
    String token;       // does not include separator
    String original;
} StringSegment;

StringSegment segment_create(String s);
int segment_next(StringSegment* l, char separator);

// HASHING -----------------------------------------------------------

typedef U32 HashKey;

// murmur 3
#define HASH(t) hash_bytes((U8*)&t, sizeof(t))
HashKey murmur_32_scramble(U32 k);
HashKey hash_bytes(const U8* key, U64 len);

// hash set -------------------------------------------------------------------

typedef struct {
    HashKey* keys;
    U32 mask;
} Set;

// allows for at least size elements
Set set_create(U32 size);

// TODO: prevent infinite lookup loops when all elements in keys are nonzero

// high half is key index, or first available index
// low half is 1 if found, 0 if not found
U64 set_lookup(Set* set, HashKey key);

// returns index in backing array
U32 set_insert(Set* set, HashKey key);

// high half is key index, or first available index
// low half is 1 if found, 0 if not found
U64 set_remove(Set* set, HashKey key);
void set_dealloc(Set* set);

// PRNG -----------------------------------------------------------------------

extern U64 PRNG_SEEDS[256];

typedef struct {
    U64 x, w, s;
} Prng;

Prng prng_create(U64 seed);
U32 prng_next(Prng* prng);

// TIMERS --------------------------------------------------------------------

typedef struct timespec TimeSpec;

double time_s(TimeSpec t);
double time_ms(TimeSpec t);
double time_us(TimeSpec t);
double time_ns(TimeSpec t);

typedef struct {
    TimeSpec start;
} Timer;

Timer timer_start(void);
TimeSpec timer_elapsed(Timer* timer);
TimeSpec timer_lap(Timer* timer);
double timer_elapsed_s(Timer* timer);
double timer_elapsed_ms(Timer* timer);
double timer_elapsed_us(Timer* timer);
double timer_elapsed_ns(Timer* timer);
double timer_lap_s(Timer* timer);
double timer_lap_ms(Timer* timer);
double timer_lap_us(Timer* timer);
double timer_lap_ns(Timer* timer);

// ALLOCATORS ----------------------------------------------------------------

Usize page_size(void);
void* vm_alloc(Usize size);
int vm_dealloc(void* ptr, Usize size);

// common arena things -----------------------------------------------------------

typedef struct {
    U16 generation;
    U16 index;
} ArenaKey;

// bump -----------------------------------------------

// Memory not contiguous
// clearing may deallocate
typedef struct {
    U8* alloc_start;
    U8* pos;  
} BumpList;

#define BUMP_PAGE_ALLOC_COUNT 32
#define ALIGN_TO(p, align) (void*)((Usize)(p) & ~((align)-1))
#define BUMP_LIST_ALLOC(bump, type) ((type*) bump_list_alloc(bump, sizeof(type), alignof(type)))
#define BUMP_LIST_ALLOC_ARRAY(bump, type, size) ((type*) bump_list_alloc(bump, sizeof(type)*size, alignof(type)))

BumpList bump_list_create(void);

void bump_list_new_page(BumpList* bump);

// alignment must be a power of 2
void* bump_list_alloc(BumpList* bump, Usize size, Usize align);
void bump_list_clear(BumpList* bump);
void bump_list_dealloc(BumpList* bump);

// arena -----------------------------------------------

#define ARENA_PAGE_ALLOC_COUNT 16
#define ARENA_MAX_ELEMENTS (page_size() * ARENA_PAGE_ALLOC_COUNT)
#define ARENA_INVALID_IDX (ArenaIdx)0xFFFF

typedef U16 ArenaIdx;

typedef struct {
    U64* free;
    ArenaIdx element_num;
} Arena;

Arena arena_create();
ArenaIdx arena_insert(Arena* ar);

void arena_remove(Arena* ar, ArenaIdx idx);
void arena_dealloc(Arena* ar);

typedef struct {
    Arena* ar;
    ArenaIdx idx;
} ArenaIter;

ArenaIter arena_iter(Arena* ar);
ArenaIdx arena_iter_next(ArenaIter* iter);

#endif
