// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_LOGICALCHANNELS_H
#define psy_audio_LOGICALCHANNELS_H

#include <hashtbl.h>

#ifdef __cplusplus
extern "C" {
#endif

// psy_audio_LogicalChannels
//
// Some Plugins can only handle 64 Channels
// psy_audio_LogicalChannels maps an unlimited index space to this limit. This
// is needed for Multisequence tracks, that add the current SequenceTrack Index
// multiplied with 64 to the patternevent channel nuumber 

typedef struct psy_audio_LogicalChannels
{
	psy_Table physicalmap;
	psy_Table logicalmap;
} psy_audio_LogicalChannels;

void psy_audio_logicalchannels_init(psy_audio_LogicalChannels* self);
void psy_audio_logicalchannels_dispose(psy_audio_LogicalChannels* self);

uintptr_t psy_audio_logicalchannels_physical(psy_audio_LogicalChannels*, uintptr_t logical);
void psy_audio_logicalchannels_reset(psy_audio_LogicalChannels*);
	
#ifdef __cplusplus
}
#endif

#endif /* psy_audio_LOGICALCHANNELS_H */
