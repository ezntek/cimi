/*
 * cimi: a scuffed scripting language
 *
 * Copyright (c) Eason Qin <eason@ezntek.com>, 2025.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#define _POSIX_C_SOURCE 200809L

#include <stddef.h>

#include "astcommon.h"
#include "common.h"
#include "expr.h"
#include "stmt.h"

C_Identifier C_Identifier_new(u32 pos, a_string ident) {
    return (C_Identifier){.pos = pos, .ident = ident};
}

void C_Identifier_free(C_Identifier* id) {
    as_free(&id->ident);
}

AST_IMPL_FREE(C_Lvalue, lv) {
    if (lv->kind == C_LV_IDENT) {
        C_Identifier_free(lv->data.ident);
        free(lv->data.ident);
    } else if (lv->kind == C_LV_ARRAY_INDEX) {
        C_ArrayIndex_free(lv->data.array_index);
        free(lv->data.array_index);
    }
}

C_Lvalue C_Lvalue_new_ident(u32 pos, struct C_Identifier ident) {
    AST_INIT_NODE(C_Lvalue);

    make(C_Identifier, res.data.ident, ident);

    return res;
}

C_Lvalue C_Lvalue_new_array_index(u32 pos, struct C_ArrayIndex array_index) {
    AST_INIT_NODE(C_Lvalue);

    make(C_ArrayIndex, res.data.array_index, array_index);

    return res;
}

C_ArrayType C_ArrayType_new(u32 pos, C_Expr index, C_Type inner) {
    AST_INIT_NODE(C_ArrayType);

    make(C_Expr, res.index, index);
    make(C_Type, res.inner, inner);

    return res;
}

AST_IMPL_FREE(C_ArrayType, s) {
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
    AST_INIT_NODE(C_Type);

    make(C_ArrayType, res.data.array, t);

    return res;
}

AST_IMPL_FREE(C_Type, t) {
    if (t->kind == C_TYPE_ARRAY) {
        C_ArrayType_free(t->data.array);
        free(t->data.array);
    }
}

C_FunctionArgument C_FunctionArgument_new(u32 pos, C_Identifier ident,
                                          C_Type type) {
    AST_INIT_NODE(C_FunctionArgument);

    make(C_Identifier, res.ident, ident);
    make(C_Type, res.type, type);

    return res;
}

AST_IMPL_FREE(C_FunctionArgument, a) {
    C_Identifier_free(a->ident);
    C_Type_free(a->type);

    free(a->ident);
    free(a->type);
}

C_ArgumentList C_ArgumentList_new(u32 pos, C_FunctionArgument* args,
                                  u32 args_len) {
    return (C_ArgumentList){.pos = pos, .args = args, .args_len = args_len};
}

AST_IMPL_FREE(C_ArgumentList, args) {
    for (u32 i = 0; i < args->args_len; ++i) {
        C_FunctionArgument_free(&args->args[i]);
    }
}

C_UnaryExpr C_UnaryExpr_new(u32 pos, C_UnaryOp op, struct C_Expr inner) {
    AST_INIT_NODE(C_UnaryExpr);

    res.op = op;
    make(C_Expr, res.inner, inner);

    return res;
}

AST_IMPL_FREE(C_UnaryExpr, e) {
    C_Expr_free(e->inner);
    free(e->inner);
}

C_BinaryExpr C_BinaryExpr_new(u32 pos, C_BinaryOp op, struct C_Expr lhs,
                              struct C_Expr rhs) {
    AST_INIT_NODE(C_BinaryExpr);

    make(C_Expr, res.lhs, lhs);
    make(C_Expr, res.rhs, rhs);

    return res;
}

AST_IMPL_FREE(C_BinaryExpr, e) {
    C_Expr_free(e->lhs);
    C_Expr_free(e->rhs);
    free(e->lhs);
    free(e->rhs);
}

C_ArrayIndex C_ArrayIndex_new(u32 pos, struct C_Identifier ident,
                              struct C_Expr index) {
    AST_INIT_NODE(C_ArrayIndex);

    make(C_Identifier, res.ident, ident);
    make(C_Expr, res.index, index);

    return res;
}

void C_Expr_FnCall_free(C_Expr_FnCall* c) {
    C_Identifier_free(c->ident);
    C_ArgumentList_free(c->args);

    free(c->ident);
    free(c->args);
}

C_Assign C_Assign_new(u32 pos, struct C_Lvalue lhs, struct C_Expr rhs) {
    AST_INIT_NODE(C_Assign);

    make(C_Lvalue, res.lhs, lhs);
    make(C_Expr, res.rhs, rhs);

    return res;
}

AST_IMPL_FREE(C_Assign, a) {
    C_Lvalue_free(a->lhs);
    C_Expr_free(a->rhs);

    free(a->lhs);
    free(a->rhs);
}

C_If_Branch C_If_Branch_new_primary(u32 pos, struct C_Expr cond,
                                    struct C_Block block) {
    AST_INIT_NODE(C_If_Branch);

    res.kind = C_IF_PRIMARY;
    make(C_Expr, res.cond, cond);
    make(C_Block, res.block, block);

    return res;
}

C_If_Branch C_If_Branch_new_elseif(u32 pos, struct C_Expr cond,
                                   struct C_Block block) {

    AST_INIT_NODE(C_If_Branch);

    res.kind = C_IF_ELSEIF;
    make(C_Expr, res.cond, cond);
    make(C_Block, res.block, block);

    return res;
}

C_If_Branch C_If_Branch_new_else(u32 pos, struct C_Expr cond,
                                 struct C_Block block) {

    AST_INIT_NODE(C_If_Branch);

    res.kind = C_IF_ELSE;
    make(C_Expr, res.cond, cond);
    make(C_Block, res.block, block);

    return res;
}

AST_IMPL_FREE(C_If_Branch, b) {
    C_Expr_free(b->cond);
    C_Block_free(b->block);

    free(b->cond);
    free(b->block);
}

C_If C_If_new(u32 pos, struct C_If_Branch* branches, u32 branches_len) {
    return (C_If){
        .pos = pos, .branches = branches, .branches_len = branches_len};
}

C_Expr C_Expr_new_unary(C_UnaryExpr e) {
    return (C_Expr){.kind = C_EXPR_UNARYOP, .data.unary = e};
}
C_Expr C_Expr_new_binary(C_BinaryExpr e) {
    return (C_Expr){.kind = C_EXPR_BINOP, .data.binary = e};
}

C_Expr C_Expr_new_array_index(C_ArrayIndex e) {
    return (C_Expr){.kind = C_EXPR_ARRAY_INDEX, .data.array_index = e};
}

C_Expr C_Expr_new_fncall(C_Expr_FnCall e) {
    return (C_Expr){.kind = C_EXPR_FNCALL, .data.fncall = e};
}

C_Expr C_Expr_new_assign(C_Assign e) {
    return (C_Expr){.kind = C_EXPR_ASSIGN, .data.assign = e};
}

C_Expr C_Expr_new_if(C_If e) {
    return (C_Expr){.kind = C_EXPR_IF, .data._if = e};
}

AST_IMPL_FREE(C_Expr, e) {
    switch (e->kind) {
        case C_EXPR_UNARYOP: {
            C_UnaryExpr_free(&e->data.unary);
        } break;
        case C_EXPR_BINOP: {
            C_BinaryExpr_free(&e->data.binary);
        } break;
        case C_EXPR_ARRAY_INDEX: {
            C_ArrayIndex_free(&e->data.array_index);
        } break;
        case C_EXPR_FNCALL: {
            C_Expr_FnCall_free(&e->data.fncall);
        } break;
        case C_EXPR_ASSIGN: {
            C_Assign_free(&e->data.assign);
        } break;
        case C_EXPR_IF: {
            C_If_free(&e->data._if);
        } break;
    }
}
