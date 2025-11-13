/*
 * cimi: a scuffed scripting language
 *
 * Copyright (c) Eason Qin <eason@ezntek.com>, 2025.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "expr.h"
#include "stmt.h"
#include <ctype.h>
#include <errno.h>
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

#define MAX_ERROR_COUNT 20

Parser ps_new(a_string file_name, Token* toks, usize len) {
    Parser p = {.tokens = toks, .tokens_len = len, .file_name = file_name};
    return p;
}

void ps_free(Parser* ps) {
    lx_free(&ps->lx);
    as_free(&ps->file_name);
}

static MaybeToken ps_consume(Parser* ps);
static MaybeToken ps_peek(Parser* ps);
static MaybeToken ps_peek_next(Parser* ps);
static MaybeToken ps_prev(Parser* ps);
static MaybeToken ps_get(Parser* ps, u32 idx);
static MaybeToken ps_peek_and_expect(Parser* ps, TokenKind expected);
static bool ps_check(Parser* ps, TokenKind expected);
static MaybeToken ps_check_and_consume(Parser* ps, TokenKind expected);
static MaybeToken ps_consume_and_expect(Parser* ps, TokenKind expected);
static Pos ps_get_pos(Parser* ps);
static bool ps_bump_error_count(Parser* ps);
static void ps_diag_expected(Parser* ps, const char* thing);
static void ps_diag_and_skip(Parser* ps, const char* format, ...);

static MaybeToken ps_consume(Parser* ps) {
    if (++ps->cur >= ps->tokens_len) {
        ps->eof = true;
        return NO_TOKEN;
    } else {
        return HAVE_TOKEN(&ps->tokens[ps->cur - 1]);
    }
}

static MaybeToken ps_peek(Parser* ps) {
    if (ps->cur < ps->tokens_len) {
        return HAVE_TOKEN(&ps->tokens[ps->cur]);
    } else {
        ps->eof = true;
        return NO_TOKEN;
    }
}

static MaybeToken ps_peek_next(Parser* ps) {
    if (ps->cur + 1 < ps->tokens_len) {
        return HAVE_TOKEN(&ps->tokens[ps->cur + 1]);
    } else {
        ps->eof = true;
        return NO_TOKEN;
    }
}

static MaybeToken ps_prev(Parser* ps) {
    if (ps->cur - 1 < ps->tokens_len) {
        return HAVE_TOKEN(&ps->tokens[ps->cur - 1]);
    } else {
        ps->eof = true;
        return NO_TOKEN;
    }
}

static MaybeToken ps_get(Parser* ps, u32 idx) {
    if (idx < ps->tokens_len) {
        return HAVE_TOKEN(&ps->tokens[idx]);
    } else {
        ps->eof = true;
        return NO_TOKEN;
    }
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

    as_free(&expected_s);
    return NO_TOKEN;
}

static bool ps_check(Parser* ps, TokenKind expected) {
    if_let(Token*, t, ps_peek(ps)) {
        return t->kind == expected;
    }
    else {
        return false;
    }
}

static MaybeToken ps_check_and_consume(Parser* ps, TokenKind expected) {
    if (ps_check(ps, expected)) {
        return ps_consume(ps);
    } else {
        return NO_TOKEN;
    }
}

static MaybeToken ps_consume_and_expect(Parser* ps, TokenKind expected) {
    a_string expected_s = token_kind_to_string(expected);
    if_let(Token*, t, ps_consume(ps)) {
        if (t->kind != expected) {
            a_string actual_s = token_kind_to_string(t->kind);
            ps_diag(ps, "expected token \"%s\" but got \"%s\"", expected_s.data,
                    actual_s.data);
            as_free(&actual_s);
        } else {
            return HAVE_TOKEN(t);
        }
    }
    else {
        ps_diag(ps,
                "expected token \"%s\" but reached the end of the token stream",
                expected_s.data);
    }
    as_free(&expected_s);
    return NO_TOKEN;
}

static Pos ps_get_pos(Parser* ps) {
    if_let(Token*, t, ps_prev(ps)) {
        return t->pos;
    }
    else {
        panic("no previous token");
    }
}

void ps_diag_at(Parser* ps, Pos pos, const char* format, ...) {
    eprintf("\033[31;1merror: \033[0;1m%.*s:%u:%u: \033[0m",
            (int)ps->file_name.len, ps->file_name.data, pos.row, pos.col);

    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);

    putchar('\n');
    ps->error_reported = true;
}

void ps_diag(Parser* ps, const char* format, ...) {
    Pos pos = ps_get_pos(ps);
    // FIXME: less code duplication due to va_list
    eprintf("\033[31;1merror: \033[0;1m%.*s:%u:%u: \033[0m",
            (int)ps->file_name.len, ps->file_name.data, pos.row, pos.col);

    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);

    putchar('\n');
    ps->error_reported = true;
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

MaybeExpr ps_ident(Parser* ps) {
    if_let(Token*, t, ps_peek_and_expect(ps, TOK_IDENT)) {
        C_Identifier id = C_Identifier_new(t->pos, as_dupe(&t->data.string));
        C_Expr res = C_Expr_new_identifier(id);
        return HAVE_EXPR(res);
    }
    else {
        return NO_EXPR;
    }
}

static char resolve_escape(char ch) {
    switch (ch) {
        case 'a': {
            return '\a';
        } break;
        case 'b': {
            return '\b';
        } break;
        case 'e': {
            return '\033';
        } break;
        case 'n': {
            return '\n';
        } break;
        case 'r': {
            return '\r';
        } break;
        case 't': {
            return '\t';
        } break;
        case '\\': {
            return '\\';
        } break;
        case '\'': {
            return '\'';
        } break;
        case '"': {
            return '\"';
        } break;
        default: return -1;
    }
}

static bool is_int(a_string* s) {
    if (!isdigit(as_first(s)))
        return false;

    char ch;
    for (usize i = 1; i < s->len; ++i) {
        ch = as_at(s, i);
        if (!isdigit(ch) && ch != '_')
            return false;
    }

    return true;
}

static bool is_float(a_string* s) {
    if (!isdigit(as_first(s)) && as_first(s) != '.')
        return false;

    if (is_int(s))
        return false;

    bool found = false;
    char ch;
    // starting from 0 because the first item might be a '.'
    for (usize i = 0; i < s->len; ++i) {
        ch = as_at(s, i);
        if (ch == '.') {
            if (found)
                return false;
            found = true;
            continue;
        }

        if (!isdigit(ch) && ch != '_')
            return false;
    }

    return true;
}

MaybeExpr ps_literal(Parser* ps) {
    let_else(Token*, t, ps_peek(ps)) {
        return NO_EXPR;
    }

    C_Expr retval = {0};
    a_string* s = &t->data.string;

    switch (t->kind) {
        case TOK_NULL: {
            retval = C_Expr_new_literal(C_Literal_new_null(t->pos));
            goto ok;
        } break;
        case TOK_LITERAL_CHAR: {
            if (s->len == 0) {
                ps_diag_at(ps, t->pos,
                           "not enough characters in character literal");
                return NO_EXPR;
            }
            char ch;
            if (as_at(s, 0) == '\\') {
                if (s->len == 1) {
                    ps_diag_at(ps, t->pos, // FIXME: position
                               "invalid escape sequence in character literal");
                    return NO_EXPR;
                }

                if ((ch = resolve_escape(as_at(s, 1))) == -1) {
                    ps_diag_at(ps, t->pos, // FIXME: position
                               "invalid escape in character literal");
                    return NO_EXPR;
                }
            } else if (s->len >= 2) {
                ps_diag_at(ps, t->pos, "character literal is too long!");
            } else {
                ch = as_at(s, 0);
            }
            retval = C_Expr_new_literal(C_Literal_new_char(t->pos, ch));
            goto ok;
        } break;
        case TOK_LITERAL_STRING: {
            a_string* s = &t->data.string;
            a_string res = as_new();

            char ch;
            for (usize i = 0; i < s->len; ++i) {
                ch = as_at(s, i);
                if (ch == '\\') {
                    // last character is an escape
                    if (i == s->len - 1) {
                        ps_diag_at(
                            ps, t->pos, // FIXME: pos
                            "last character of string literal is an escape!");
                        as_free(&res);
                        return NO_EXPR;
                    }
                    ch = resolve_escape(as_at(s, ++i));
                    if (ch == -1) {
                        ps_diag_at(ps, t->pos, // FIXME: pos
                                   "invalid escape sequence in string literal");
                        as_free(&res);
                        return NO_EXPR;
                    }
                }

                as_append_char(&res, ch);
            }

            retval = C_Expr_new_literal(C_Literal_new_string(t->pos, res));
            as_free(&res); // duped already
            goto ok;
        } break;
        case TOK_LITERAL_NUMBER: {
            if (is_float(s)) {
                double res;
                usize erridx = 0;
                if ((erridx = as_to_double(s, &res)) != s->len) {
                    if (errno == ERANGE) {
                        ps_diag_at(ps, t->pos,
                                   "float literal \"%s\" is either too large "
                                   "or too small!",
                                   s->data);
                        return NO_EXPR;
                    } else {
                        Pos p = t->pos;
                        p.col += erridx; // XXX: idk if this is even right lol
                        p.span -= erridx;
                        ps_diag_at(ps, p, "float literal \"%s\" is invalid!",
                                   s->data);
                        return NO_EXPR;
                    }
                }

                retval = C_Expr_new_literal(C_Literal_new_float(t->pos, res));
                goto ok;
            } else if (is_int(s)) {
                int64_t res;
                usize erridx = 0;
                if ((erridx = as_to_integer(s, &res, 0)) != s->len) {
                    if (errno == ERANGE) {
                        ps_diag_at(ps, t->pos,
                                   "int literal \"%s\" is either too large or "
                                   "too small!",
                                   s->data);
                        return NO_EXPR;
                    } else {
                        Pos p = t->pos;
                        p.col += erridx;
                        p.span -= erridx;
                        ps_diag_at(ps, p, "int literal \"%s\" is invalid!",
                                   s->data);
                        return NO_EXPR;
                    }
                }

                retval = C_Expr_new_literal(C_Literal_new_int(t->pos, res));
                goto ok;
            } else {
                ps_diag_at(ps, t->pos, "found invalid number literal \"%s\"",
                           s->data);
                return NO_EXPR;
            }
        } break;
        case TOK_LITERAL_BOOLEAN: {
            retval =
                C_Expr_new_literal(C_Literal_new_bool(t->pos, t->data.boolean));
            goto ok;
        } break;
        default: return NO_EXPR;
    }
ok:
    return HAVE_EXPR(retval);
}

MaybeExpr ps_unary_expr(Parser* ps) {
    return NO_EXPR;
}

MaybeExpr ps_expr(Parser* ps) {
    return ps_literal(ps);
}

C_Block ps_block(Parser* ps) {
    return C_Block_new(NULL, 0);
}
