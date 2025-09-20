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
#include <stdio.h>
#include <string.h>

#include "a_string.h"
#include "common.h"
#include "lexer.h"

#define CUR (l->src[l->cur])
#define BUMP_NEWLINE                                                           \
    do {                                                                       \
        l->row++;                                                              \
        l->bol = ++l->cur;                                                     \
    } while (0);
#define IN_BOUNDS (l->cur < l->src_len)
#define POS(sp)                                                                \
    (Pos) {                                                                    \
        .row = l->row, .col = l->cur - l->bol, .span = sp                      \
    }

#define POS_FROM(sv, sp)                                                       \
    (Pos) {                                                                    \
        .row = l->row, .col = sv - l->bol, .span = sp                          \
    }

#define TOKEN(k, sp)                                                           \
    (Token) {                                                                  \
        .kind = TOK_##k, .data = {{NULL}}, .pos = (POS(sp))                    \
    }

#define ERROR_BUFSZ 128

static char ERROR_BUF[ERROR_BUFSZ] = {0};

Token token_new_ident(const char* str) {
    a_string s = astr(str);
    return (Token){
        .kind = TOK_IDENT,
        .data = {s},
    };
}

void token_free(Token* t) {
    if (t->kind == TOK_IDENT) {
        as_free(&t->data.string);
    }
}

a_string token_kind_to_string(TokenKind k) {
    char* s;
    switch (k) {
        case TOK_IDENT: {
            s = "ident";
        } break;
        case TOK_EOF: {
            s = "eof";
        } break;
        case TOK_VAR: {
            s = "var";
        } break;
        case TOK_CONST: {
            s = "const";
        } break;
        case TOK_ECHO: {
            s = "echo";
        } break;
        case TOK_READ: {
            s = "read";
        } break;
        case TOK_AND: {
            s = "and";
        } break;
        case TOK_OR: {
            s = "or";
        } break;
        case TOK_NOT: {
            s = "not";
        } break;
        case TOK_IF: {
            s = "if";
        } break;
        case TOK_THEN: {
            s = "then";
        } break;
        case TOK_ELSE: {
            s = "else";
        } break;
        case TOK_END: {
            s = "end";
        } break;
        case TOK_SWITCH: {
            s = "switch";
        } break;
        case TOK_CASE: {
            s = "case";
        } break;
        case TOK_DEFAULT: {
            s = "default";
        } break;
        case TOK_WHILE: {
            s = "while";
        } break;
        case TOK_FOR: {
            s = "for";
        } break;
        case TOK_FN: {
            s = "fn";
        } break;
        case TOK_RETURN: {
            s = "return";
        } break;
        case TOK_INCLUDE: {
            s = "include";
        } break;
        case TOK_EXPORT: {
            s = "export";
        } break;
        case TOK_BREAK: {
            s = "break";
        } break;
        case TOK_CONTINUE: {
            s = "continue";
        } break;
        case TOK_INT: {
            s = "int";
        } break;
        case TOK_FLOAT: {
            s = "float";
        } break;
        case TOK_BOOL: {
            s = "bool";
        } break;
        case TOK_STRING: {
            s = "string";
        } break;
        case TOK_CHAR: {
            s = "char";
        } break;
        case TOK_NULL: {
            s = "null";
        } break;
        case TOK_LPAREN: {
            s = "lparen";
        } break;
        case TOK_RPAREN: {
            s = "rparen";
        } break;
        case TOK_LBRACKET: {
            s = "lbracket";
        } break;
        case TOK_RBRACKET: {
            s = "rbracket";
        } break;
        case TOK_LCURLY: {
            s = "lcurly";
        } break;
        case TOK_RCURLY: {
            s = "rcurly";
        } break;
        case TOK_COMMA: {
            s = "comma";
        } break;
        case TOK_COLON: {
            s = "colon";
        } break;
        case TOK_SEMICOLON: {
            s = "semicolon";
        } break;
        case TOK_ADD: {
            s = "add";
        } break;
        case TOK_SUB: {
            s = "sub";
        } break;
        case TOK_MUL: {
            s = "mul";
        } break;
        case TOK_DIV: {
            s = "div";
        } break;
        case TOK_PERCENT: {
            s = "percent";
        } break;
        case TOK_CARET: {
            s = "caret";
        } break;
        case TOK_TILDE: {
            s = "tilde";
        } break;
        case TOK_LT: {
            s = "lt";
        } break;
        case TOK_GT: {
            s = "gt";
        } break;
        case TOK_LEQ: {
            s = "leq";
        } break;
        case TOK_GEQ: {
            s = "geq";
        } break;
        case TOK_EQ: {
            s = "eq";
        } break;
        case TOK_NEQ: {
            s = "neq";
        } break;
        case TOK_ASSIGN: {
            s = "assign";
        } break;
        case TOK_SHR: {
            s = "shr";
        } break;
        case TOK_SHL: {
            s = "shl";
        } break;
        case TOK_ADD_ASSIGN: {
            s = "add_assign";
        } break;
        case TOK_SUB_ASSIGN: {
            s = "sub_assign";
        } break;
        case TOK_MUL_ASSIGN: {
            s = "mul_assign";
        } break;
        case TOK_DIV_ASSIGN: {
            s = "div_assign";
        } break;
    }

    return astr(s);
}

void token_print_long(Token* t) {
    printf("token[%d, %d, %d]: ", t->pos.row, t->pos.col, t->pos.span);
    if (t->kind == TOK_IDENT) {
        printf("\"%.*s\"", (int)t->data.string.len, t->data.string.data);
    } else {
        token_print(t);
    }
    putchar('\n');
}

void token_print(Token* t) {
    a_string s = token_kind_to_string(t->kind);
    printf("<%.*s>", (int)s.len, s.data);
    as_free(&s);
}

// lexer stuff

static void lx_trim_spaces(Lexer* l);
static void lx_trim_comment(Lexer* l);
static bool lx_is_separator(char ch);
static bool lx_is_operator_start(char ch);

static bool lx_next_double_symbol(Lexer* l); // true if found
static bool lx_next_single_symbol(Lexer* l); // true if found
static bool lx_next_word(Lexer* l, a_string* res);

static bool lx_is_separator(char ch) {
    return strchr("{}[]();:,", ch);
}

static bool lx_is_operator_start(char ch) {
    return strchr("+-*/=<>^", ch);
}

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

    if (CUR == '\n')
        BUMP_NEWLINE;

    lx_trim_comment(l);
    return;
}

static void lx_trim_comment(Lexer* l) {
    if (l->cur + 2 >= l->src_len) {
        return;
    }

    if (!strncmp(&CUR, "//", 2)) {
        l->cur += 2; // skip past comment marker

        while (IN_BOUNDS && CUR != '\n')
            l->cur++;

        BUMP_NEWLINE;

        lx_trim_spaces(l);
        return;
    }

    if (!strncmp(&CUR, "/*", 2)) {
        l->cur += 2; // skip past

        while (IN_BOUNDS && !strncmp(&CUR, "*/", 2)) {
            if (CUR == '\n') {
                BUMP_NEWLINE;
            } else {
                l->cur++;
            }
        }

        // we found */
        l->cur += 2;

        lx_trim_spaces(l);
        return;
    }
}

static bool lx_next_double_symbol(Lexer* l) {
    if (!lx_is_operator_start(CUR))
        return false;

    if (l->cur + 2 >= l->src_len) {
        return false;
    }

    if (!strncmp(&CUR, "==", 2)) {
        l->token = TOKEN(EQ, 2);
    } else if (!strncmp(&CUR, ">=", 2)) {
        l->token = TOKEN(GEQ, 2);
    } else if (!strncmp(&CUR, "<=", 2)) {
        l->token = TOKEN(LEQ, 2);
    } else if (!strncmp(&CUR, "!=", 2)) {
        l->token = TOKEN(NEQ, 2);
    } else if (!strncmp(&CUR, ">>", 2)) {
        l->token = TOKEN(SHR, 2);
    } else if (!strncmp(&CUR, "<<", 2)) {
        l->token = TOKEN(SHL, 2);
    } else if (!strncmp(&CUR, "+=", 2)) {
        l->token = TOKEN(ADD_ASSIGN, 2);
    } else if (!strncmp(&CUR, "-=", 2)) {
        l->token = TOKEN(SUB_ASSIGN, 2);
    } else if (!strncmp(&CUR, "*=", 2)) {
        l->token = TOKEN(MUL_ASSIGN, 2);
    } else if (!strncmp(&CUR, "/=", 2)) {
        l->token = TOKEN(DIV_ASSIGN, 2);
    } else {
        return false;
    }

    l->token.pos = POS(2);
    l->cur += 2;

    return true;
}

static bool lx_next_single_symbol(Lexer* l) {
    if (!lx_is_separator(CUR))
        return false;

    const TokenKind TABLE[] = {
        ['{'] = TOK_LCURLY,   ['}'] = TOK_RCURLY, ['['] = TOK_LBRACKET,
        [']'] = TOK_RBRACKET, ['('] = TOK_LPAREN, [')'] = TOK_RPAREN,
        [':'] = TOK_COLON,    [','] = TOK_COMMA,  [';'] = TOK_SEMICOLON,
        ['>'] = TOK_LT,       ['<'] = TOK_GT,     ['='] = TOK_EQ,
        ['*'] = TOK_MUL,      ['/'] = TOK_DIV,    ['+'] = TOK_ADD,
        ['-'] = TOK_SUB,      ['^'] = TOK_CARET,  ['%'] = TOK_PERCENT,
    };

    TokenKind t;
    if ((t = TABLE[l->cur]) == 0)
        return false;

    l->token = (Token){
        .kind = t,
        .data.null = 0,
        .pos = POS(1),
    };

    l->cur++;

    return true;
}

static bool lx_next_word(Lexer* l, a_string* res) {
    const char* begin = &CUR;
    u32 len = 0;
    bool delimited_literal = (CUR == '"' || CUR == '\'');
    char delim = 0;
    if (delimited_literal) {
        delim = CUR;
        l->cur++;
        len++;
    }

    do {
        bool stop;
        if (delimited_literal)
            stop = strchr("\n\r", CUR) || CUR == delim;
        else
            stop = lx_is_operator_start(CUR) || lx_is_separator(CUR) ||
                   isspace(CUR);

        if (stop || !IN_BOUNDS)
            break;

        len++;
        l->cur++;
    } while (1);

    if (delimited_literal) {
        len++;
        if (CUR != delim) {
            l->error = LX_ERROR_UNTERMINATED_LITERAL;
            return false;
        } else {
            l->cur++;
        }
    } else if (!IN_BOUNDS) {
        l->error = LX_ERROR_EOF;
        return false;
    }

    *res = as_new();
    as_ncopy_cstr(res, begin, len);

    return true;
}

Token* lx_next_token(Lexer* l) {
    token_free(&l->token);
    l->token = (Token){0};

    lx_trim_spaces(l);
    if (l->cur >= l->src_len) {
        l->token = TOKEN(EOF, 1);
        goto success;
    }

    if (CUR == '\n')
        BUMP_NEWLINE;

    if (lx_next_double_symbol(l))
        goto success;

    if (lx_next_single_symbol(l))
        goto success;

    u32 saved_point = l->cur;
    a_string word = {0};
    if (!lx_next_word(l, &word)) // error
        return NULL;

    l->token = (Token){
        .kind = TOK_IDENT,
        .data.string = word,
        .pos = POS_FROM(saved_point, word.len),
    };

success:
    return &l->token;
}

char* lx_strerror(LexerError e) {
    char* s;
    switch (e) {
        case LX_ERROR_NULL: {
            s = "(no error)";
        } break;
        case LX_ERROR_UNTERMINATED_LITERAL: {
            s = "unterminated string or character literal";
        } break;
        case LX_ERROR_EOF: {
            s = "unexpected end of file";
        } break;
    }

    return strncpy(ERROR_BUF, s, ERROR_BUFSZ);
}

a_string lx_as_strerror(LexerError e) {
    return astr(lx_strerror(e));
}

void lx_perror(LexerError e, const char* pre) {
    char* err = lx_strerror(e);
    printf("%s: %s\n", pre, err);
}

void lx_reset(Lexer* l) {
    l->row = 1;
    l->bol = 0;
    l->cur = 0;
}
