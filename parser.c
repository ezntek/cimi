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

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "a_string.h"
#include "a_vector.h"
#include "common.h"
#include "lexer.h"
#include "lexertypes.h"
#include "parser.h"

A_VECTOR_IMPL(Token, Tokens);

#define MAX_ERROR_COUNT 20

Parser ps_new(a_string file_name, Tokens toks) {
    return ps_new_static(file_name, toks);
}

Parser ps_new_streaming(a_string file_name, Lexer lx) {
    panic("not implemented");
}

Parser ps_new_static(a_string file_name, Tokens toks) {
    Parser p = {.streaming = false, .tokens = toks, .file_name = file_name};
    return p;
}

void ps_free(Parser* ps) {
    lx_free(&ps->lx);
    as_free(&ps->file_name);
    Tokens_free(&ps->tokens);
}

// local stuff
typedef struct {
    Token* data;
    bool have; // Singapore flavored!
} MaybeToken;

#define HAVE_TOKEN(tok)                                                        \
    (MaybeToken) {                                                             \
        .have = true, .data = (tok)                                            \
    }
#define NO_TOKEN                                                               \
    (MaybeToken) {                                                             \
        .have = false                                                          \
    }

static MaybeToken ps_consume(Parser* ps);
static MaybeToken ps_peek(Parser* ps);
static MaybeToken ps_peek_next(Parser* ps);
static MaybeToken ps_prev(Parser* ps);
static MaybeToken ps_get(Parser* ps, u32 idx);
static MaybeToken ps_peek_and_expect(Parser* ps, TokenKind expected);
static bool ps_check(Parser* ps, TokenKind expected);
static MaybeToken ps_check_and_consume(Parser* ps, TokenKind expected);
static MaybeToken ps_consume_and_check(Parser* ps, TokenKind expected);
static Pos ps_get_span(Parser* ps);
static bool ps_bump_error_count(Parser* ps);
static void ps_diag_expected(Parser* ps, const char* thing);
static void ps_diag_and_skip(Parser* ps, const char* format, ...);

static MaybeToken ps_consume(Parser* ps) {
    if (++ps->cur >= ps->tokens.len)
        return NO_TOKEN;
    else
        return HAVE_TOKEN(&ps->tokens.data[ps->cur - 1]);
}

static MaybeToken ps_peek(Parser* ps) {
    if (ps->cur < ps->tokens.len)
        return HAVE_TOKEN(&ps->tokens.data[ps->tokens.len - 1]);
    else
        return NO_TOKEN;
}

static MaybeToken ps_peek_next(Parser* ps) {
    if (ps->cur + 1 < ps->tokens.len)
        return HAVE_TOKEN(&ps->tokens.data[ps->cur + 1]);
    else
        return NO_TOKEN;
}

static MaybeToken ps_prev(Parser* ps) {
    if (ps->cur - 1 < ps->tokens.len)
        return HAVE_TOKEN(&ps->tokens.data[ps->cur - 1]);
    else
        return NO_TOKEN;
}

static MaybeToken ps_get(Parser* ps, u32 idx) {
    if (idx < ps->tokens.len)
        return HAVE_TOKEN(&ps->tokens.data[idx]);
    else
        return NO_TOKEN;
}

static MaybeToken ps_peek_and_expect(Parser* ps, TokenKind expected) {
    a_string expected_s = token_kind_to_string(expected);
    if_let(Token*, t, ps_peek(ps)) {
        if (t->kind == TOK_EOF) {
            ps_diag(ps, "expected token %s, but reached end of file",
                    expected_s);
        } else if (t->kind != expected) {
            a_string got_s = token_kind_to_string(t->kind);
            ps_diag(ps, "expected token %s, but found %s", got_s, expected_s);
        } else {
            as_free(&expected_s);
            return HAVE_TOKEN(t);
        }
    }
    else {
        ps_diag(ps, "expected token %s, but got no token", expected_s);
    }
end:
    as_free(&expected_s);
    return NO_TOKEN;
}

static bool ps_check(Parser* ps, TokenKind expected) {
    //
}

static MaybeToken ps_check_and_consume(Parser* ps, TokenKind expected) {
    //
}

static MaybeToken ps_consume_and_check(Parser* ps, TokenKind expected) {
    //
}

static Pos ps_get_span(Parser* ps) {
    if_let(Token*, t, ps_prev(ps)) {
        return t->pos;
    }
    else {
        panic("no previous token");
    }
}

void ps_diag(Parser* ps, const char* format, ...) {
    Pos pos = ps_get_span(ps);

    eprintf("\033[31;1merror: \033[0;1m%.*s:%u:%u: \033[0m",
            (int)ps->file_name.len, ps->file_name.data, pos.row, pos.col);

    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);

    putchar('\n');
}

static bool ps_bump_error_count(Parser* ps) {
    return ++ps->cur > MAX_ERROR_COUNT;
}

static void ps_diag_expected(Parser* ps, const char* thing) {
    if_let(Token*, tok, ps_peek(ps)) {
        a_string tokstring = token_kind_to_string(tok->kind);
        ps_diag(ps, "expected %s, but found token \"%s\"", tokstring.data);
        as_free(&tokstring);
    }
    else {
        ps_diag(ps, "expected %s, but found no token", thing);
    }
}

static void ps_diag_and_skip(Parser* ps, const char* format, ...);

static void ps_consume_newlines(Parser* ps) {
    while_let(Token*, tok, ps_peek(ps)) {
        if (tok->kind == TOK_NEWLINE) {
            ps_consume(ps);
        } else {
            break;
        }
    }
}

C_Expr ps_expr(Parser* ps) {
    // TODO: implement
}

C_Block ps_block(Parser* ps) {
    // TODO: implement
}
