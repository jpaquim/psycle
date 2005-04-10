/*
	GameFX (C)2005 by Jan-Marco Edelmann [voskomo], voskomo_at_yahoo_dot_de
	Programm is based on Arguru Bass. Filter seems to be Public Domain.

	This plugin is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.\n"\

	This plugin is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#pragma once
#include "filter.h"

struct PERFORMANCE
{
	int Volume[16];
	int Waveform[16];
	int Transpose[16];
	int Option[16];
	int Command[16];
	int Parameter[16];
	int Speed[16];
	int StartPos;
	int LoopStart;
	int LoopEnd;
	int ReplaySpeed;
	int AEGAttack;
	int AEGDecay;
	int AEGSustain;
	int AEGRelease;
	int Cutoff;
	int Resonance;
	int EnvMod;
	int FEGAttack;
	int FEGDecay;
	int FEGSustain;
	int FEGRelease;
	int Finetune;
	signed short Wavetable[8][4096];
	signed short shortnoise;
	long noise;
	long reg;
	long bit22;
	long bit17;
	int  noiseindex;
	bool noiseused;
};

class CSynthTrack  
{
public:
	void InitEffect(int cmd,int val);
	void PerformFx();
	void DoGlide();
	float Filter(float x);
	void NoteOff();
	float GetEnvAmp();
	void GetEnvVcf();
	float oscglide;
	float GetSample();
	void NoteOn(int note, PERFORMANCE *perf, int spd);
	
	CSynthTrack();
	virtual ~CSynthTrack();
	int AmpEnvStage;
	
	filter m_filter;

private:
	short timetocompute;
	void InitEnvelopes();

	float VcfResonance;
	int sp_cmd;
	int sp_val;

	float OSCPosition;
	float OSCSpeed;
	float ROSCSpeed;

	// Envelope [Amplitude]
	float AmpEnvValue;
	float AmpEnvCoef;
	float AmpEnvSustainLevel;
	float OSCVol;

	// Envelope [Amplitude]
	float VcfEnvValue;
	float VcfEnvCoef;
	float VcfEnvSustainLevel;
	int VcfEnvStage;
	float VcfEnvMod;
	float VcfCutoff;
	
	PERFORMANCE *vpar;
	int replaycount;
	int	perf_count;
	int perf_index;

	int cur_basenote;
	int cur_realnote;
	int	cur_volume;
	float voicevol;
	float volmulti;
	int	cur_waveform;
	int	cur_transpose;
	float add_to_pitch;
	int	cur_option;
	int	cur_command;
	int	cur_parameter;
	int	cur_speed;
	int cur_pw;
	bool keyrelease;
	float speed;
	float spdcoef;

	inline int f2i(double d)
	{
		const double magic = 6755399441055744.0; // 2^51 + 2^52
		double tmp = (d-0.5) + magic;
		return *(int*) &tmp;
	}
};
