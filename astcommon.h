/*
 * cimi: a scuffed scripting language
 *
 * Copyright (c) Eason Qin <eason@ezntek.com>, 2025.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _ASTCOMMON_H
#define _ASTCOMMON_H

#define AST_DECL_FREE(T)    void T##_free(T*);
#define AST_IMPL_FREE(T, n) void T##_free(T* n)

#define AST_INIT_NODE(T) T res = {.pos = pos}

#endif // _ASTCOMMON_H
