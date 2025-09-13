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
#define _GNU_SOURCE

#include "a_string.h"
#include "common.h"

i32 main(i32 argc, char* argv[argc]) {
    a_string s = astr("hello, world");
    as_println(&s);
    as_free(&s);
    return 0;
}
