/*
 * cimi: a scuffed scripting language
 *
 * Copyright (c) Eason Qin <eason@ezntek.com>, 2025.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <stdio.h>
#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE

#include <errno.h>
#include <string.h> // used by macro

#include "a_string.h"
#include "a_vector.h"
#include "ast_printer.h"
#include "common.h"
#include "expr.h"
#include "lexer.h"
#include "lexertypes.h"
#include "parser.h"

// #include "tests/ast_printer.c"

A_VECTOR_DECL(Token, Tokens)
A_VECTOR_IMPL(Token, Tokens)

i32 main(i32 argc, char* argv[argc]) {
    argv++;
    argc--;

    a_string s = {0};
    if (argc == 0) {
        s = as_new();
        if (!as_read_line(&s, stdin))
            panic("could not read line from stdin");

    } else {
        s = as_read_file(argv[0]);
        if (errno == ENOENT)
            panic("file \"%s\" not found", argv[0]);
    }

    Lexer l = lx_new(s.data, s.len);
    Tokens toks = Tokens_new();
    Token* tok = {0};
    do {
        tok = lx_next_token(&l);

        if (!tok) {
            lx_perror(l.error.kind, "\033[31;1mlexer error\033[0m");
            break;
        } else {
            token_print_long(tok);
        }

        Tokens_append(&toks, *tok);
    } while (!tok || tok->kind != TOK_EOF);

    a_string filename = {0};
    if (argc == 0) {
        filename = astr("(stdin)");
    } else {
        filename = astr(argv[0]);
    }

    Parser ps = ps_new(filename, toks.data, toks.len);
    MaybeExpr exp = ps_expr(&ps);
    if_let(C_Expr, e, exp) {
        AstPrinter p = ap_new();
        ap_visit_expr(&p, &e);
        putchar('\n');
        C_Expr_free(&e);
    }
    else {
        eprintf("got error\n");
    }

    for (usize i = 0; i < toks.len; i++) {
        token_free(&toks.data[i]);
    }
    Tokens_free(&toks);
    lx_free(&l);
    ps_free(&ps);
    as_free(&s);
    return 0;
}
