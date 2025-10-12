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

#define A_VECTOR_DECL(T, name)                                                 \
    typedef struct {                                                           \
        T* data;                                                               \
        size_t len;                                                            \
        size_t cap;                                                            \
    } name;                                                                    \
    name name##_new(void);                                                     \
    name name##_with_capacity(size_t cap);                                     \
    name name##_from_slice(const T* slice, size_t nitems);                     \
    void name##_free(name* v);                                                 \
    bool name##_valid(name* v);                                                \
    void name##_reserve(name* v, size_t cap);                                  \
    void name##_append(name* v, T new_elem);                                   \
    void name##_append_vector(name* v, const name* other);                     \
    void name##_append_slice(name* v, const T* ptr, size_t nitems);            \
    T name##_pop(name* v);                                                     \
    T name##_pop_at(name* v, size_t pos);
#define A_VECTOR_GROWTH_FACTOR 3
#define A_VECTOR_IMPL(T, name)                                                 \
    name name##_new(void) {                                                    \
        return name##_with_capacity(5);                                        \
    }                                                                          \
    name name##_with_capacity(size_t cap) {                                    \
        name res = {.len = 0, .cap = cap};                                     \
        res.data = calloc(res.cap, sizeof(T));                                 \
        check_alloc(res.data);                                                 \
        return res;                                                            \
    }                                                                          \
    name name##_from_slice(const T* slice, size_t nitems) {                    \
        name res = name##_with_capacity(nitems);                               \
        memcpy(res.data, slice, nitems * sizeof(T));                           \
        res.len = nitems;                                                      \
        return res;                                                            \
    }                                                                          \
    void name##_free(name* v) {                                                \
        free(v->data);                                                         \
        v->len = (size_t)-1;                                                   \
        v->cap = (size_t)-1;                                                   \
    }                                                                          \
    bool name##_valid(name* v) {                                               \
        return !(v->len == (size_t)-1 || v->cap == (size_t)-1 ||               \
                 v->data == NULL);                                             \
    }                                                                          \
    void name##_reserve(name* v, size_t cap) {                                 \
        if (!name##_valid(v)) {                                                \
            panic("the vector is invalid");                                    \
        }                                                                      \
        v->data = realloc(v->data, sizeof(T) * cap);                           \
        check_alloc(v->data);                                                  \
        v->cap = cap;                                                          \
    }                                                                          \
    void name##_append(name* v, T new_elem) {                                  \
        if (!name##_valid(v)) {                                                \
            panic("the vector is invalid");                                    \
        }                                                                      \
        if (v->len + 1 > v->cap) {                                             \
            name##_reserve(v, v->cap* A_VECTOR_GROWTH_FACTOR);                 \
        }                                                                      \
        v->data[v->len++] = new_elem;                                          \
    }                                                                          \
    void name##_append_vector(name* v, const name* other) {                    \
        if (!name##_valid(v)) {                                                \
            panic("the vector is invalid");                                    \
        }                                                                      \
        size_t len = v->len + other->len;                                      \
        if (len > v->cap) {                                                    \
            size_t sz = v->cap;                                                \
            while (sz < len)                                                   \
                sz *= A_VECTOR_GROWTH_FACTOR;                                  \
            name##_reserve(v, sz);                                             \
        }                                                                      \
        memcpy(&v->data[v->len], other->data, sizeof(T) * other->len);         \
        v->len += other->len;                                                  \
    }                                                                          \
    void name##_append_slice(name* v, const T* data, size_t nitems) {          \
        if (!name##_valid(v)) {                                                \
            panic("the vector is invalid");                                    \
        }                                                                      \
        size_t len = v->len + nitems;                                          \
        if (len > v->cap) {                                                    \
            size_t sz = v->cap;                                                \
            while (sz < len)                                                   \
                sz *= A_VECTOR_GROWTH_FACTOR;                                  \
            name##_reserve(v, sz);                                             \
        }                                                                      \
        memcpy(&v->data[v->len], data, sizeof(T) * nitems);                    \
        v->len += nitems;                                                      \
    }                                                                          \
    T name##_pop(name* v) {                                                    \
        if (!name##_valid(v)) {                                                \
            panic("the vector is invalid");                                    \
        }                                                                      \
        T res = v->data[--v->len];                                             \
        if (v->len < (size_t)(v->cap / A_VECTOR_GROWTH_FACTOR)) {              \
            name##_reserve(v, v->cap / A_VECTOR_GROWTH_FACTOR);                \
        }                                                                      \
        return res;                                                            \
    }                                                                          \
    T name##_pop_at(name* v, size_t pos) {                                     \
        if (!name##_valid(v)) {                                                \
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
