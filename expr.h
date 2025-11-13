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
#include "astcommon.h"
#include "lexertypes.h"

// external
struct C_Stmt;
struct C_Block;

struct C_ArrayType;
struct C_Type;

struct C_Literal;

struct C_Identifier;
struct C_Lvalue;

struct C_FunctionArgument;
struct C_ArgumentList;

struct C_UnaryExpr;
struct C_BinaryExpr;
struct C_ArrayIndex;
struct C_FnCall;

struct C_Assign;

struct C_If_Branch;
struct C_If;

union C_ExprData;
struct C_Expr;

typedef enum C_PrimitiveType {
    C_INT = 0,
    C_FLOAT,
    C_CHAR,
    C_STRING,
    C_BOOL,
    C_ANY,
    C_NULL,
} C_PrimitiveType;

typedef struct C_ArrayType {
    struct C_Expr* size;
    struct C_Type* inner;
    Pos pos;
} C_ArrayType;
AST_DECL_FREE(C_ArrayType);

C_ArrayType C_ArrayType_new(Pos pos, struct C_Expr index, struct C_Type inner);
void C_ArrayType_free(C_ArrayType* s);

typedef enum {
    C_TYPE_PRIMITIVE = 0,
    C_TYPE_ARRAY,
} C__TypeKind;

typedef struct C_Type {
    C__TypeKind kind;
    Pos pos;
    union {
        enum C_PrimitiveType primitive;
        struct C_ArrayType* array;
    } data;
} C_Type;
AST_DECL_FREE(C_Type);

C_Type C_Type_new_primitive(Pos pos, C_PrimitiveType t);
C_Type C_Type_new_array(Pos pos, C_ArrayType t);

typedef struct C_Literal {
    Pos pos;
    C_PrimitiveType type;
    union {
        a_string string;
        char _char;
        bool _bool;
        i64 _int;
        f64 _float;
        // null not needed
    } data;
} C_Literal;

AST_DECL_FREE(C_Literal);

C_Literal C_Literal_new_string(Pos pos, a_string string);
C_Literal C_Literal_new_char(Pos pos, char _char);
C_Literal C_Literal_new_bool(Pos pos, bool _bool);
C_Literal C_Literal_new_int(Pos pos, i64 _int);
C_Literal C_Literal_new_float(Pos pos, f64 _float);
C_Literal C_Literal_new_null(Pos pos);

typedef struct C_Identifier {
    a_string ident;
    Pos pos;
} C_Identifier;
AST_DECL_FREE(C_Identifier);

C_Identifier C_Identifier_new(Pos pos, a_string ident);

typedef enum {
    C_LV_IDENTIFIER = 0,
    C_LV_ARRAY_INDEX,
} C__LvalueKind;

typedef struct C_Lvalue {
    C__LvalueKind kind;
    union {
        struct C_Identifier* ident;
        struct C_ArrayIndex* array_index;
    } data;
    Pos pos;
} C_Lvalue;
AST_DECL_FREE(C_Lvalue);

C_Lvalue C_Lvalue_new_ident(Pos pos, struct C_Identifier ident);
C_Lvalue C_Lvalue_new_array_index(Pos pos, struct C_ArrayIndex array_index);

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

typedef struct C_FunctionArgument {
    struct C_Identifier* ident;
    struct C_Type* type;
    Pos pos;
} C_FunctionArgument;
AST_DECL_FREE(C_FunctionArgument);

C_FunctionArgument C_FunctionArgument_new(Pos pos, C_Identifier ident,
                                          C_Type type);

typedef struct C_ArgumentList {
    struct C_FunctionArgument* args;
    u32 args_len;
    Pos pos;
} C_ArgumentList;
AST_DECL_FREE(C_ArgumentList);

// args must be allocated
C_ArgumentList C_ArgumentList_new(Pos pos, C_FunctionArgument* args,
                                  u32 args_len);
void C_ArgumentList_free(C_ArgumentList* args);

typedef struct C_UnaryExpr {
    C_UnaryOp op;
    struct C_Expr* inner;
    Pos pos;
} C_UnaryExpr;
AST_DECL_FREE(C_UnaryExpr);

C_UnaryExpr C_UnaryExpr_new(Pos pos, C_UnaryOp op, struct C_Expr inner);

typedef struct C_BinaryExpr {
    C_BinaryOp op;
    struct C_Expr* lhs;
    struct C_Expr* rhs;
    Pos pos;
} C_BinaryExpr;
AST_DECL_FREE(C_BinaryExpr);

C_BinaryExpr C_BinaryExpr_new(Pos pos, C_BinaryOp op, struct C_Expr lhs,
                              struct C_Expr rhs);

typedef struct C_ArrayIndex {
    struct C_Expr* ident;
    struct C_Expr* index;
    Pos pos;
} C_ArrayIndex;
AST_DECL_FREE(C_ArrayIndex);

C_ArrayIndex C_ArrayIndex_new(Pos pos, struct C_Expr ident,
                              struct C_Expr index);

typedef struct C_FnCall {
    struct C_Identifier* ident;
    struct C_ArgumentList* args;
    Pos pos;
} C_FnCall;
AST_DECL_FREE(C_FnCall);

C_FnCall C_FnCall_new(Pos pos, C_Identifier ident, C_ArgumentList args);

typedef struct C_Assign {
    struct C_Lvalue* lhs;
    struct C_Expr* rhs;
    Pos pos;
} C_Assign;
AST_DECL_FREE(C_Assign);

C_Assign C_Assign_new(Pos pos, struct C_Lvalue lhs, struct C_Expr rhs);

typedef enum C_If_BranchKind {
    C_IF_PRIMARY = 0,
    C_IF_ELSEIF,
    C_IF_ELSE,
} C_If_BranchKind;

typedef struct C_If_Branch {
    enum C_If_BranchKind kind;
    struct C_Expr* cond;
    struct C_Block* block;
    Pos pos;
} C_If_Branch;
AST_DECL_FREE(C_If_Branch);

C_If_Branch C_If_Branch_new_primary(Pos pos, struct C_Expr cond,
                                    struct C_Block block);
C_If_Branch C_If_Branch_new_elseif(Pos pos, struct C_Expr cond,
                                   struct C_Block block);
C_If_Branch C_If_Branch_new_else(Pos pos, struct C_Expr cond,
                                 struct C_Block block);

typedef struct C_If {
    struct C_If_Branch* branches;
    u32 branches_len;
    Pos pos;
} C_If;
AST_DECL_FREE(C_If);

C_If C_If_new(Pos pos, struct C_If_Branch* branches, u32 branches_len);

typedef enum C_ExprKind {
    C_EXPR_IDENTIFIER = 0,
    C_EXPR_UNARYOP,
    C_EXPR_BINOP,
    C_EXPR_ARRAY_INDEX,
    C_EXPR_FNCALL,
    C_EXPR_ASSIGN,
    C_EXPR_IF,
    C_EXPR_LITERAL,
} C_ExprKind;

typedef union C_ExprData {
    struct C_Identifier* ident;
    struct C_UnaryExpr unary;
    struct C_BinaryExpr binary;
    struct C_ArrayIndex array_index;
    struct C_FnCall fn_call;
    struct C_Assign assign;
    struct C_If _if;
    struct C_Literal literal;
} C_ExprData;

typedef struct C_Expr {
    enum C_ExprKind kind;
    union C_ExprData data;
} C_Expr;
AST_DECL_FREE(C_Expr);

C_Expr C_Expr_new_identifier(C_Identifier i);
C_Expr C_Expr_new_unary(C_UnaryExpr e);
C_Expr C_Expr_new_binary(C_BinaryExpr e);
C_Expr C_Expr_new_array_index(C_ArrayIndex e);
C_Expr C_Expr_new_fncall(C_FnCall e);
C_Expr C_Expr_new_assign(C_Assign e);
C_Expr C_Expr_new_if(C_If e);
C_Expr C_Expr_new_literal(C_Literal e);

#endif // _EXPR_H
