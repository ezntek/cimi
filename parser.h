/*
 * cimi: a scuffed scripting language
 *
 * Copyright (c) Eason Qin <eason@ezntek.com>, 2025.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _PARSER_H
#define _PARSER_H

#include <stdio.h>

#include "a_vector.h"
#include "ast.h"
#include "lexer.h"
#include "lexertypes.h"

A_VECTOR_DECL(Token, Tokens);

typedef struct {
    Lexer lx;
    a_string file_name;
    Tokens tokens;
    u32 error_count;
    u32 cur;
    bool error_reported;
} Parser;

#define DECL_MAYBE(T, name)                                                    \
    typedef struct {                                                           \
        T data;                                                                \
        bool have;                                                             \
    } name

DECL_MAYBE(Token*, MaybeToken);

#define HAVE_TOKEN(tok)                                                        \
    (MaybeToken) {                                                             \
        .have = true, .data = (tok)                                            \
    }
#define NO_TOKEN                                                               \
    (MaybeToken) {                                                             \
        .have = false                                                          \
    }

DECL_MAYBE(C_Expr, MaybeExpr);

#define HAVE_EXPR(exp)                                                         \
    (MaybeExpr) {                                                              \
        .have = true, .data = exp                                              \
    }
#define NO_EXPR                                                                \
    (MaybeExpr) {                                                              \
        .have = false                                                          \
    }

Parser ps_new(a_string file_name, Tokens toks);
void ps_free(Parser* ps);
MaybeExpr ps_expr(Parser* ps);
C_Block ps_block(Parser* ps);

void ps_diag(Parser* ps, const char* format, ...);

#endif // _PARSER_H
