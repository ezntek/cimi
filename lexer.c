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
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "3rdparty/uthash.h"
#include "a_string.h"
#include "common.h"
#include "lexer.h"

#define CUR  (l->src[l->cur])
#define PEEK (l->src[l->cur + 1])
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

#define ERROR(k, sp)                                                           \
    (LexerError) {                                                             \
        .kind = LX_ERROR_##k, .pos = (POS(sp))                                 \
    }

#define TRY(expr)                                                              \
    do {                                                                       \
        if ((expr)) {                                                          \
            goto done;                                                         \
        } else if (l->error.kind != LX_ERROR_NULL) {                           \
            return NULL;                                                       \
        }                                                                      \
    } while (0)

#define TRY_AND_FREE(expr)                                                     \
    do {                                                                       \
        if ((expr)) {                                                          \
            goto free_and_done;                                                \
        } else if (l->error.kind != LX_ERROR_NULL) {                           \
            as_free(&word);                                                    \
            return NULL;                                                       \
        }                                                                      \
    } while (0)

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
            s = "!!! BOGUS AMOGUS TOKEN !!";
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
        case TOK_LET: {
            s = "let";
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

    lx_kwt_add("let", TOK_LET);
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
    const char DELIMS[] = "\"'";
    bool delimited_literal = strchr(DELIMS, *begin);
    char delim = 0;
    if (delimited_literal) {
        delim = CUR;
        l->cur++;
        len++;
    }

    do {
        bool stop;
        if (delimited_literal)
            stop = strchr("\n\r", CUR) || strchr(DELIMS, CUR);
        else
            stop = lx_is_operator_start(CUR) || lx_is_separator(CUR) ||
                   isspace(CUR);

        if (CUR == '\\') {
            len++;
            l->cur++;
        }

        if (stop || !IN_BOUNDS)
            break;

        len++;
        l->cur++;
    } while (1);

    if (delimited_literal) {
        len++;
        if (CUR != delim) {
            if (strchr(DELIMS, CUR) != NULL)
                l->error = ERROR(MISMATCHED_DELIMITER, len);
            else
                l->error = ERROR(UNTERMINATED_LITERAL, len);
            return false;
        } else {
            l->cur++;
        }
    } else if (!IN_BOUNDS) {
        l->error = ERROR(EOF, 1);
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

static bool lx__is_number(const a_string* word) {
    bool found_decimal = false;
    for (usize i = 0; i < word->len; i++) {
        char cur = as_at(word, i);

        if (isdigit(cur))
            continue;

        if (cur == '_') {
            if (i == 0)
                return false;
            else
                continue;
        }

        if (cur == '.') {
            if (found_decimal)
                return false;
            else
                found_decimal = true;

            continue;
        }

        return false;
    }

    return true;
}

static char lx__resolve_escape(char ch) {
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
        default: return ch;
    }
}

static bool lx_next_literal(Lexer* l, const a_string* word) {
    char* p;
    if ((p = strchr("\"'", as_first(word)))) {
        if (word->len == 1)
            unreachable;

        a_string res = as_new();
        char ch;
        for (usize i = 1; i < word->len - 1; ++i) {
            ch = as_at(word, i);
            if (ch == '\\') {
                // last character is an escape
                if (i + 1 == word->len - 1) {
                    l->error = ERROR(BAD_ESCAPE, word->len);
                    return false;
                }
                ch = lx__resolve_escape(as_at(word, ++i));
            }

            as_append_char(&res, ch);
        }

        TokenKind k = (*p == '\'') ? TOK_LITERAL_CHAR : TOK_LITERAL_STRING;

        if (k == TOK_LITERAL_CHAR && res.len > 1) {
            l->error = ERROR(CHAR_LITERAL_TOO_LONG, word->len);
            return false;
        }

        l->token = (Token){
            .kind = k,
            .pos = POS(word->len),
            .data.string = res,
        };
        return true;
    }

    if (lx__is_number(word)) {
        a_string new = as_dupe(word);
        if (!as_valid(&new))
            panic("failed to dupe a_string for number");

        l->token = (Token){
            .kind = TOK_LITERAL_NUMBER,
            .pos = POS(word->len),
            .data.string = new,
        };
        return true;
    }

    if (as_equal_cstr(word, "true")) {
        l->token = (Token){
            .kind = TOK_LITERAL_BOOLEAN,
            .pos = POS(word->len),
            .data.boolean = true,
        };
        return true;
    }

    if (as_equal_cstr(word, "false")) {
        l->token = (Token){
            .kind = TOK_LITERAL_BOOLEAN,
            .pos = POS(word->len),
            .data.boolean = false,
        };
        return true;
    }

    return false;
}

static bool lx__is_ident(const a_string* s) {
    if (!isalpha(as_first(s)))
        return false;

    for (size_t i = 0; i < s->len; i++) {
        char ch = as_at(s, i);
        if (!isalnum(ch) && !strchr("_.", ch))
            return false;
    }

    return true;
}

static bool lx_next_ident(Lexer* l, const a_string* word) {
    if (lx__is_ident(word)) {
        a_string new = as_dupe(word);
        if (!as_valid(&new))
            panic("failed to dupe a_string for ident");

        l->token = (Token){
            .kind = TOK_IDENT,
            .data.string = new,
            .pos = POS(new.len),
        };
        return true;
    } else {
        l->error = ERROR(INVALID_IDENTIFIER, word->len);
        return false;
    }
}

Token* lx_next_token(Lexer* l) {
    if (l->error.kind != LX_ERROR_NULL) {
        token_free(&l->token);
    }

    l->token = (Token){0};
    l->error = (LexerError){0};

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

    TRY(lx_next_double_symbol(l));
    TRY(lx_next_double_symbol(l));
    TRY(lx_next_single_symbol(l));

    a_string word = {0};
    if (!lx_next_word(l, &word)) // error
        return NULL;

    TRY_AND_FREE(lx_next_keyword(l, &word));
    TRY_AND_FREE(lx_next_literal(l, &word));
    TRY_AND_FREE(lx_next_ident(l, &word));

free_and_done:
    as_free(&word);
done:
    return &l->token;
}

void lx_free(Lexer* l) {
    (void)l;
    lx_kwt_free();
}

char* lx_strerror(LexerErrorKind k) {
    char* s;
    switch (k) {
        case LX_ERROR_NULL: {
            s = "(no error)";
        } break;
        case LX_ERROR_UNTERMINATED_LITERAL: {
            s = "unterminated string or character literal";
        } break;
        case LX_ERROR_EOF: {
            s = "unexpected end of file";
        } break;
        case LX_ERROR_BAD_ESCAPE: {
            s = "bad escape sequence";
        } break;
        case LX_ERROR_MISMATCHED_DELIMITER: {
            s = "mismatched delimiter in delimited literal";
        } break;
        case LX_ERROR_INVALID_IDENTIFIER: {
            s = "invalid identifier";
        } break;
        case LX_ERROR_CHAR_LITERAL_TOO_LONG: {
            s = "character literal is too long";
        } break;
    }

    return strncpy(ERROR_BUF, s, ERROR_BUFSZ);
}

a_string lx_as_strerror(LexerErrorKind k) {
    return astr(lx_strerror(k));
}

void lx_perror(LexerErrorKind k, const char* pre) {
    char* err = lx_strerror(k);
    printf("%s: %s\n", pre, err);
}

void lx_reset(Lexer* l) {
    l->row = 1;
    l->bol = 0;
    l->cur = 0;
}
