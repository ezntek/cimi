/*
 * cimi: a scuffed scripting language
 *
 * Copyright (c) Eason Qin <eason@ezntek.com>, 2025.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _AST_PRINTER_H
#define _AST_PRINTER_H

#include "ast.h"
#include "expr.h"

struct AstPrinter;

// boilerplate

typedef void (*AstPrinterWriter)(struct AstPrinter*, const char*);
typedef void (*AstPrinterWritefer)(struct AstPrinter*, const char*, ...);

typedef struct AstPrinter {
    AstPrinterWriter write;
    AstPrinterWritefer writef;
    FILE* fp;     // null if file writer is not used
    a_string buf; // for the string writer
    u32 indent;
} AstPrinter;

void ap_write_stdout(AstPrinter* p, const char* data);
void ap_write_stderr(AstPrinter* p, const char* data);
void ap_write_file(AstPrinter* p, const char* data);
void ap_write_string(AstPrinter* p, const char* data);

AstPrinter ap_new(void);
AstPrinter ap_new_stdout_writer(void);
AstPrinter ap_new_stderr_writer(void);
AstPrinter ap_new_file_writer(FILE* fp);
AstPrinter ap_new_string_writer(void);

// actual visit functions

void ap_visit_primitive_type(AstPrinter* p, C_PrimitiveType t);
void ap_visit_array_type(AstPrinter* p, C_ArrayType* t);
void ap_visit_type(AstPrinter* p, C_Type* t);
void ap_visit_identifier(AstPrinter* p, C_Identifier* id);
void ap_visit_lvalue(AstPrinter* p, C_Lvalue* lv);

void ap_visit_unary(AstPrinter* p, C_UnaryExpr* n);
void ap_visit_binary(AstPrinter* p, C_BinaryExpr* n);
void ap_visit_array_index(AstPrinter* p, C_ArrayIndex* n);

void ap_visit_fn_argument(AstPrinter* p, C_FunctionArgument* arg);
void ap_visit_argument_list(AstPrinter* p, C_ArgumentList* lst);
void ap_visit_fn_call(AstPrinter* p, C_FnCall* n);

void ap_visit_assign(AstPrinter* p, C_Assign* n);
void ap_visit_if(AstPrinter* p, C_If* n);

void ap_visit_literal(AstPrinter* p, C_Literal* l);

void ap_visit_expr(AstPrinter* p, C_Expr* n);

#endif // _AST_PRINTER_H
