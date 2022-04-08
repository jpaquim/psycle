// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"
#include "../../detail/os.h"

#include "interpreter.h"

int interpreter_init(Interpreter* self, Workspace* workspace_)
{
    self->workspace = workspace_;
    return 0;
}

int interpreter_start(Interpreter* self)
{
    workspace_output(self->workspace, PSYCLE__TITLE);
    workspace_output(self->workspace, "\n");
    workspace_output(self->workspace, PSYCLE__COPYRIGHT);
    workspace_output(self->workspace, "\n");
    workspace_output(self->workspace, "ready.\n");
#if !defined(PSYCLE_USE_MACHINEPROXY)
    workspace_output_warning(self->workspace, "NO MACHINEPROXY PROTECTION\n");
#endif
    return 0;
}

void interpreter_dispose(Interpreter* self)
{    
}

int interpreter_onidle(Interpreter* self)
{
    return 0;
}
