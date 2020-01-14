// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(PSY_AUDIO_PSY2CONVERTER_H)
#define PSY_AUDIO_PSY2CONVERTER_H


struct psy_audio_Machine* psy_audio_psy2converter_load(
	struct psy_audio_SongFile*,
	int index, int* newindex, int* x, int* y);

#endif
