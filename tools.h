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
#include <math.h>

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

// MATH -----------------------------------------------------------------------

typedef union {
    struct {
        F32 x, y;
    };
    F32 arr[2];
} Vec_2;

typedef union {
    struct {
        F32 x, y, z;
    };

    struct {
        Vec_2 xy;
        F32 z_xy;
    };

    struct {
        F32 x_yz;
        Vec_2 yz;
    };

    F32 arr[3];
} Vec_3;

typedef union {
    struct {
        F32 x, y, z, w;
    };

    struct {
        F32 r, g, b, a;
    };

    struct {
        Vec_2 xy;
        Vec_2 zw;
    };

    struct {
        F32 x_yz;
        Vec_2 yz;
        F32 w_yz;
    };

    struct {
        Vec_3 xyz;
        F32 z_xyz;
    };

    struct {
        Vec_3 rgb;
        F32 a_rgb;
    };

    struct {
        F32 x_yzw;
        Vec_3 yzw;
    };

    F32 arr[4];
} Vec_4;

Vec_2 vec_add_2(Vec_2 a, Vec_2 b);
Vec_3 vec_add_3(Vec_3 a, Vec_3 b);
Vec_4 vec_add_4(Vec_4 a, Vec_4 b);

Vec_2 vec_mul_2(Vec_2 a, Vec_2 b);
Vec_3 vec_mul_3(Vec_3 a, Vec_3 b);
Vec_4 vec_mul_4(Vec_4 a, Vec_4 b);

Vec_2 vec_div_2(Vec_2 a, Vec_2 b);
Vec_3 vec_div_3(Vec_3 a, Vec_3 b);
Vec_4 vec_div_4(Vec_4 a, Vec_4 b);

Vec_2 vec_sub_2(Vec_2 a, Vec_2 b);
Vec_3 vec_sub_3(Vec_3 a, Vec_3 b);
Vec_4 vec_sub_4(Vec_4 a, Vec_4 b);

Vec_2 vec_add_F32_2(Vec_2 a, F32 b);
Vec_3 vec_add_F32_3(Vec_3 a, F32 b);
Vec_4 vec_add_F32_4(Vec_4 a, F32 b);

Vec_2 vec_mul_F32_2(Vec_2 a, F32 b);
Vec_3 vec_mul_F32_3(Vec_3 a, F32 b);
Vec_4 vec_mul_F32_4(Vec_4 a, F32 b);

Vec_2 vec_div_F32_2(Vec_2 a, F32 b);
Vec_3 vec_div_F32_3(Vec_3 a, F32 b);
Vec_4 vec_div_F32_4(Vec_4 a, F32 b);

Vec_2 vec_sub_F32_2(Vec_2 a, F32 b);
Vec_3 vec_sub_F32_3(Vec_3 a, F32 b);
Vec_4 vec_sub_F32_4(Vec_4 a, F32 b);

F32 vec_dot_2(Vec_2 a, Vec_2 b);
F32 vec_dot_3(Vec_3 a, Vec_3 b);
F32 vec_dot_4(Vec_4 a, Vec_4 b);

Vec_2 vec_neg_2(Vec_2 a);
Vec_3 vec_neg_3(Vec_3 a);
Vec_4 vec_neg_4(Vec_4 a);

F32 vec_len_sq_2(Vec_2 a);
F32 vec_len_sq_3(Vec_3 a);
F32 vec_len_sq_4(Vec_4 a);

F32 vec_len_2(Vec_2 a);
F32 vec_len_3(Vec_3 a);
F32 vec_len_4(Vec_4 a);

Vec_2 vec_recip_2(Vec_2 a);
Vec_3 vec_recip_3(Vec_3 a);
Vec_4 vec_recip_4(Vec_4 a);

Vec_2 vec_bcast_2(F32 a);
Vec_3 vec_bcast_3(F32 a);
Vec_4 vec_bcast_4(F32 a);

Vec_2 vec_norm_2(Vec_2 a);
Vec_3 vec_norm_3(Vec_3 a);
Vec_4 vec_norm_4(Vec_4 a);

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
typedef U16 ArenaGen;

typedef struct {
    ArenaGen gen;
    ArenaIdx idx;
} ArenaKey;

typedef struct {
    U64* free;
    ArenaGen* generations;
    ArenaIdx element_num;
} ArenaTracking;

ArenaTracking arena_tracking_create();
ArenaKey arena_tracking_insert(ArenaTracking* ar);

F32 arena_utilization(ArenaTracking* ar);
bool arena_tracking_key_valid(ArenaTracking* ar, ArenaKey k);
void arena_tracking_remove(ArenaTracking* ar, ArenaKey k);
void arena_tracking_dealloc(ArenaTracking* ar);

typedef struct {
    ArenaTracking* ar;
    ArenaIdx idx;
} ArenaIter;

ArenaIter arena_iter(ArenaTracking* ar);
ArenaKey arena_iter_next(ArenaIter* iter);

#endif
