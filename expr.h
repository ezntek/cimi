/*
 * cimi: a scuffed scripting language
 *
 * Copyright (c) Eason Qin <eason@ezntek.com>, 2025.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _EXPR_H
#define _EXPR_H

#include "a_string.h"
#include "lexertypes.h"

struct C_Identifier;
enum C_PrimitiveType;
struct C_ArrayType;
struct C_Type;
enum C_ExprKind;
struct C_Expr_Unary;
struct C_Expr_Binary;
struct C_Expr_ArrayIndex;
struct C_FunctionArgument;
struct C_ArgumentList;
struct C_Expr_FnCall;
union C_ExprData;
struct C_Expr;

typedef struct C_Identifier {
    a_string ident;
    u32 pos;
} C_Identifier;

C_Identifier C_Identifier_new(u32 pos, a_string ident);
void C_Identifier_free(C_Identifier* id);

typedef enum C_UnaryOp {
    C_UNARYOP_GROUPING = 0,
    C_UNARYOP_NEGATION,
    C_UNARYOP_NOT,
} C_UnaryOp;

typedef enum C_BinaryOp {
    C_BINARYOP_ADD = 0,
    C_BINARYOP_SUB,
    C_BINARYOP_MUL,
    C_BINARYOP_DIV,
    C_BINARYOP_POW,
    C_BINARYOP_NEQ,
    C_BINARYOP_GEQ,
    C_BINARYOP_LEQ,
    C_BINARYOP_LT,
    C_BINARYOP_GT,
    C_BINARYOP_EQ,
} C_BinaryOp;

typedef enum C_PrimitiveType {
    C_INT = 0,
    C_FLOAT,
    C_CHAR,
    C_STRING,
    C_BOOLEAN,
    C_NULL,
} C_PrimitiveType;

typedef struct C_ArrayType {
    struct C_Expr* index;
    struct C_Type* inner;
    u32 pos;
} C_ArrayType;

C_ArrayType C_ArrayType_new(u32 pos, struct C_Expr index, struct C_Type inner);
void C_ArrayType_free(C_ArrayType* s);

typedef enum {
    C_TYPE_PRIMITIVE = 0,
    C_TYPE_ARRAY,
} C__TypeVariant;

typedef struct C_Type {
    C__TypeVariant kind;
    u32 pos;
    union {
        enum C_PrimitiveType primitive;
        struct C_ArrayType* array;
    } data;
} C_Type;

C_Type C_Type_new_primitive(u32 pos, C_PrimitiveType t);
C_Type C_Type_new_array(u32, C_ArrayType t);
void C_Type_free(C_Type* t);

typedef enum C_ExprKind {
    C_EXPR_UNARYOP = 0,
    C_EXPR_BINOP,
    C_EXPR_ARRAY_INDEX,
    C_EXPR_FNCALL,
} C_ExprKind;

typedef struct C_Expr_Unary {
    C_UnaryOp op;
    struct C_Expr* inner;
    u32 pos;
} C_Expr_Unary;

C_Expr_Unary C_Expr_Unary_new(u32 pos, C_UnaryOp op, struct C_Expr inner);
void C_Expr_Unary_free(C_Expr_Unary* e);

typedef struct C_Expr_Binary {
    C_BinaryOp op;
    struct C_Expr* lhs;
    struct C_Expr* rhs;
    u32 pos;
} C_Expr_Binary;

C_Expr_Binary C_Expr_Binary_new(u32 pos, C_BinaryOp op, struct C_Expr lhs,
                                struct C_Expr rhs);
void C_Expr_Binary_free(C_Expr_Binary* e);

typedef struct C_Expr_ArrayIndex {
    struct C_Identifier* ident;
    struct C_Expr* index;
    u32 pos;
} C_Expr_ArrayIndex;

C_Expr_ArrayIndex C_Expr_ArrayIndex_new(u32 pos, struct C_Identifier ident,
                                        struct C_Expr index);
void C_Expr_ArrayIndex_free(C_Expr_ArrayIndex* e);

typedef struct C_FunctionArgument {
    struct C_Identifier* ident;
    struct C_Type* type;
    u32 pos;
} C_FunctionArgument;

C_FunctionArgument C_FunctionArgument_new(u32 pos, C_Identifier ident,
                                          C_Type type);
void C_FunctionArgument_free(C_FunctionArgument* a);

typedef struct C_ArgumentList {
    struct C_FunctionArgument** args;
    u32 nargs;
    u32 pos;
} C_ArgumentList;

// args must be allocated
C_ArgumentList C_ArgumentList_new(u32 pos, C_FunctionArgument** args,
                                  u32 nargs);
void C_ArgumentList_free(C_ArgumentList* args);

typedef struct C_Expr_FnCall {
    struct C_Identifier* ident;
    struct C_ArgumentList* args;
    u32 pos;
} C_Expr_FnCall;

C_Expr_FnCall C_Expr_FnCall_new(u32 pos, C_Identifier ident,
                                C_ArgumentList args);
void C_Expr_FnCall_free(C_Expr_FnCall* c);

typedef union C_ExprData {
    struct C_Expr_Unary unary;
    struct C_Expr_Binary binary;
    struct C_Expr_ArrayIndex array_index;
    struct C_Expr_FnCall fncall;
} C_ExprData;

typedef struct C_Expr {
    enum C_ExprKind kind;
    union C_ExprData data;
} C_Expr;

C_Expr C_Expr_new_unary(C_Expr_Unary e);
C_Expr C_Expr_new_binary(C_Expr_Binary e);
C_Expr C_Expr_new_array_index(C_Expr_ArrayIndex e);
C_Expr C_Expr_new_fncall(C_Expr_FnCall e);
void C_Expr_free(C_Expr* e);

#endif // _EXPR_H
