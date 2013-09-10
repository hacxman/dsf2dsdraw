// Copyright 2010 fail0verflow <master@fail0verflow.com>
// Licensed under the terms of the GNU GPL, version 2
// http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

#ifndef ELF_H__
#define ELF_H__

#include "emulate.h"
#include "types.h"

void elf_load(spe_ctx_t *ctx, const char *path);

#endif
