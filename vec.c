#ifndef VEC_DIM
#error "VEC_DIM must be defined before including vec.h"
#else

#define NAME(a) CAT2(a, VEC_DIM)
#define VEC NAME(Vec)

VEC NAME(vec_add)(VEC a, VEC b) {
    VEC out;
    for (U64 i = 0; i < VEC_DIM; ++i) {
        out.arr[i] = a.arr[i] + b.arr[i];
    }
    return out;
}

VEC NAME(vec_mul)(VEC a, VEC b) {
    VEC out;
    for (U64 i = 0; i < VEC_DIM; ++i) {
        out.arr[i] = a.arr[i] * b.arr[i];
    }
    return out;
}

VEC NAME(vec_div)(VEC a, VEC b) {
    VEC out;
    for (U64 i = 0; i < VEC_DIM; ++i) {
        out.arr[i] = a.arr[i] / b.arr[i];
    }
    return out;
}

VEC NAME(vec_sub)(VEC a, VEC b) {
    VEC out;
    for (U64 i = 0; i < VEC_DIM; ++i) {
        out.arr[i] = a.arr[i] - b.arr[i];
    }
    return out;
}

VEC NAME(vec_add_F32)(VEC a, F32 b) {
    VEC out;
    for (U64 i = 0; i < VEC_DIM; ++i) {
        out.arr[i] = a.arr[i] + b;
    }
    return out;
}

VEC NAME(vec_mul_F32)(VEC a, F32 b) {
    VEC out;
    for (U64 i = 0; i < VEC_DIM; ++i) {
        out.arr[i] = a.arr[i] * b;
    }
    return out;
}

VEC NAME(vec_div_F32)(VEC a, F32 b) {
    VEC out;
    for (U64 i = 0; i < VEC_DIM; ++i) {
        out.arr[i] = a.arr[i] / b;
    }
    return out;
}

VEC NAME(vec_sub_F32)(VEC a, F32 b) {
    VEC out;
    for (U64 i = 0; i < VEC_DIM; ++i) {
        out.arr[i] = a.arr[i] - b;
    }
    return out;
}

F32 NAME(vec_dot)(VEC a, VEC b) {
    F32 out = 0.0f;
    for (U64 i = 0; i < VEC_DIM; ++i) {
        out += a.arr[i] * b.arr[i];
    }
    return out;
}

VEC NAME(vec_neg)(VEC a) {
    VEC out;
    for (U64 i = 0; i < VEC_DIM; ++i) {
        out.arr[i] = -a.arr[i];
    }
    return out;
}

F32 NAME(vec_len_sq)(VEC a) {
    F32 out = 0.0f;
    for (U64 i = 0; i < VEC_DIM; ++i) {
        out += a.arr[i]*a.arr[i];
    }
    return out;
}

F32 NAME(vec_len)(VEC a) {
    return sqrtf(NAME(vec_len_sq)(a));
}

VEC NAME(vec_recip)(VEC a) {
    VEC out;
    for (U64 i = 0; i < VEC_DIM; ++i) {
        out.arr[i] = 1.0f / a.arr[i];
    }
    return out;
}

VEC NAME(vec_bcast)(F32 a) {
    VEC out;
    for (U64 i = 0; i < VEC_DIM; ++i) {
        out.arr[i] = a;
    }
    return out;
}

VEC NAME(vec_norm)(VEC a) {
    VEC out;
    F32 len = NAME(vec_len)(a);
    for (U64 i = 0; i < VEC_DIM; ++i) {
        out.arr[i] = a.arr[i] / len;
    }
    return out;
}

#undef VEC_DIM
#undef NAME
#undef VEC

#endif
