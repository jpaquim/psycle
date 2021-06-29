/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "midisongexport.h"
/* local */
#include "constants.h"
#include "plugin_interface.h"
#include "psyconvert.h"
#include "song.h"
#include "songio.h"
#include "machinefactory.h"
#include "wire.h"
/* dsp */
#include <operations.h>
#include <valuemapper.h>
/* platform */
#include "../../detail/portable.h"

#if !defined DIVERSALIS__OS__MICROSOFT
#define _MAX_PATH 4096
#endif

static void writevarlen(PsyFile* fp, uint32_t value);

void psy_audio_midisongexport_init(psy_audio_MidiSongExport* self, psy_audio_SongFile* songfile)
{
	assert(self && songfile && songfile->song);

	self->songfile = songfile;
}

void psy_audio_midisongexport_dispose(psy_audio_MidiSongExport* self)
{

}

int psy_audio_midisongexport_save(psy_audio_MidiSongExport* self)
{	
	int status;

	status = PSY_OK;

	return status;
}

void writevarlen(PsyFile* fp, uint32_t value)
{
    uint32_t buffer;
    buffer = value & 0x7F;

    while ((value >>= 7))
    {
        buffer <<= 8;
        buffer |= ((value & 0x7F) | 0x80);
    }

    while (TRUE)
    {
        psyfile_write(fp, &buffer, 1);
        if (buffer & 0x80)
            buffer >>= 8;
        else
            break;
    }
}