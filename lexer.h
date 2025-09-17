/*
 * cimi: a scuffed scripting language
 *
 * Copyright (c) Eason Qin <eason@ezntek.com>, 2025.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _LEXER_H
#define _LEXER_H

#include "a_string.h"
#include "common.h"

typedef enum {
    // Keywords
    TOK_DECLARE,
    TOK_CONSTANT,
    TOK_OUTPUT,
    TOK_INPUT,
    TOK_AND,
    TOK_OR,
    TOK_NOT,
    TOK_IF,
    TOK_THEN,
    TOK_ELSE,
    TOK_ENDIF,
    TOK_CASE,
    TOK_OF,
    TOK_OTHERWISE,
    TOK_ENDCASE,
    TOK_WHILE,
    TOK_DO,
    TOK_ENDWHILE,
    TOK_REPEAT,
    TOK_UNTIL,
    TOK_FOR,
    TOK_TO,
    TOK_STEP,
    TOK_NEXT,
    TOK_PROCEDURE,
    TOK_ENDPROCEDURE,
    TOK_CALL,
    TOK_FUNCTION,
    TOK_RETURN,
    TOK_RETURNS,
    TOK_ENDFUNCTION,
    TOK_OPENFILE,
    TOK_READFILE,
    TOK_WRITEFILE,
    TOK_CLOSEFILE,
    TOK_MOD,
    TOK_INCLUDE,
    TOK_INCLUDE_FFI,
    TOK_EXPORT,
    TOK_SCOPE,
    TOK_ENDSCOPE,
    TOK_PRINT,

    // TYPES
    TOK_INT,
    TOK_FLOAT,
    TOK_BOOL,
    TOK_STRING,
    TOK_CHAR,
    TOK_NULL,

    // Separators
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_LBRACKET,
    TOK_RBRACKET,
    TOK_LCURLY,
    TOK_RCURLY,
    TOK_COMMA,
    TOK_COLON,

    // Operators
    TOK_ADD,
    TOK_SUB,
    TOK_MUL,
    TOK_DIV,
    TOK_PERCENT,
    TOK_CARET,
    TOK_LT,
    TOK_GT,
    TOK_LEQ,
    TOK_REQ,
    TOK_EQ,
    TOK_NEQ,
    TOK_ASSIGN,

    TOK_IDENT,
    TOK_EOF,
} TokenKind;

typedef struct {
    u32 row;
    u16 col;
    u16 span;
} Pos;

typedef struct {
    TokenKind kind;
    Pos pos;
    union {
        a_string string;
        u8 null;
    } ident;
} Token;

#define TOKEN(k)                                                               \
    (Token) {                                                                  \
        .kind = TOK_##k, .ident = { NULL }                                     \
    }

Token token_new_ident(const char* str);

// free heap allocated data.
// of course, you can still read the kind if you have to.
void token_free(Token* t);

typedef struct {
    const char* src;
    usize src_len;

    // current token (public access)
    Token token;

    // internal lexer state
    u32 cur;
    u32 row;
    u32 bol;
} Lexer;

Lexer lx_new(const char* src, usize src_len);
Token* lx_next_token(Lexer* l);
void lx_reset(Lexer* l);

#endif // _LEXER_H
