// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_PSY2CONVERTER_H
#define psy_audio_PSY2CONVERTER_H

#ifdef __cplusplus
extern "C" {
#endif

struct psy_audio_SongFile;

struct psy_audio_Machine* psy_audio_psy2converter_load(
	struct psy_audio_SongFile*,
	int index, int* newindex, int* x, int* y);

void psy_audio_psy2converter_retweak(int type, int parameter, int* integral_value);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_PSY2CONVERTER_H */
