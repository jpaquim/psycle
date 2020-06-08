// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(COMMAND_H)
#define COMMAND_H

// aim: Interface that implements the Command Pattern used
//      for undo/redo operations

typedef struct {
	void (*dispose)(void*);
	void (*execute)(void*);	
	void (*revert)(void*);
} Command;

#endif
