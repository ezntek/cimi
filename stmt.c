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

#include <stdlib.h>

#include "astcommon.h"
#include "common.h"
#include "expr.h"
#include "stmt.h"

C_BlockItem C_BlockItem_new_expr(struct C_Expr expr) {
    C_BlockItem res = {0};
    make(C_Expr, res.expr, expr);
    return res;
}

C_BlockItem C_BlockItem_new_stmt(struct C_Stmt stmt) {
    C_BlockItem res = {0};
    make(C_Stmt, res.stmt, stmt);
    return res;
}

AST_IMPL_FREE(C_BlockItem, itm) {
    if (itm->stmt) {
        C_Stmt_free(itm->stmt);
        free(itm->stmt);
    } else if (itm->expr) {
        C_Expr_free(itm->expr);
        free(itm->expr);
    }
}

C_Block C_Block_new(C_BlockItem* items, u32 len) {
    return (C_Block){
        .items = items,
        .len = len,
    };
}

AST_IMPL_FREE(C_Block, b) {
    for (u32 i = 0; i < b->len; ++i) {
        C_BlockItem_free(&b->items[i]);
    }
    free(b->items);
}

AST_IMPL_FREE(C_Stmt, stmt) {
    return;
}
