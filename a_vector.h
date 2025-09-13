/*
 * a_string/a_vector: a scuffed dynamic vector/string implementation.
 *
 * Copyright (c) Eason Qin, 2025.
 *
 * This source code form is licensed under the MIT/Expat license.
 * Visit the OSI website for a digital version.
 */
#ifndef _A_VECTOR_H
#define _A_VECTOR_H

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#define A_VECTOR_DECL(T)                                                       \
    typedef struct {                                                           \
        T* data;                                                               \
        size_t len;                                                            \
        size_t cap;                                                            \
    } av_##T;                                                                  \
    av_##T av_##T##_new(void);                                                 \
    av_##T av_##T##_with_capacity(size_t cap);                                 \
    av_##T av_##T##_from_slice(const T* slice, size_t nitems);                 \
    void av_##T##_free(av_##T* v);                                             \
    bool av_##T##_valid(av_##T* v);                                            \
    void av_##T##_reserve(av_##T* v, size_t cap);                              \
    void av_##T##_append(av_##T* v, T new_elem);                               \
    void av_##T##_append_vector(av_##T* v, const av_##T* other);               \
    void av_##T##_append_slice(av_##T* v, const T* ptr, size_t nitems);        \
    T av_##T##_pop(av_##T* v);                                                 \
    T av_##T##_pop_at(av_##T* v, size_t pos);
#define A_VECTOR_GROWTH_FACTOR 3
#define A_VECTOR_IMPL(T)                                                       \
    av_##T av_##T##_new(void) { return av_##T##_with_capacity(5); }            \
    av_##T av_##T##_with_capacity(size_t cap) {                                \
        av_##T res = {.len = 0, .cap = cap};                                   \
        res.data = calloc(res.cap, sizeof(T));                                 \
        check_alloc(res.data);                                                 \
        return res;                                                            \
    }                                                                          \
    av_##T av_##T##_from_slice(const T* slice, size_t nitems) {                \
        av_##T res = av_##T##_with_capacity(nitems);                           \
        memcpy(res.data, slice, nitems * sizeof(T));                           \
        res.len = nitems;                                                      \
        return res;                                                            \
    }                                                                          \
    void av_##T##_free(av_##T* v) {                                            \
        free(v->data);                                                         \
        v->len = (size_t)-1;                                                   \
        v->cap = (size_t)-1;                                                   \
    }                                                                          \
    bool av_##T##_valid(av_##T* v) {                                           \
        return !(v->len == (size_t)-1 || v->cap == (size_t)-1 ||               \
                 v->data == NULL);                                             \
    }                                                                          \
    void av_##T##_reserve(av_##T* v, size_t cap) {                             \
        if (!av_##T##_valid(v)) {                                              \
            panic("the vector is invalid");                                    \
        }                                                                      \
        v->data = realloc(v->data, sizeof(T) * cap);                           \
        check_alloc(v->data);                                                  \
        v->cap = cap;                                                          \
    }                                                                          \
    void av_##T##_append(av_##T* v, T new_elem) {                              \
        if (!av_##T##_valid(v)) {                                              \
            panic("the vector is invalid");                                    \
        }                                                                      \
        if (v->len + 1 > v->cap) {                                             \
            av_##T##_reserve(v, v->cap * A_VECTOR_GROWTH_FACTOR);              \
        }                                                                      \
        v->data[v->len++] = new_elem;                                          \
    }                                                                          \
    void av_##T##_append_vector(av_##T* v, const av_##T* other) {              \
        if (!av_##T##_valid(v)) {                                              \
            panic("the vector is invalid");                                    \
        }                                                                      \
        size_t len = v->len + other->len;                                      \
        if (len > v->cap) {                                                    \
            size_t sz = v->cap;                                                \
            while (sz < len)                                                   \
                sz *= A_VECTOR_GROWTH_FACTOR;                                  \
            av_##T##_reserve(v, sz);                                           \
        }                                                                      \
        memcpy(&v->data[v->len], other->data, sizeof(T) * other->len);         \
        v->len += other->len;                                                  \
    }                                                                          \
    void av_##T##_append_slice(av_##T* v, const T* data, size_t nitems) {      \
        if (!av_##T##_valid(v)) {                                              \
            panic("the vector is invalid");                                    \
        }                                                                      \
        size_t len = v->len + nitems;                                          \
        if (len > v->cap) {                                                    \
            size_t sz = v->cap;                                                \
            while (sz < len)                                                   \
                sz *= A_VECTOR_GROWTH_FACTOR;                                  \
            av_##T##_reserve(v, sz);                                           \
        }                                                                      \
        memcpy(&v->data[v->len], data, sizeof(T) * nitems);                    \
        v->len += nitems;                                                      \
    }                                                                          \
    T av_##T##_pop(av_##T* v) {                                                \
        if (!av_##T##_valid(v)) {                                              \
            panic("the vector is invalid");                                    \
        }                                                                      \
        T res = v->data[--v->len];                                             \
        if (v->len < (size_t)(v->cap / A_VECTOR_GROWTH_FACTOR)) {              \
            av_##T##_reserve(v, v->cap / A_VECTOR_GROWTH_FACTOR);              \
        }                                                                      \
        return res;                                                            \
    }                                                                          \
    T av_##T##_pop_at(av_##T* v, size_t pos) {                                 \
        if (!av_##T##_valid(v)) {                                              \
            panic("the vector is invalid");                                    \
        }                                                                      \
        if (pos >= v->len) {                                                   \
            panic("array index %zu out of range", pos);                        \
        }                                                                      \
        T res = v->data[pos];                                                  \
        size_t items = (v->len - pos - 1);                                     \
        memmove(&v->data[pos], &v->data[pos + 1], items * sizeof(T));          \
        v->len--;                                                              \
        return res;                                                            \
    }

#endif // _A_VECTOR_H
