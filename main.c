/*
 * cimi: a scuffed scripting language
 *
 * Copyright (c) Eason Qin <eason@ezntek.com>, 2025.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "lexertypes.h"
#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE

#include <errno.h>

#include "a_string.h"
#include "common.h"
#include "lexer.h"

// #include "tests/ast_printer.c"

i32 main(i32 argc, char* argv[argc]) {
    argv++;
    argc--;

    if (argc == 0) {
        panic("no file");
    }

    a_string s = as_read_file(argv[0]);
    if (errno == ENOENT)
        panic("file \"%s\" not found", argv[0]);

    Lexer l = lx_new(s.data, s.len);

    Token* tok = {0};
    do {
        tok = lx_next_token(&l);

        if (!tok) {
            lx_perror(l.error.kind, "\033[31;1mlexer error\033[0m");
        } else {
            token_print_long(tok);
            token_free(tok);
        }
    } while (!tok || tok->kind != TOK_EOF);

    as_free(&s);
    return 0;
}
