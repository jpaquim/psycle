// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "noteperiods.h"

#include <math.h>

/// Size of the Instrument's note mapping.
#define NOTE_MAP_SIZE 120 // C-0 .. B-9
#define MIDDLEC 60

// calculated table from the following formula:
// period =  pow(2.0,5.0-double(note)/12.0) * 1712;
// being 5 = the middle octave (on PC), 1712 the middle C period on PC
// (on Amiga it was 428, which was multiplied by 4 on PC, to add fine pitch slide).
const float AmigaPeriod[NOTE_MAP_SIZE] = {
	54784.00f,	51709.21f,	48806.99f,	46067.67f,	43482.09f,	41041.62f,	38738.13f,	36563.93f,	34511.76f,	32574.76f,	30746.48f,	29020.81f, //Oct 0
	27392.00f,	25854.61f,	24403.50f,	23033.83f,	21741.04f,	20520.81f,	19369.07f,	18281.97f,	17255.88f,	16287.38f,	15373.24f,	14510.41f, //Oct 1
	13696.00f,	12927.30f,	12201.75f,	11516.92f,	10870.52f,	10260.41f,	9684.53f,	9140.98f,	8627.94f,	8143.69f,	7686.62f,	7255.20f, //Oct 2
	6848.00f,	6463.65f,	6100.87f,	5758.46f,	5435.26f,	5130.20f,	4842.27f,	4570.49f,	4313.97f,	4071.85f,	3843.31f,	3627.60f, //Oct 3
	3424.00f,	3231.83f,	3050.44f,	2879.23f,	2717.63f,	2565.10f,	2421.13f,	2285.25f,	2156.98f,	2035.92f,	1921.66f,	1813.80f, //Oct 4
	1712.00f,	1615.91f,	1525.22f,	1439.61f,	1358.82f,	1282.55f,	1210.57f,	1142.62f,	1078.49f,	1017.96f,	960.828f,	906.900f, //Oct 5 (middle)
	856.000f,	807.956f,	762.609f,	719.807f,	679.408f,	641.275f,	605.283f,	571.311f,	539.246f,	508.981f,	480.414f,	453.450f, //Oct 6
	428.000f,	403.978f,	381.305f,	359.904f,	339.704f,	320.638f,	302.642f,	285.656f,	269.623f,	254.490f,	240.207f,	226.725f, //Oct 7
	214.000f,	201.989f,	190.652f,	179.952f,	169.852f,	160.319f,	151.321f,	142.828f,	134.812f,	127.245f,	120.103f,	113.363f, //Oct 8
	107.000f,	100.995f,	95.3262f,	89.9759f,	84.9259f,	80.1594f,	75.6604f,	71.4139f,	67.4058f,	63.6226f,	60.0517f,	56.6813f //Oct 9
};

double psy_dsp_notetoamigaperiod(int note, double wavsamplerate, int tune, int finetune)
{
	// Amiga Period.
	double c5speed = (double)wavsamplerate * pow(2.0, (tune + (finetune * 0.01)) / 12.0);
	return AmigaPeriod[note] * 8363.0 / c5speed;
}

double psy_dsp_notetoperiod(int note, int tune, int finetune)
{	
	// 9216 = 12octaves*12notes*64fine.
	return 9216 - ((double)((double)note + (double)tune) * 64.0)
		- ((double)(finetune) * 0.64); // 0.64 since the range is +-100 for XMSampler as opposed to +-128 for FT.
}

int psy_dsp_amigaperiodtonote(double period, double wavsamplerate, int tune, int finetune)
{
	// period_t (table) = pow(2.0,5.0-note/12.0) * 1712.0;
	// final_period = period_t*8363.0/ ( _wave.WaveSampleRate()*pow(2.0,(_wave.WaveTune()+(_wave.WaveFineTune()*0.01))/12.0)  )
	// final_period * _wave.WaveSampleRate()*pow(2.0,(_wave.WaveTune()+(_wave.WaveFineTune()*0.01))/12.0) = pow(2.0,5.0-note/12.0) * 1712.0 *8363.0
	// final_period * _wave.WaveSampleRate()/(1712.0 *8363.0)*pow(2.0,(_wave.WaveTune()+(_wave.WaveFineTune()*0.01))/12.0) = pow(2.0,5.0-note/12.0)
	// log2(final_period * _wave.WaveSampleRate()/(1712.0 *8363.0)) + log2(pow(2.0,(_wave.WaveTune()+(_wave.WaveFineTune()*0.01))/12.0)) = 5.0-note/12.0
	// note = 60 - 12*log2(final_period * _wave.WaveSampleRate()/(1712.0 *8363.0)) + (_wave.WaveTune()+(_wave.WaveFineTune()*0.01))/12.0
	double _note = MIDDLEC - 12 * (log10(period * wavsamplerate) * 3.3219280948873623478703194294894 /*1/log10(2)*/
		- 23.77127183403184445503933415201/*log2(1.0/(1712.0 *8363.0))*/) + (tune + (finetune * 0.01)) / 12.0;
	return (int)_note + 12;
}

int psy_dsp_periodtonote(double period, int tune, int finetune)
{	
		// period = ((12.0 * 12.0 * 64.0 - ((double)note + (double)_wave.WaveTune()) * 64.0)
		//	- (_wave.WaveFineTune() *0.01) * 64.0);
		// period / 64.0 = 12.0 * 12.0  - ((double)note + (double)_wave.WaveTune()) - _wave.WaveFineTune()*0.01;
		// note = (int)(12.0 * 12.0  - (double)_wave.WaveTune() - _wave.WaveFineTune() *0.01 - period / 64.0 + 0.5);

		return (int)(144 - (double)tune - ((double)finetune * 0.01) - (period * 0.015625)); // Apparently,  (int)(x.5) rounds to x+1, so no need for +0.5	
}

double psy_dsp_amigaperiodtospeed(int period, double samplerate, int modamount)
{
	// Amiga period mode. Original conversion:
	//	PAL:   7093789.2 / (428*2) = 8287.14 Hz   ( Amiga clock frequency, middle C period and final samplerate)
	//	NSTC:  7159090.5 / (428*2) = 8363.42 Hz   ( *2 is used to convert clock frequency to ticks).
	// in PC, the middle C period is 1712. It was increased by 4 to add extra fine pitch slides.
	// so 1712 *8363 = 14317456, which is used in IT and FT2 (and in ST3, if the value that exists in their doc is a typo).
	// One could also use 7159090.5 /2 *4 = 14318181
	return (14317456.0 / period) * pow(2.0, (modamount * 16.0) / 12.0) / samplerate;
}

double psy_dsp_periodtospeed(int period, double samplerate, double wavsamplerate, int modamount)
{	
	// Linear Frequency
	// base_samplerate * 2^((7*12*64 - Period) / (12*64))
	// 12*64 = 12 notes * 64 finetune steps.
	// 7 = 12 - middle_C ( if C-4 is middle_C, then 8*12*64, if C-3, then 9*12*64, etc..)
	return	pow(2.0,
		((5376 - period + modamount * 1024.0)
			/ 768.0)
	) * wavsamplerate / samplerate;
}
