/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_LOGGER_H
#define psy_LOGGER_H

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*fp_string_output)(void*, const char*);

typedef struct psy_LoggerVTable {
	fp_string_output output;
	fp_string_output warn;
	fp_string_output error;
} psy_LoggerVTable;

typedef struct psy_Logger {
	psy_LoggerVTable* vtable;
	void* context;
} psy_Logger;

void psy_logger_init(psy_Logger*);

#ifdef __cplusplus
}
#endif

#endif /* psy_LOGGER_H */
