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
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h> // used in macro
#define _POSIX_C_SOURCE 200809L

#include "a_string.h"
#include "ast_printer.h"
#include "common.h"
#include "expr.h"

void ap_write_stdout(AstPrinter* p, const char* data) {
    printf("%s", data);
}

void ap_write_stderr(AstPrinter* p, const char* data) {
    eprintf("%s", data);
}

void ap_write_file(AstPrinter* p, const char* data) {
    fprintf(p->fp, "%s", data);
}

void ap_write_string(AstPrinter* p, const char* data) {
    as_append_cstr(&p->buf, data);
}

#define AP_WRITEF_MAXSIZE 512
static void ap_writefer(AstPrinter* p, const char* data, ...) {
    char buf[AP_WRITEF_MAXSIZE] = {0};
    va_list lst;
    va_start(lst, data);
    vsprintf(buf, data, lst);
    va_end(lst);
    p->write(p, buf);
}

AstPrinter ap_new(void) {
    return ap_new_stdout_writer();
}

AstPrinter ap_new_stdout_writer(void) {
    return (AstPrinter){
        .write = ap_write_stdout,
        .writef = ap_writefer,
    };
}

AstPrinter ap_new_stderr_writer(void) {
    return (AstPrinter){
        .write = ap_write_stderr,
        .writef = ap_writefer,
    };
}

AstPrinter ap_new_file_writer(FILE* fp) {
    return (AstPrinter){
        .write = ap_write_file,
        .writef = ap_writefer,
        .fp = fp,
    };
}

AstPrinter ap_new_string_writer(void) {
    return (AstPrinter){
        .write = ap_write_string,
        .writef = ap_writefer,
        .buf = as_with_capacity(128),
    };
}

void ap_visit_primitive_type(AstPrinter* p, C_PrimitiveType t) {
    switch (t) {
        case C_INT: {
            p->write(p, "int");
        } break;
        case C_FLOAT: {
            p->write(p, "float");
        } break;
        case C_CHAR: {
            p->write(p, "char");
        } break;
        case C_STRING: {
            p->write(p, "string");
        } break;
        case C_BOOL: {
            p->write(p, "boolean");
        } break;
        case C_ANY: {
            p->write(p, "any");
        } break;
        case C_NULL: {
            p->write(p, "null");
        } break;
    }
}

void ap_visit_array_type(AstPrinter* p, C_ArrayType* t) {
    p->write(p, "array_type(");
    ap_visit_expr(p, t->size);
    p->write(p, ", ");
    ap_visit_type(p, t->inner);
    p->write(p, ")");
}

void ap_visit_type(AstPrinter* p, C_Type* t) {
    switch (t->kind) {
        case C_TYPE_PRIMITIVE: {
            ap_visit_primitive_type(p, t->data.primitive);
        } break;
        case C_TYPE_ARRAY: {
            ap_visit_array_type(p, t->data.array);
        } break;
    }
}

void ap_visit_identifier(AstPrinter* p, C_Identifier* id) {
    p->write(p, "\"");
    p->write(p, id->ident.data);
    p->write(p, "\"");
}

void ap_visit_lvalue(AstPrinter* p, C_Lvalue* lv) {
    switch (lv->kind) {
        case C_LV_IDENTIFIER: {
            ap_visit_identifier(p, lv->data.ident);
        } break;
        case C_LV_ARRAY_INDEX: {
            ap_visit_array_index(p, lv->data.array_index);
        } break;
    }
}

void ap_visit_unary(AstPrinter* p, C_UnaryExpr* n) {
    switch (n->op) {
        case C_UNARYOP_GROUPING: {
            p->write(p, "grouping");
        } break;
        case C_UNARYOP_NEGATION: {
            p->write(p, "negation");
        } break;
        case C_UNARYOP_NOT: {
            p->write(p, "not");
        } break;
    }
    p->write(p, "(");
    ap_visit_expr(p, n->inner);
    p->write(p, ")");
}

void ap_visit_binary(AstPrinter* p, C_BinaryExpr* n) {
    switch (n->op) {
        case C_BINARYOP_ADD: {
            p->write(p, "add");
        } break;
        case C_BINARYOP_SUB: {
            p->write(p, "sub");
        } break;
        case C_BINARYOP_MUL: {
            p->write(p, "mul");
        } break;
        case C_BINARYOP_DIV: {
            p->write(p, "div");
        } break;
        case C_BINARYOP_POW: {
            p->write(p, "pow");
        } break;
        case C_BINARYOP_NEQ: {
            p->write(p, "neq");
        } break;
        case C_BINARYOP_GEQ: {
            p->write(p, "geq");
        } break;
        case C_BINARYOP_LEQ: {
            p->write(p, "leq");
        } break;
        case C_BINARYOP_LT: {
            p->write(p, "lt");
        } break;
        case C_BINARYOP_GT: {
            p->write(p, "gt");
        } break;
        case C_BINARYOP_EQ: {
            p->write(p, "eq");
        } break;
    }
    p->write(p, "(");
    ap_visit_expr(p, n->lhs);
    p->write(p, ", ");
    ap_visit_expr(p, n->rhs);
    p->write(p, ")");
}

void ap_visit_array_index(AstPrinter* p, C_ArrayIndex* n) {
    p->write(p, "array_index(");
    ap_visit_expr(p, n->ident);
    p->write(p, ", ");
    ap_visit_expr(p, n->index);
    p->write(p, ")");
}

void ap_visit_fn_argument(AstPrinter* p, C_FunctionArgument* arg) {
    p->write(p, "(");
    ap_visit_identifier(p, arg->ident);
    p->write(p, ": ");
    ap_visit_type(p, arg->type);
    p->write(p, ")");
}

void ap_visit_argument_list(AstPrinter* p, C_ArgumentList* lst) {
    p->write(p, "args(");
    for (u32 i = 0; i < lst->args_len; ++i) {
        ap_visit_fn_argument(p, &lst->args[i]);
        if (i != lst->args_len - 1)
            p->write(p, ", ");
    }
    p->write(p, ")");
}

void ap_visit_fn_call(AstPrinter* p, C_FnCall* n) {
    p->write(p, "fn_call(");
    ap_visit_identifier(p, n->ident);
    p->write(p, ", ");
    ap_visit_argument_list(p, n->args);
    p->write(p, ")");
}

void ap_visit_assign(AstPrinter* p, C_Assign* n) {
    p->write(p, "lvalue(");
    ap_visit_lvalue(p, n->lhs);
    p->write(p, ", ");
    ap_visit_expr(p, n->rhs);
    p->write(p, ")");
}

void ap_visit_literal(AstPrinter* p, C_Literal* l) {
    switch (l->type) {
        case C_INT: {
            p->writef(p, "int(%ld)", l->data._int);
        } break;
        case C_FLOAT: {
            p->writef(p, "float(%lf)", l->data._float);
        } break;
        case C_STRING: {
            if (l->data.string.len > 64) {
                p->write(p, "<a very long string>");
            } else {
                // TODO: escape sequences
                p->writef(p, "\"%.*s\"", (int)l->data.string.len,
                          l->data.string.data);
            }
        } break;
        case C_CHAR: {
            char ch = l->data._char;
            if (isprint(ch)) {
                p->writef(p, "'%s'", ch);
            } else {
                p->writef(p, "char(%x)", (int)ch);
            }
        } break;
        case C_BOOL: {
            if (l->data._bool) {
                p->write(p, "true");
            } else {
                p->write(p, "false");
            }
        } break;
        case C_NULL: {
            p->write(p, "<null>");
        } break;
        case C_ANY: {
            panic("bogus amogus (cannot have an any literal!)");
        } break;
    }
}

void ap_visit_if(AstPrinter* p, C_If* n) {
    eprintf("not implemented\n");
    unreachable;
}

void ap_visit_expr(AstPrinter* p, C_Expr* n) {
    switch (n->kind) {
        case C_EXPR_IDENTIFIER: {
            ap_visit_identifier(p, n->data.ident);
        } break;
        case C_EXPR_UNARYOP: {
            ap_visit_unary(p, &n->data.unary);
        } break;
        case C_EXPR_BINOP: {
            ap_visit_binary(p, &n->data.binary);
        } break;
        case C_EXPR_ARRAY_INDEX: {
            ap_visit_array_index(p, &n->data.array_index);
        } break;
        case C_EXPR_FNCALL: {
            ap_visit_fn_call(p, &n->data.fn_call);
        } break;
        case C_EXPR_ASSIGN: {
            ap_visit_assign(p, &n->data.assign);
        } break;
        case C_EXPR_IF: {
            ap_visit_if(p, &n->data._if);
        } break;
        case C_EXPR_LITERAL: {
            ap_visit_literal(p, &n->data.literal);
        } break;
    }
}
