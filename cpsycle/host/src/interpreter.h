// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(INTERPRETER_H)
#define INTERPRETER_H

#include "workspace.h"

#ifdef __cplusplus
extern "C" {
#endif

// aim: terminal host control

typedef struct {
	Workspace* workspace;
} Interpreter;

int interpreter_init(Interpreter*, Workspace*);
void interpreter_dispose(Interpreter*);
int interpreter_start(Interpreter*);
int interpreter_onidle(Interpreter*);

#ifdef __cplusplus
}
#endif

#endif
