/*
 * cimi: a scuffed scripting language
 *
 * Copyright (c) Eason Qin <eason@ezntek.com>, 2025.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "expr.h"
#include <stddef.h>

C_Identifier C_Identifier_new(u32 pos, a_string ident) {
    return (C_Identifier){.pos = pos, .ident = ident};
}

void C_Identifier_free(C_Identifier* id) {
    as_free(&id->ident);
}

C_ArrayType C_ArrayType_new(u32 pos, C_Expr index, C_Type inner) {
    C_ArrayType res = {.pos = pos};

    res.index = malloc(sizeof(C_Expr));
    check_alloc(res.index);
    *res.index = index;

    res.index = malloc(sizeof(C_Type));
    check_alloc(res.inner);
    *res.inner = inner;

    return res;
}

void C_ArrayType_free(C_ArrayType* s) {
    C_Expr_free(s->index);
    C_Type_free(s->inner);

    if (s->index)
        free(s->index);

    if (s->inner)
        free(s->index);
}

C_Type C_Type_new_primitive(u32 pos, C_PrimitiveType t) {
    return (C_Type){.pos = pos, .kind = C_TYPE_PRIMITIVE, .data = t};
}

C_Type C_Type_new_array(u32 pos, C_ArrayType t) {
    C_Type res = {.pos = pos};

    res.data.array = malloc(sizeof(C_ArrayType));
    check_alloc(res.data.array);
    *res.data.array = t;

    return res;
}

void C_Type_free(C_Type* t) {
    if (t->kind == C_TYPE_ARRAY) {
        C_ArrayType_free(t->data.array);
        free(t->data.array);
    }
}

C_FunctionArgument C_FunctionArgument_new(u32 pos, C_Identifier ident,
                                          C_Type type) {
    C_FunctionArgument res = {.pos = pos};

    res.ident = malloc(sizeof(C_Identifier));
    check_alloc(res.ident);
    *res.ident = ident;

    res.type = malloc(sizeof(C_Type));
    check_alloc(res.type);
    *res.type = type;

    return res;
}

void C_FunctionArgument_free(C_FunctionArgument* a) {
    C_Identifier_free(a->ident);
    C_Type_free(a->type);

    free(a->ident);
    free(a->type);
}

C_ArgumentList C_ArgumentList_new(u32 pos, C_FunctionArgument** args,
                                  u32 nargs) {
    return (C_ArgumentList){.pos = pos, .args = args, .nargs = nargs};
}

void C_ArgumentList_free(C_ArgumentList* args) {
    for (u32 i = 0; i < args->nargs; ++i) {
        C_FunctionArgument_free(args->args[i]);
    }
}

C_Expr_Unary C_Expr_Unary_new(u32 pos, C_UnaryOp op, struct C_Expr inner) {
    C_Expr_Unary res = {.pos = pos, .op = op};
    res.inner = malloc(sizeof(C_Expr));
    check_alloc(res.inner);
    *res.inner = inner;
    return res;
}

void C_Expr_Unary_free(C_Expr_Unary* e) {
    C_Expr_free(e->inner);
    free(e->inner);
}

C_Expr_Binary C_Expr_Binary_new(u32 pos, C_BinaryOp op, struct C_Expr lhs,
                                struct C_Expr rhs) {
    C_Expr_Binary res = {.pos = pos, .op = op};

    res.lhs = malloc(sizeof(C_Expr));
    check_alloc(res.lhs);
    *res.lhs = lhs;

    res.rhs = malloc(sizeof(C_Expr));
    check_alloc(res.rhs);
    *res.rhs = rhs;

    return res;
}

void C_Expr_Binary_free(C_Expr_Binary* e) {
    C_Expr_free(e->lhs);
    C_Expr_free(e->rhs);
    free(e->lhs);
    free(e->rhs);
}

C_ArrayIndex C_ArrayIndex_new(u32 pos, struct C_Identifier ident,
                              struct C_Expr index) {
    C_ArrayIndex res = {.pos = pos};

    res.ident = malloc(sizeof(C_Identifier));
    check_alloc(res.ident);
    *res.ident = ident;

    res.index = malloc(sizeof(C_Expr));
    check_alloc(res.index);
    *res.index = index;

    return res;
}

void C_ArrayIndex_free(C_ArrayIndex* e) {
    C_Identifier_free(e->ident);
    C_Expr_free(e->index);

    free(e->ident);
    free(e->index);
}

C_Expr_FnCall C_Expr_FnCall_new(u32 pos, C_Identifier ident,
                                C_ArgumentList args) {
    C_Expr_FnCall res = {.pos = pos};

    res.ident = malloc(sizeof(C_Identifier));
    check_alloc(res.ident);
    *res.ident = ident;

    res.args = malloc(sizeof(C_ArgumentList));
    check_alloc(res.args);
    *res.args = args;

    return res;
}

void C_Expr_FnCall_free(C_Expr_FnCall* c) {
    C_Identifier_free(c->ident);
    C_ArgumentList_free(c->args);

    free(c->ident);
    free(c->args);
}

C_Expr C_Expr_new_unary(C_Expr_Unary e) {
    return (C_Expr){.kind = C_EXPR_UNARYOP, .data.unary = e};
}
C_Expr C_Expr_new_binary(C_Expr_Binary e) {
    return (C_Expr){.kind = C_EXPR_BINOP, .data.binary = e};
}

C_Expr C_Expr_new_array_index(C_ArrayIndex e) {
    return (C_Expr){.kind = C_EXPR_ARRAY_INDEX, .data.array_index = e};
}

C_Expr C_Expr_new_fncall(C_Expr_FnCall e) {
    return (C_Expr){.kind = C_EXPR_FNCALL, .data.fncall = e};
}

void C_Expr_free(C_Expr* e) {
    switch (e->kind) {
        case C_EXPR_UNARYOP: {
            C_Expr_Unary_free(&e->data.unary);
        } break;
        case C_EXPR_BINOP: {
            C_Expr_Binary_free(&e->data.binary);
        } break;
        case C_EXPR_ARRAY_INDEX: {
            C_ArrayIndex_free(&e->data.array_index);
        } break;
        case C_EXPR_FNCALL: {
            C_Expr_FnCall_free(&e->data.fncall);
        } break;
    }
}
