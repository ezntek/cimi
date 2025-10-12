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

#include "a_vector.h"
#include "ast.h"
#include "lexer.h"
#include "lexertypes.h"
#include <stdio.h>

A_VECTOR_DECL(Token, Tokens);

typedef struct {
    Lexer lx;
    a_string file_name;
    Tokens tokens;
    u32 cur;
    u32 error_count;
    bool streaming;
} Parser;

Parser ps_new(a_string file_name, Tokens toks);
Parser ps_new_streaming(a_string file_name, Lexer lx);
Parser ps_new_static(a_string file_name, Tokens toks);
void ps_free(Parser* ps);
C_Expr ps_expr(Parser* ps);
C_Block ps_block(Parser* ps);

void ps_diag(Parser* ps, const char* format, ...);

#endif // _PARSER_H
