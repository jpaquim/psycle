/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uibitmaps.h"
/* local */
#include "uibitmap.h"
/* file */
#include <dir.h>
/* platform */
#include "../../detail/portable.h"

void psy_ui_bitmaps_init(psy_ui_Bitmaps* self)
{
	assert(self);
	
	psy_table_init(&self->bmps);
	psy_table_init(&self->files);
	self->app_bmp_path = NULL;
}

void psy_ui_bitmaps_dispose(psy_ui_Bitmaps* self)
{	
	assert(self);
	
	psy_table_dispose(&self->bmps);
	psy_table_dispose_all(&self->files, NULL);
	free(self->app_bmp_path);
	self->app_bmp_path = NULL;
}

void psy_ui_bitmaps_set_app_bmp_path(psy_ui_Bitmaps* self,
	const char* app_bmp_path)
{
	assert(self);
	
	psy_strreset(&self->app_bmp_path, app_bmp_path);
}

void psy_ui_bitmaps_add(psy_ui_Bitmaps* self,
	uintptr_t id, const char* filename)
{
	char* old;
	
	assert(self);
	
	old = psy_table_at(&self->files, id);
	free(old);	
	psy_table_insert(&self->files, id, psy_strdup(filename));
}

int psy_ui_bitmaps_load(psy_ui_Bitmaps* self, uintptr_t id,
	psy_ui_Bitmap* rv)
{
	const char* filename;
	int status;
	
	assert(rv);
	
	filename = psy_table_at(&self->files, id);
	status = PSY_OK;
	if (filename) {
		psy_Path path;
		
		psy_path_init(&path, NULL);
		psy_path_set_prefix(&path, self->app_bmp_path);
		psy_path_set_name(&path, filename);		
		status = psy_ui_bitmap_load(rv, psy_path_full(&path));
		psy_path_dispose(&path);
	} else {		
		status = rv->imp->vtable->dev_loadresource(rv->imp, id);
	}
	return status;
}
