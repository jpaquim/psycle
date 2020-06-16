// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_dsp_NOTEPERIODS_H
#define psy_dsp_NOTEPERIODS_H

#include "../../detail/psydef.h"

#ifdef __cplusplus
extern "C" {
#endif

double psy_dsp_notetoamigaperiod(int note, double wavsamplerate, int tune, int finetune);
double psy_dsp_notetoperiod(int note, int tune, int finetune);
int psy_dsp_amigaperiodtonote(double period, double wavsamplerate, int tune, int finetune);	
int psy_dsp_periodtonote(double period, int tune, int finetune);
double psy_dsp_amigaperiodtospeed(int period, double samplerate, int modamount);
double psy_dsp_periodtospeed(int period, double samplerate, double wavsamplerate, int modamount);

#ifdef __cplusplus
}
#endif

#endif /* psy_dsp_NOTEPERIODS_H */
