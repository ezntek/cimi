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
    LX_ERROR_NULL = 0,
    LX_ERROR_UNTERMINATED_LITERAL,
    LX_ERROR_EOF,
    LX_BAD_ESCAPE,
} LexerError;

typedef enum {
    TOK_IDENT = 0,
    TOK_EOF,
    TOK_INVALID, // internal use only!!
    TOK_NEWLINE,

    // Literals
    TOK_LITERAL_STRING,
    TOK_LITERAL_CHAR,
    TOK_LITERAL_NUMBER,
    TOK_LITERAL_BOOLEAN,

    // Keywords
    TOK_VAR,
    TOK_CONST,
    TOK_ECHO,
    TOK_READ,
    TOK_AND,
    TOK_OR,
    TOK_NOT,
    TOK_IF,
    TOK_THEN,
    TOK_ELSE,
    TOK_END,
    TOK_SWITCH,
    TOK_CASE,
    TOK_DEFAULT,
    TOK_WHILE,
    TOK_FOR,
    TOK_FN,
    TOK_RETURN,
    TOK_INCLUDE,
    TOK_EXPORT,
    TOK_BREAK,
    TOK_CONTINUE,

    // Types
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
    TOK_SEMICOLON,

    // Operators
    TOK_ADD,
    TOK_SUB,
    TOK_MUL,
    TOK_DIV,
    TOK_PERCENT,
    TOK_CARET,
    TOK_TILDE,
    TOK_LT,
    TOK_GT,
    TOK_LEQ,
    TOK_GEQ,
    TOK_EQ,
    TOK_NEQ,
    TOK_ASSIGN,
    TOK_SHR,
    TOK_SHL,

    TOK_ADD_ASSIGN,
    TOK_SUB_ASSIGN,
    TOK_MUL_ASSIGN,
    TOK_DIV_ASSIGN,
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
        a_string string; // idents, other literals
        bool boolean;    // bool literals
        u8 null;         // dummy field
    } data;
} Token;

Token token_new_ident(const char* str);
a_string token_kind_to_string(TokenKind k);
void token_print_long(Token* t);
void token_print(Token* t);

// free heap allocated data.
// of course, you can still read the kind if you have to.
void token_free(Token* t);

typedef struct {
    const char* src;
    usize src_len;

    // current token (public)
    Token token;

    // error (public)
    LexerError error;

    // internal lexer state
    u32 cur;
    u32 row;
    u32 bol;
} Lexer;

Lexer lx_new(const char* src, usize src_len);
Token* lx_next_token(Lexer* l);
void lx_free(Lexer* l);

char* lx_strerror(LexerError e);
a_string lx_as_strerror(LexerError e);
void lx_perror(LexerError e, const char* pre);
void lx_reset(Lexer* l);

#endif // _LEXER_H
