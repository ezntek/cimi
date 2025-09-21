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

#include "3rdparty/uthash.h"
#include "a_string.h"
#include "common.h"
#include "lexer.h"

#define CUR (l->src[l->cur])
#define BUMP_NEWLINE                                                           \
    do {                                                                       \
        l->row++;                                                              \
        l->bol = ++l->cur;                                                     \
    } while (0)
#define IN_BOUNDS (l->cur < l->src_len)
#define POS(sp)                                                                \
    (Pos) {                                                                    \
        .row = l->row, .col = l->cur - l->bol + 1 - (sp), .span = (sp)         \
    }

#define POS_HERE(sp)                                                           \
    (Pos) {                                                                    \
        .row = l->row, .col = l->cur - l->bol + 1, .span = (sp)                \
    }

#define TOKEN(kfull, sp)                                                       \
    (Token) {                                                                  \
        .kind = (kfull), .data = {{NULL}}, .pos = (POS(sp))                    \
    }

#define TOK(k, sp)                                                             \
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
    if (t->kind == TOK_IDENT || t->kind == TOK_LITERAL_STRING ||
        t->kind == TOK_LITERAL_CHAR || t->kind == TOK_LITERAL_NUMBER) {
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
        case TOK_INVALID: {
            s = "!!! BOGUS AMOGUS TOKEN !!!";
        } break;
        case TOK_NEWLINE: {
            s = "newline";
        } break;
        case TOK_LITERAL_STRING: {
            s = "literal_string";
        } break;
        case TOK_LITERAL_CHAR: {
            s = "literal_char";
        } break;
        case TOK_LITERAL_NUMBER: {
            s = "literal_number";
        } break;
        case TOK_LITERAL_BOOLEAN: {
            s = "literal_boolean";
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

    const a_string* s = &t->data.string;
    switch (t->kind) {
        case TOK_IDENT: {
            printf("(%.*s)", (int)s->len, s->data);
        } break;
        case TOK_LITERAL_STRING: {
            printf("\"%.*s\"", (int)s->len, s->data);
        } break;
        case TOK_LITERAL_CHAR: {
            printf("'%.*s'", (int)s->len, s->data);
        } break;
        case TOK_LITERAL_NUMBER: {
            printf("%.*s", (int)s->len, s->data);
        } break;
        case TOK_LITERAL_BOOLEAN: {
            if (t->data.boolean)
                printf("<true>");
            else
                printf("<false>");
        } break;
        default: token_print(t);
    }

    putchar('\n');
}

void token_print(Token* t) {
    a_string s = token_kind_to_string(t->kind);
    printf("<%.*s>", (int)s.len, s.data);
    as_free(&s);
}

// lexer stuff
#define LX_KWT_STRSZ 64

typedef struct {
    char txt[LX_KWT_STRSZ];
    TokenKind kw;
    UT_hash_handle hh;
} KeywordTable;

KeywordTable* kwt;

static void lx_kwt_setup(void);
static void lx_kwt_add(const char* s, TokenKind k);
static TokenKind lx_kwt_get(const char* s);
static void lx_kwt_free(void);

static void lx_kwt_setup(void) {
    kwt = NULL;

    lx_kwt_add("var", TOK_VAR);
    lx_kwt_add("const", TOK_CONST);
    lx_kwt_add("echo", TOK_ECHO);
    lx_kwt_add("read", TOK_READ);
    lx_kwt_add("and", TOK_AND);
    lx_kwt_add("or", TOK_OR);
    lx_kwt_add("not", TOK_NOT);
    lx_kwt_add("if", TOK_IF);
    lx_kwt_add("then", TOK_THEN);
    lx_kwt_add("else", TOK_ELSE);
    lx_kwt_add("end", TOK_END);
    lx_kwt_add("switch", TOK_SWITCH);
    lx_kwt_add("case", TOK_CASE);
    lx_kwt_add("default", TOK_DEFAULT);
    lx_kwt_add("while", TOK_WHILE);
    lx_kwt_add("for", TOK_FOR);
    lx_kwt_add("fn", TOK_FN);
    lx_kwt_add("return", TOK_RETURN);
    lx_kwt_add("include", TOK_INCLUDE);
    lx_kwt_add("export", TOK_EXPORT);
    lx_kwt_add("break", TOK_BREAK);
    lx_kwt_add("continue", TOK_CONTINUE);
    lx_kwt_add("int", TOK_INT);
    lx_kwt_add("float", TOK_FLOAT);
    lx_kwt_add("bool", TOK_BOOL);
    lx_kwt_add("string", TOK_STRING);
    lx_kwt_add("char", TOK_CHAR);
    lx_kwt_add("null", TOK_NULL);
}

static void lx_kwt_add(const char* s, TokenKind k) {
    KeywordTable* row = malloc(sizeof(KeywordTable));
    check_alloc(row);
    strncpy(row->txt, s, LX_KWT_STRSZ);
    row->kw = k;
    HASH_ADD_STR(kwt, txt, row);
}

static TokenKind lx_kwt_get(const char* s) {
    KeywordTable* out;
    HASH_FIND_STR(kwt, s, out);
    if (out)
        return out->kw;
    else
        return TOK_INVALID;
}

static void lx_kwt_free(void) {
    KeywordTable *elem, *tmp;

    HASH_ITER(hh, kwt, elem, tmp) {
        HASH_DEL(kwt, elem);
        free(elem);
    }
}

static void lx_trim_spaces(Lexer* l);
static void lx_trim_comment(Lexer* l);
static bool lx_is_separator(char ch);
static bool lx_is_operator_start(char ch);

static bool lx_next_double_symbol(Lexer* l); // true if found
static bool lx_next_single_symbol(Lexer* l); // true if found
static bool lx_next_word(Lexer* l, a_string* res);
static bool lx_next_keyword(Lexer* l, const a_string* word);
static bool lx_next_literal(Lexer* l, const a_string* word);

static bool lx_is_separator(char ch) {
    return strchr("{}[]();:,", ch);
}

static bool lx_is_operator_start(char ch) {
    return strchr("+-*/=<>^!", ch);
}

Lexer lx_new(const char* src, usize src_len) {
    lx_kwt_setup();
    Lexer res = {.src = src, .src_len = src_len, .row = 1};
    return res;
}

static void lx_trim_spaces(Lexer* l) {
    if (!IN_BOUNDS) {
        return;
    }

    while (l->cur < l->src_len && isspace(CUR) && CUR != '\n')
        l->cur++;

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

        lx_trim_spaces(l);
        return;
    }

    if (!strncmp(&CUR, "/*", 2)) {
        l->cur += 2; // skip past

        while (IN_BOUNDS && strncmp(&CUR, "*/", 2)) {
            if (CUR == '\n')
                BUMP_NEWLINE;
            else
                l->cur++;
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
        l->token = TOK(EQ, 2);
    } else if (!strncmp(&CUR, ">=", 2)) {
        l->token = TOK(GEQ, 2);
    } else if (!strncmp(&CUR, "<=", 2)) {
        l->token = TOK(LEQ, 2);
    } else if (!strncmp(&CUR, "!=", 2)) {
        l->token = TOK(NEQ, 2);
    } else if (!strncmp(&CUR, ">>", 2)) {
        l->token = TOK(SHR, 2);
    } else if (!strncmp(&CUR, "<<", 2)) {
        l->token = TOK(SHL, 2);
    } else if (!strncmp(&CUR, "+=", 2)) {
        l->token = TOK(ADD_ASSIGN, 2);
    } else if (!strncmp(&CUR, "-=", 2)) {
        l->token = TOK(SUB_ASSIGN, 2);
    } else if (!strncmp(&CUR, "*=", 2)) {
        l->token = TOK(MUL_ASSIGN, 2);
    } else if (!strncmp(&CUR, "/=", 2)) {
        l->token = TOK(DIV_ASSIGN, 2);
    } else {
        return false;
    }

    l->token.pos = POS_HERE(2);
    l->cur += 2;

    return true;
}

static bool lx_next_single_symbol(Lexer* l) {
    if (!lx_is_separator(CUR) && !lx_is_operator_start(CUR))
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
    if ((t = TABLE[(int)CUR]) == 0)
        return false;

    l->token = (Token){
        .kind = t,
        .data.null = 0,
        .pos = POS_HERE(1),
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

static bool lx_next_keyword(Lexer* l, const a_string* word) {
    if (word->len >= LX_KWT_STRSZ)
        return false;

    TokenKind kw;
    a_string word_lower = as_tolower(word);
    if ((kw = lx_kwt_get(word_lower.data)) != TOK_INVALID) {
        l->token = TOKEN(kw, word_lower.len);
        as_free(&word_lower);
        return true;
    } else {
        as_free(&word_lower);
        return false;
    }
}

static bool lx_next_literal(Lexer* l, const a_string* word) {
    if (as_first(word) == '"') {
        if (word->len == 1)
            unreachable;

        // TODO: escape sequences
        a_string contents = as_slice(word, 1, word->len - 1);
        l->token = (Token){
            .kind = TOK_LITERAL_STRING,
            .pos = POS(word->len),
            .data.string = contents,
        };
        return true;
    }
    return false;
}

Token* lx_next_token(Lexer* l) {
    token_free(&l->token);
    l->token = (Token){0};

    lx_trim_spaces(l);
    if (l->cur >= l->src_len) {
        l->token = TOK(EOF, 1);
        goto done;
    }

    if (CUR == '\n') {
        l->token = TOK(NEWLINE, 1);
        BUMP_NEWLINE;
        goto done;
    }

    if (lx_next_double_symbol(l))
        goto done;

    if (lx_next_single_symbol(l))
        goto done;

    a_string word = {0};
    if (!lx_next_word(l, &word)) // error
        return NULL;

    if (lx_next_keyword(l, &word)) {
        as_free(&word);
        goto done;
    }

    if (lx_next_literal(l, &word)) {
        as_free(&word);
        goto done;
    }

    l->token = (Token){
        .kind = TOK_IDENT,
        .data.string = word,
        .pos = POS(word.len),
    };

done:
    return &l->token;
}

void lx_free(Lexer* l) {
    (void)l;
    lx_kwt_free();
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
