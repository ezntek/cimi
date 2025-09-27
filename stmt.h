/*
 * cimi: a scuffed scripting language
 *
 * Copyright (c) Eason Qin <eason@ezntek.com>, 2025.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _STMT_H
#define _STMT_H

#include "astcommon.h"
#include "common.h"

// external
struct C_Expr;

typedef struct C_BlockItem {
    struct C_Expr* expr;
    struct C_Stmt* stmt;
} C_BlockItem;
AST_DECL_FREE(C_BlockItem);

C_BlockItem C_BlockItem_new_expr(struct C_Expr expr);
C_BlockItem C_BlockItem_new_stmt(struct C_Stmt stmt);

typedef struct C_Block {
    C_BlockItem* items;
    u32 len;
} C_Block;
AST_DECL_FREE(C_Block);

C_Block C_Block_new(C_BlockItem* items, u32 len);

typedef struct C_Stmt {

} C_Stmt;
AST_DECL_FREE(C_Stmt);

#endif // _STMT_H
