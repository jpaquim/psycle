/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "logger.h"
/* platform */
#include "../../detail/portable.h"


static void logger_output(psy_Logger* self, const char* str)
{
}

/* logger_vtable */
static psy_LoggerVTable logger_vtable;
static bool logger_vtable_initialized = FALSE;

static void logger_vtable_init(void)
{
	if (!logger_vtable_initialized) {		
		logger_vtable.output =
		logger_vtable.warn =
		logger_vtable.error =
			(fp_string_output)
			logger_output;		
		logger_vtable_initialized = TRUE;
	}
}

/* implementation */
void psy_logger_init(psy_Logger* self)
{	
	logger_vtable_init();
	self->vtable = &logger_vtable;
	self->context = NULL;
}
