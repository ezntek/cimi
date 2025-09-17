/*
 * cimi: a scuffed scripting language
 *
 * Copyright (c) Eason Qin <eason@ezntek.com>, 2025.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <ctype.h>
#include <string.h>

#include "a_string.h"
#include "lexer.h"

Token token_new_ident(const char* str) {
    a_string s = astr(str);
    return (Token){
        .kind = TOK_IDENT,
        .ident = {s},
    };
}

void token_free(Token* t) {
    if (t->kind == TOK_IDENT) {
        as_free(&t->ident.string);
    }
}

// lexer stuff

#define CUR       (l->src[l->cur])
#define IN_BOUNDS (l->cur < l->src_len)

static void lx_trim_spaces(Lexer* l);
static void lx_trim_comment(Lexer* l);

Lexer lx_new(const char* src, usize src_len) {
    Lexer res = {.src = src, .src_len = src_len, .row = 1};
    return res;
}

static void lx_trim_spaces(Lexer* l) {
    if (!IN_BOUNDS) {
        return;
    }

    while (l->cur < l->src_len && isspace(CUR) && CUR != '\n')
        l->cur++;

    return lx_trim_comment(l);
}

static void lx_trim_comment(Lexer* l) {
    if (l->cur + 2 > l->src_len) {
        l->cur += 2;
    }

    if (!strncmp(&CUR, "//", 2)) {
        l->cur += 2; // skip past comment marker
        while (IN_BOUNDS && CUR != '\n')
            l->cur++;
        // bump row and bol
        l->row++;
        l->cur++;
        l->bol = l->cur;

        return lx_trim_spaces(l);
    }

    if (!strncmp(&CUR, "/*", 2)) {
        l->cur += 2; // skip past

        while (IN_BOUNDS && !strncmp(&CUR, "*/", 2)) {
            if (CUR == '\n') {
                l->row++;
                l->bol = ++l->cur;
            } else {
                l->cur++;
            }
        }

        // we found */
        l->cur += 2;

        return lx_trim_spaces(l);
    }
}

Token* lx_next_token(Lexer* l) {
    if (l->cur == l->src_len) {
        l->token = TOKEN(EOF);
        goto end;
    }

    lx_trim_spaces(l);

end:
    return &l->token;
}

void lx_reset(Lexer* l) {
    l->row = 1;
    l->bol = 0;
    l->cur = 0;
}
