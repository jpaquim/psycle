/* -*- mode:c++, indent-tabs-mode:t -*- */
/*
	GameFX (C)2005 by jme
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

#include <packageneric/pre-compiled.private.hpp>
#include <psycle/plugin_interface.hpp>
#include "voice.h"
#include <cstring>
#include <cstdlib>
#include <cassert>
#include <cmath>

#define MAX_ENV_TIME 65536


CMachineParameter const paraVol1 = 
{ 
	"Volume 1",
	"Volume 1",
	0,
	255,
	MPF_STATE,
	129
};

CMachineParameter const paraVol2 = 
{ 
	"Volume 2",
	"Volume 2",
	0,
	255,
	MPF_STATE,
	0
};

CMachineParameter const paraVol3 = 
{ 
	"Volume 3",
	"Volume 3",
	0,
	255,
	MPF_STATE,
	0
};

CMachineParameter const paraVol4 = 
{ 
	"Volume 4",
	"Volume 4",
	0,
	255,
	MPF_STATE,
	0
};

CMachineParameter const paraVol5 = 
{ 
	"Volume 5",
	"Volume 5",
	0,
	255,
	MPF_STATE,
	0
};

CMachineParameter const paraVol6 = 
{ 
	"Volume 6",
	"Volume 6",
	0,
	255,
	MPF_STATE,
	0
};

CMachineParameter const paraVol7 = 
{ 
	"Volume 7",
	"Volume 7",
	0,
	255,
	MPF_STATE,
	0
};

CMachineParameter const paraVol8 = 
{ 
	"Volume 8",
	"Volume 8",
	0,
	255,
	MPF_STATE,
	0
};

CMachineParameter const paraVol9 = 
{ 
	"Volume 9",
	"Volume 9",
	0,
	255,
	MPF_STATE,
	0
};

CMachineParameter const paraVol10 = 
{ 
	"Volume 10",
	"Volume 10",
	0,
	255,
	MPF_STATE,
	0
};

CMachineParameter const paraVol11 = 
{ 
	"Volume 11",
	"Volume 11",
	0,
	255,
	MPF_STATE,
	0
};

CMachineParameter const paraVol12 = 
{ 
	"Volume 12",
	"Volume 12",
	0,
	255,
	MPF_STATE,
	0
};

CMachineParameter const paraVol13 = 
{ 
	"Volume 13",
	"Volume 13",
	0,
	255,
	MPF_STATE,
	0
};

CMachineParameter const paraVol14 = 
{ 
	"Volume 14",
	"Volume 14",
	0,
	255,
	MPF_STATE,
	0
};

CMachineParameter const paraVol15 = 
{ 
	"Volume 15",
	"Volume 15",
	0,
	255,
	MPF_STATE,
	0
};

CMachineParameter const paraVol16 = 
{ 
	"Volume 16",
	"Volume 16",
	0,
	255,
	MPF_STATE,
	0
};



CMachineParameter const paraWave1 = 
{ 
	"Wave 1",
	"Wave 1",
	0,
	9,				
	MPF_STATE,
	1
};


CMachineParameter const paraWave2 = 
{ 
	"Wave 2",
	"Wave 2",
	0,
	9,				
	MPF_STATE,
	0
};

CMachineParameter const paraWave3 = 
{ 
	"Wave 3",
	"Wave 3",
	0,
	9,				
	MPF_STATE,
	0
};

CMachineParameter const paraWave4 = 
{ 
	"Wave 4",
	"Wave 4",
	0,
	9,				
	MPF_STATE,
	0
};

CMachineParameter const paraWave5 = 
{ 
	"Wave 5",
	"Wave 5",
	0,
	9,				
	MPF_STATE,
	0
};

CMachineParameter const paraWave6 = 
{ 
	"Wave 6",
	"Wave 6",
	0,
	9,				
	MPF_STATE,
	0
};

CMachineParameter const paraWave7 = 
{ 
	"Wave 7",
	"Wave 7",
	0,
	9,				
	MPF_STATE,
	0
};

CMachineParameter const paraWave8 = 
{ 
	"Wave 8",
	"Wave 8",
	0,
	9,				
	MPF_STATE,
	0
};

CMachineParameter const paraWave9 = 
{ 
	"Wave 9",
	"Wave 9",
	0,
	9,				
	MPF_STATE,
	0
};

CMachineParameter const paraWave10 = 
{ 
	"Wave 10",
	"Wave 10",
	0,
	9,				
	MPF_STATE,
	0
};

CMachineParameter const paraWave11 = 
{ 
	"Wave 11",
	"Wave 11",
	0,
	9,				
	MPF_STATE,
	0
};

CMachineParameter const paraWave12 = 
{ 
	"Wave 12",
	"Wave 12",
	0,
	9,				
	MPF_STATE,
	0
};

CMachineParameter const paraWave13 = 
{ 
	"Wave 13",
	"Wave 13",
	0,
	9,				
	MPF_STATE,
	0
};

CMachineParameter const paraWave14 = 
{ 
	"Wave 14",
	"Wave 14",
	0,
	9,				
	MPF_STATE,
	0
};

CMachineParameter const paraWave15 = 
{ 
	"Wave 15",
	"Wave 15",
	0,
	9,				
	MPF_STATE,
	0
};

CMachineParameter const paraWave16 = 
{ 
	"Wave 16",
	"Wave 16",
	0,
	9,				
	MPF_STATE,
	0
};

CMachineParameter const paraTranspose1 = 
{ 
	"Transpose 1",
	"Transpose 1",
	0,
	96,				
	MPF_STATE,
	1
};

CMachineParameter const paraTranspose2 = 
{ 
	"Transpose 2",
	"Transpose 2",
	0,
	96,				
	MPF_STATE,
	0
};

CMachineParameter const paraTranspose3 = 
{ 
	"Transpose 3",
	"Transpose 3",
	0,
	96,				
	MPF_STATE,
	0
};

CMachineParameter const paraTranspose4 = 
{ 
	"Transpose 4",
	"Transpose 4",
	0,
	96,				
	MPF_STATE,
	0
};

CMachineParameter const paraTranspose5 = 
{ 
	"Transpose 5",
	"Transpose 5",
	0,
	96,				
	MPF_STATE,
	0
};

CMachineParameter const paraTranspose6 = 
{ 
	"Transpose 6",
	"Transpose 6",
	0,
	96,				
	MPF_STATE,
	0
};

CMachineParameter const paraTranspose7 = 
{ 
	"Transpose 7",
	"Transpose 7",
	0,
	96,				
	MPF_STATE,
	0
};

CMachineParameter const paraTranspose8 = 
{ 
	"Transpose 8",
	"Transpose 8",
	0,
	96,				
	MPF_STATE,
	0
};

CMachineParameter const paraTranspose9 = 
{ 
	"Transpose 9",
	"Transpose 9",
	0,
	96,				
	MPF_STATE,
	0
};

CMachineParameter const paraTranspose10 = 
{ 
	"Transpose 10",
	"Transpose 10",
	0,
	96,				
	MPF_STATE,
	0
};

CMachineParameter const paraTranspose11 = 
{ 
	"Transpose 11",
	"Transpose 11",
	0,
	96,				
	MPF_STATE,
	0
};

CMachineParameter const paraTranspose12 = 
{ 
	"Transpose 12",
	"Transpose 12",
	0,
	96,				
	MPF_STATE,
	0
};

CMachineParameter const paraTranspose13 = 
{ 
	"Transpose 13",
	"Transpose 13",
	0,
	96,				
	MPF_STATE,
	0
};

CMachineParameter const paraTranspose14 = 
{ 
	"Transpose 14",
	"Transpose 14",
	0,
	96,				
	MPF_STATE,
	0
};

CMachineParameter const paraTranspose15 = 
{ 
	"Transpose 15",
	"Transpose 15",
	0,
	96,				
	MPF_STATE,
	0
};

CMachineParameter const paraTranspose16 = 
{ 
	"Transpose 16",
	"Transpose 16",
	0,
	96,				
	MPF_STATE,
	0
};

CMachineParameter const paraOption1 = 
{ 
	"Option 1",
	"Option 1",
	0,
	3,				
	MPF_STATE,
	0
};

CMachineParameter const paraOption2 = 
{ 
	"Option 2",
	"Option 2",
	0,
	3,				
	MPF_STATE,
	0
};

CMachineParameter const paraOption3 = 
{ 
	"Option 3",
	"Option 3",
	0,
	3,				
	MPF_STATE,
	0
};

CMachineParameter const paraOption4 = 
{ 
	"Option 4",
	"Option 4",
	0,
	3,				
	MPF_STATE,
	0
};

CMachineParameter const paraOption5 = 
{ 
	"Option 5",
	"Option 5",
	0,
	3,				
	MPF_STATE,
	0
};

CMachineParameter const paraOption6 = 
{ 
	"Option 6",
	"Option 6",
	0,
	3,				
	MPF_STATE,
	0
};

CMachineParameter const paraOption7 = 
{ 
	"Option 7",
	"Option 7",
	0,
	3,				
	MPF_STATE,
	0
};

CMachineParameter const paraOption8 = 
{ 
	"Option 8",
	"Option 8",
	0,
	3,				
	MPF_STATE,
	0
};

CMachineParameter const paraOption9 = 
{ 
	"Option 9",
	"Option 9",
	0,
	3,				
	MPF_STATE,
	0
};

CMachineParameter const paraOption10 = 
{ 
	"Option 10",
	"Option 10",
	0,
	3,				
	MPF_STATE,
	0
};

CMachineParameter const paraOption11 = 
{ 
	"Option 11",
	"Option 11",
	0,
	3,				
	MPF_STATE,
	0
};

CMachineParameter const paraOption12 = 
{ 
	"Option 12",
	"Option 12",
	0,
	3,				
	MPF_STATE,
	0
};

CMachineParameter const paraOption13 = 
{ 
	"Option 13",
	"Option 13",
	0,
	3,				
	MPF_STATE,
	0
};

CMachineParameter const paraOption14 = 
{ 
	"Option 14",
	"Option 14",
	0,
	3,				
	MPF_STATE,
	0
};

CMachineParameter const paraOption15 = 
{ 
	"Option 15",
	"Option 15",
	0,
	3,				
	MPF_STATE,
	0
};

CMachineParameter const paraOption16 = 
{ 
	"Option 16",
	"Option 16",
	0,
	3,				
	MPF_STATE,
	0
};


CMachineParameter const paraCommand1 = 
{ 
	"Command 1",
	"Command 1",
	0,
	15,				
	MPF_STATE,
	0
};

CMachineParameter const paraCommand2 = 
{ 
	"Command 2",
	"Command 2",
	0,
	15,				
	MPF_STATE,
	0
};

CMachineParameter const paraCommand3 = 
{ 
	"Command 3",
	"Command 3",
	0,
	15,				
	MPF_STATE,
	0
};
CMachineParameter const paraCommand4 = 
{ 
	"Command 4",
	"Command 4",
	0,
	15,				
	MPF_STATE,
	0
};
CMachineParameter const paraCommand5 = 
{ 
	"Command 5",
	"Command 5",
	0,
	15,				
	MPF_STATE,
	0
};
CMachineParameter const paraCommand6 = 
{ 
	"Command 6",
	"Command 6",
	0,
	15,				
	MPF_STATE,
	0
};
CMachineParameter const paraCommand7 = 
{ 
	"Command 7",
	"Command 7",
	0,
	15,				
	MPF_STATE,
	0
};
CMachineParameter const paraCommand8 = 
{ 
	"Command 8",
	"Command 8",
	0,
	15,				
	MPF_STATE,
	0
};
CMachineParameter const paraCommand9 = 
{ 
	"Command 9",
	"Command 9",
	0,
	15,				
	MPF_STATE,
	0
};
CMachineParameter const paraCommand10 = 
{ 
	"Command 10",
	"Command 10",
	0,
	15,				
	MPF_STATE,
	0
};
CMachineParameter const paraCommand11 = 
{ 
	"Command 11",
	"Command 11",
	0,
	15,				
	MPF_STATE,
	0
};
CMachineParameter const paraCommand12 = 
{ 
	"Command 12",
	"Command 12",
	0,
	15,				
	MPF_STATE,
	0
};
CMachineParameter const paraCommand13 = 
{ 
	"Command 13",
	"Command 13",
	0,
	15,				
	MPF_STATE,
	0
};
CMachineParameter const paraCommand14 = 
{ 
	"Command 14",
	"Command 14",
	0,
	15,				
	MPF_STATE,
	0
};
CMachineParameter const paraCommand15 = 
{ 
	"Command 15",
	"Command 15",
	0,
	15,				
	MPF_STATE,
	0
};
CMachineParameter const paraCommand16 = 
{ 
	"Command 16",
	"Command 16",
	0,
	15,				
	MPF_STATE,
	0
};

CMachineParameter const paraParameter1 = 
{ 
	"Parameter 1",
	"Parameter 1",
	0,
	255,
	MPF_STATE,
	0
};

CMachineParameter const paraParameter2 = 
{ 
	"Parameter 2",
	"Parameter 2",
	0,
	255,
	MPF_STATE,
	0
};

CMachineParameter const paraParameter3 = 
{ 
	"Parameter 3",
	"Parameter 3",
	0,
	255,
	MPF_STATE,
	0
};

CMachineParameter const paraParameter4 = 
{ 
	"Parameter 4",
	"Parameter 4",
	0,
	255,
	MPF_STATE,
	0
};

CMachineParameter const paraParameter5 = 
{ 
	"Parameter 5",
	"Parameter 5",
	0,
	255,
	MPF_STATE,
	0
};

CMachineParameter const paraParameter6 = 
{ 
	"Parameter 6",
	"Parameter 6",
	0,
	255,
	MPF_STATE,
	0
};

CMachineParameter const paraParameter7 = 
{ 
	"Parameter 7",
	"Parameter 7",
	0,
	255,
	MPF_STATE,
	0
};

CMachineParameter const paraParameter8 = 
{ 
	"Parameter 8",
	"Parameter 8",
	0,
	255,
	MPF_STATE,
	0
};

CMachineParameter const paraParameter9 = 
{ 
	"Parameter 9",
	"Parameter 9",
	0,
	255,
	MPF_STATE,
	0
};

CMachineParameter const paraParameter10 = 
{ 
	"Parameter 10",
	"Parameter 10",
	0,
	255,
	MPF_STATE,
	0
};

CMachineParameter const paraParameter11 = 
{ 
	"Parameter 11",
	"Parameter 11",
	0,
	255,
	MPF_STATE,
	0
};

CMachineParameter const paraParameter12 = 
{ 
	"Parameter 12",
	"Parameter 12",
	0,
	255,
	MPF_STATE,
	0
};

CMachineParameter const paraParameter13 = 
{ 
	"Parameter 13",
	"Parameter 13",
	0,
	255,
	MPF_STATE,
	0
};

CMachineParameter const paraParameter14 = 
{ 
	"Parameter 14",
	"Parameter 14",
	0,
	255,
	MPF_STATE,
	0
};

CMachineParameter const paraParameter15 = 
{
	"Parameter 15",
	"Parameter 15",
	0,
	255,
	MPF_STATE,
	0
};

CMachineParameter const paraParameter16 = 
{ 
	"Parameter 16",
	"Parameter 16",
	0,
	255,
	MPF_STATE,
	0
};

CMachineParameter const paraSpeed1 = 
{ 
	"Speed 1",
	"Speed 1",
	0,
	255,
	MPF_STATE,
	0
};

CMachineParameter const paraSpeed2 = 
{ 
	"Speed 2",
	"Speed 2",
	0,
	255,
	MPF_STATE,
	0
};

CMachineParameter const paraSpeed3 = 
{ 
	"Speed 3",
	"Speed 3",
	0,
	255,
	MPF_STATE,
	0
};

CMachineParameter const paraSpeed4 = 
{ 
	"Speed 4",
	"Speed 4",
	0,
	255,
	MPF_STATE,
	0
};

CMachineParameter const paraSpeed5 = 
{ 
	"Speed 5",
	"Speed 5",
	0,
	255,
	MPF_STATE,
	0
};

CMachineParameter const paraSpeed6 = 
{ 
	"Speed 6",
	"Speed 6",
	0,
	255,
	MPF_STATE,
	0
};

CMachineParameter const paraSpeed7 = 
{ 
	"Speed 7",
	"Speed 7",
	0,
	255,
	MPF_STATE,
	0
};

CMachineParameter const paraSpeed8 = 
{ 
	"Speed 8",
	"Speed 8",
	0,
	255,
	MPF_STATE,
	0
};

CMachineParameter const paraSpeed9 = 
{ 
	"Speed 9",
	"Speed 9",
	0,
	255,
	MPF_STATE,
	0
};

CMachineParameter const paraSpeed10 = 
{ 
	"Speed 10",
	"Speed 10",
	0,
	255,
	MPF_STATE,
	0
};

CMachineParameter const paraSpeed11 = 
{ 
	"Speed 11",
	"Speed 11",
	0,
	255,
	MPF_STATE,
	0
};

CMachineParameter const paraSpeed12 = 
{ 
	"Speed 12",
	"Speed 12",
	0,
	255,
	MPF_STATE,
	0
};

CMachineParameter const paraSpeed13 = 
{ 
	"Speed 13",
	"Speed 13",
	0,
	255,
	MPF_STATE,
	0
};

CMachineParameter const paraSpeed14 = 
{ 
	"Speed 14",
	"Speed 14",
	0,
	255,
	MPF_STATE,
	0
};

CMachineParameter const paraSpeed15 = 
{ 
	"Speed 15",
	"Speed 15",
	0,
	255,
	MPF_STATE,
	0
};

CMachineParameter const paraSpeed16 = 
{ 
	"Speed 16",
	"Speed 16",
	0,
	255,
	MPF_STATE,
	0
};


CMachineParameter const paraStartPos = 
{ 
	"Start Pos",
	"Start Pos",
	1,
	16,				
	MPF_STATE,
	1
};

CMachineParameter const paraLoopStart = 
{ 
	"Loop Start",
	"Loop Start",
	1,				
	16,								
	MPF_STATE,				
	1
};


CMachineParameter const paraLoopEnd = 
{ 
	"Loop End",
	"Loop End",				
	1,				
	16,								
	MPF_STATE,				
	1
};

CMachineParameter const paraReplaySpeed = 
{ 
	"Replay Speed",
	"Replay Speed",
	1,				
	255,				
	MPF_STATE,				
	128
};


CMachineParameter const paraVCAattack = 
{ 
	"VCA Attack",
	"VCA Attack",
	32,				
	MAX_ENV_TIME,
	MPF_STATE,				
	32
};


CMachineParameter const paraVCAdecay = 
{ 
	"VCA Decay",
	"VCA Decay",
	32,				
	MAX_ENV_TIME,
	MPF_STATE,				
	6341
};

CMachineParameter const paraVCAsustain =
{ 
	"VCA Sustain",
	"VCA Sustain level",
	0,				
	256,				
	MPF_STATE,				
	192
};


CMachineParameter const paraVCArelease = 
{ 
	"VCA Release",
	"VCA Release",
	32,				
	MAX_ENV_TIME,
	MPF_STATE,				
	2630
};

CMachineParameter const paraVCFcutoff =
{ 
	"VCF Cutoff",
	"VCF Cutoff",
	0,				
	255,				
	MPF_STATE,				
	120
};

CMachineParameter const paraVCFresonance = 
{ 
	"VCF Resonance",
	"VCF Resonance",
	1,				
	240,				
	MPF_STATE,				
	1
};



CMachineParameter const paraVCFenvmod = 
{ 
	"VCF Envmod",
	"VCF Envmod",
	0,				
	255,				
	MPF_STATE,				
	80
};

CMachineParameter const paraVCFattack = 
{ 
	"VCF Attack",
	"VCF Attack",
	32,				
	MAX_ENV_TIME,
	MPF_STATE,				
	589
};


CMachineParameter const paraVCFdecay = 
{ 
	"VCF Decay",
	"VCF Decay",
	32,				
	MAX_ENV_TIME,
	MPF_STATE,				
	2630
};

CMachineParameter const paraVCFsustain = 
{ 
	"VCF Sustain",
	"VCF Sustain level",
	0,				
	256,				
	MPF_STATE,				
	0
};

CMachineParameter const paraVCFrelease = 
{ 
	"VCF Release",
	"VCF Release",
	32,				
	MAX_ENV_TIME,
	MPF_STATE,				
	2630
};


CMachineParameter const paraFinetune = 
{
	"Finetune",
	"Finetune",				
	-256,
	256,				
	MPF_STATE,				
	0
};

CMachineParameter const *pParameters[] = 
{ 
	&paraVol1,
	&paraVol2,
	&paraVol3,
	&paraVol4,
	&paraVol5,
	&paraVol6,
	&paraVol7,
	&paraVol8,
	&paraVol9,
	&paraVol10,
	&paraVol11,
	&paraVol12,
	&paraVol13,
	&paraVol14,
	&paraVol15,
	&paraVol16,
	&paraWave1,
	&paraWave2,
	&paraWave3,
	&paraWave4,
	&paraWave5,
	&paraWave6,
	&paraWave7,
	&paraWave8,
	&paraWave9,
	&paraWave10,
	&paraWave11,
	&paraWave12,
	&paraWave13,
	&paraWave14,
	&paraWave15,
	&paraWave16,
	&paraTranspose1,
	&paraTranspose2,
	&paraTranspose3,
	&paraTranspose4,
	&paraTranspose5,
	&paraTranspose6,
	&paraTranspose7,
	&paraTranspose8,
	&paraTranspose9,
	&paraTranspose10,
	&paraTranspose11,
	&paraTranspose12,
	&paraTranspose13,
	&paraTranspose14,
	&paraTranspose15,
	&paraTranspose16,
	&paraOption1,
	&paraOption2,
	&paraOption3,
	&paraOption4,
	&paraOption5,
	&paraOption6,
	&paraOption7,
	&paraOption8,
	&paraOption9,
	&paraOption10,
	&paraOption11,
	&paraOption12,
	&paraOption13,
	&paraOption14,
	&paraOption15,
	&paraOption16,
	&paraCommand1,
	&paraCommand2,
	&paraCommand3,
	&paraCommand4,
	&paraCommand5,
	&paraCommand6,
	&paraCommand7,
	&paraCommand8,
	&paraCommand9,
	&paraCommand10,
	&paraCommand11,
	&paraCommand12,
	&paraCommand13,
	&paraCommand14,
	&paraCommand15,
	&paraCommand16,
	&paraParameter1,
	&paraParameter2,
	&paraParameter3,
	&paraParameter4,
	&paraParameter5,
	&paraParameter6,
	&paraParameter7,
	&paraParameter8,
	&paraParameter9,
	&paraParameter10,
	&paraParameter11,
	&paraParameter12,
	&paraParameter13,
	&paraParameter14,
	&paraParameter15,
	&paraParameter16,
	&paraSpeed1,
	&paraSpeed2,
	&paraSpeed3,
	&paraSpeed4,
	&paraSpeed5,
	&paraSpeed6,
	&paraSpeed7,
	&paraSpeed8,
	&paraSpeed9,
	&paraSpeed10,
	&paraSpeed11,
	&paraSpeed12,
	&paraSpeed13,
	&paraSpeed14,
	&paraSpeed15,
	&paraSpeed16,
	&paraStartPos,
	&paraLoopStart,
	&paraLoopEnd,
	&paraReplaySpeed,
	&paraVCAattack,
	&paraVCAdecay,
	&paraVCAsustain,
	&paraVCArelease,
	&paraVCFcutoff,
	&paraVCFresonance,
	&paraVCFenvmod,
	&paraVCFattack,
	&paraVCFdecay,
	&paraVCFsustain,
	&paraVCFrelease,
	&paraFinetune,
};

CMachineInfo const MacInfo = 
{
	MI_VERSION,				
	GENERATOR,
	128,
	pParameters,
	#ifndef NDEBUG
		"GameFX (Debug build)",
	#else
		"GameFX 1.3",
	#endif
	"GameFX",
	"jme",
	"Help",
	8
};

class mi : public CMachineInterface
{
public:
	void InitWaveTable();
	mi();
	virtual ~mi();
	virtual void Init();
	virtual void Work(float *psamplesleft, float* psamplesright, int numsamples, int tracks);
	virtual bool DescribeValue(char* txt,int const param, int const value);
	virtual void Command();
	virtual void ParameterTweak(int par, int val);
	virtual void SeqTick(int channel, int note, int ins, int cmd, int val);
	virtual void Stop();
private:
	CSynthTrack track[MAX_TRACKS];
	PERFORMANCE globals;
};

PSYCLE__PLUGIN__INSTANCIATOR(mi, MacInfo)

mi::mi()
{
	Vals=new int[128];
	InitWaveTable();
}

mi::~mi()
{
	delete Vals;
}

void mi::Init()
{
	globals.reg = 0x7ffff8; // init noise register
	globals.noiseindex = 0;
}

void mi::Stop()
{
	for(int c=0;c<MAX_TRACKS;c++)
	track[c].NoteOff();
}

void mi::ParameterTweak(int par, int val)
{
	Vals[par]=val;
	globals.Volume[0]=Vals[0];
	globals.Volume[1]=Vals[1];
	globals.Volume[2]=Vals[2];
	globals.Volume[3]=Vals[3];
	globals.Volume[4]=Vals[4];
	globals.Volume[5]=Vals[5];
	globals.Volume[6]=Vals[6];
	globals.Volume[7]=Vals[7];
	globals.Volume[8]=Vals[8];
	globals.Volume[9]=Vals[9];
	globals.Volume[10]=Vals[10];
	globals.Volume[11]=Vals[11];
	globals.Volume[12]=Vals[12];
	globals.Volume[13]=Vals[13];
	globals.Volume[14]=Vals[14];
	globals.Volume[15]=Vals[15];
	globals.Waveform[0]=Vals[16];
	globals.Waveform[1]=Vals[17];
	globals.Waveform[2]=Vals[18];
	globals.Waveform[3]=Vals[19];
	globals.Waveform[4]=Vals[20];
	globals.Waveform[5]=Vals[21];
	globals.Waveform[6]=Vals[22];
	globals.Waveform[7]=Vals[23];
	globals.Waveform[8]=Vals[24];
	globals.Waveform[9]=Vals[25];
	globals.Waveform[10]=Vals[26];
	globals.Waveform[11]=Vals[27];
	globals.Waveform[12]=Vals[28];
	globals.Waveform[13]=Vals[29];
	globals.Waveform[14]=Vals[30];
	globals.Waveform[15]=Vals[31];
	globals.Transpose[0]=Vals[32];
	globals.Transpose[1]=Vals[33];
	globals.Transpose[2]=Vals[34];
	globals.Transpose[3]=Vals[35];
	globals.Transpose[4]=Vals[36];
	globals.Transpose[5]=Vals[37];
	globals.Transpose[6]=Vals[38];
	globals.Transpose[7]=Vals[39];
	globals.Transpose[8]=Vals[40];
	globals.Transpose[9]=Vals[41];
	globals.Transpose[10]=Vals[42];
	globals.Transpose[11]=Vals[43];
	globals.Transpose[12]=Vals[44];
	globals.Transpose[13]=Vals[45];
	globals.Transpose[14]=Vals[46];
	globals.Transpose[15]=Vals[47];
	globals.Option[0]=Vals[48];
	globals.Option[1]=Vals[49];
	globals.Option[2]=Vals[50];
	globals.Option[3]=Vals[51];
	globals.Option[4]=Vals[52];
	globals.Option[5]=Vals[53];
	globals.Option[6]=Vals[54];
	globals.Option[7]=Vals[55];
	globals.Option[8]=Vals[56];
	globals.Option[9]=Vals[57];
	globals.Option[10]=Vals[58];
	globals.Option[11]=Vals[59];
	globals.Option[12]=Vals[60];
	globals.Option[13]=Vals[61];
	globals.Option[14]=Vals[62];
	globals.Option[15]=Vals[63];
	globals.Command[0]=Vals[64];
	globals.Command[1]=Vals[65];
	globals.Command[2]=Vals[66];
	globals.Command[3]=Vals[67];
	globals.Command[4]=Vals[68];
	globals.Command[5]=Vals[69];
	globals.Command[6]=Vals[70];
	globals.Command[7]=Vals[71];
	globals.Command[8]=Vals[72];
	globals.Command[9]=Vals[73];
	globals.Command[10]=Vals[74];
	globals.Command[11]=Vals[75];
	globals.Command[12]=Vals[76];
	globals.Command[13]=Vals[77];
	globals.Command[14]=Vals[78];
	globals.Command[15]=Vals[79];
	globals.Parameter[0]=Vals[80];
	globals.Parameter[1]=Vals[81];
	globals.Parameter[2]=Vals[82];
	globals.Parameter[3]=Vals[83];
	globals.Parameter[4]=Vals[84];
	globals.Parameter[5]=Vals[85];
	globals.Parameter[6]=Vals[86];
	globals.Parameter[7]=Vals[87];
	globals.Parameter[8]=Vals[88];
	globals.Parameter[9]=Vals[89];
	globals.Parameter[10]=Vals[90];
	globals.Parameter[11]=Vals[91];
	globals.Parameter[12]=Vals[92];
	globals.Parameter[13]=Vals[93];
	globals.Parameter[14]=Vals[94];
	globals.Parameter[15]=Vals[95];
	globals.Speed[0]=Vals[96];
	globals.Speed[1]=Vals[97];
	globals.Speed[2]=Vals[98];
	globals.Speed[3]=Vals[99];
	globals.Speed[4]=Vals[100];
	globals.Speed[5]=Vals[101];
	globals.Speed[6]=Vals[102];
	globals.Speed[7]=Vals[103];
	globals.Speed[8]=Vals[104];
	globals.Speed[9]=Vals[105];
	globals.Speed[10]=Vals[106];
	globals.Speed[11]=Vals[107];
	globals.Speed[12]=Vals[108];
	globals.Speed[13]=Vals[109];
	globals.Speed[14]=Vals[110];
	globals.Speed[15]=Vals[111];
	globals.StartPos=Vals[112]-1;
	globals.LoopStart=Vals[113]-1;
	globals.LoopEnd=Vals[114]-1;
	globals.ReplaySpeed=Vals[115];
	globals.AEGAttack=Vals[116];
	globals.AEGDecay=Vals[117];
	globals.AEGSustain=Vals[118];
	globals.AEGRelease=Vals[119];
	globals.Cutoff=Vals[120];
	globals.Resonance=Vals[121];
	globals.EnvMod=Vals[122];
	globals.FEGAttack=Vals[123];
	globals.FEGDecay=Vals[124];
	globals.FEGSustain=Vals[125];
	globals.FEGRelease=Vals[126];
	globals.Finetune=Vals[127];
}

void mi::Command()
{
	char buffer[2048];
	sprintf
		(
			buffer,"%s%s%s",
			"Pattern commands\n",
			"\n03xx : Glide",
			"\n0Cxx : Volume"
		);
	pCB->MessBox(buffer,"hello",0);
}

void mi::Work(float *psamplesleft, float *psamplesright , int numsamples, int tracks)
{
	float sl=0;
	globals.noiseused=false;
	for(int c=0;c<tracks;c++)
	{
		if(track[c].AmpEnvStage)
		{
			float *xpsamplesleft=psamplesleft;
			float *xpsamplesright=psamplesright;
			int xnumsamples=numsamples;

			--xpsamplesleft;
			--xpsamplesright;

			track[c].PerformFx();
			do
			{
				sl=0;								
				sl+=track[c].GetSample();

				*++xpsamplesleft+=sl;
				*++xpsamplesright+=sl;
			
			} while(--xnumsamples);
		}
	}
	if (globals.noiseused)
	{
		signed short noisework=0;
		for (int i = 0; i < 256; i++){
			globals.noise=(((globals.reg & (1<<22))? 1:0) << 7) |
			(((globals.reg & (1<<20))? 1:0) << 6) |
			(((globals.reg & (1<<16))? 1:0) << 5) |
			(((globals.reg & (1<<13))? 1:0) << 4) |
			(((globals.reg & (1<<11))? 1:0) << 3) |
			(((globals.reg & (1<<7))? 1:0) << 2)  |
			(((globals.reg & (1<<4))? 1:0) << 1)  |
			(((globals.reg & (1<<2))? 1:0) << 0);
			/* Save bits used to feed bit 0 */
			globals.bit22= (globals.reg & (1<<22))? 1:0;
			globals.bit17= (globals.reg & (1<<17))? 1:0;
			/* Shift 1 bit left */
			globals.reg= globals.reg << 1;
			/* Feed bit 0 */
			globals.reg= globals.reg | (globals.bit22 ^ globals.bit17);
			noisework=(signed short)globals.noise<<8;
			globals.Wavetable[7][globals.noiseindex++]=noisework;
			globals.Wavetable[7][globals.noiseindex++]=noisework;
			globals.Wavetable[7][globals.noiseindex++]=noisework;
			globals.Wavetable[7][globals.noiseindex++]=noisework;
			globals.Wavetable[7][globals.noiseindex++]=noisework;
			globals.Wavetable[7][globals.noiseindex++]=noisework;
			globals.Wavetable[7][globals.noiseindex++]=noisework;
			globals.Wavetable[7][globals.noiseindex++]=noisework;
			globals.Wavetable[7][globals.noiseindex++]=noisework;
			globals.Wavetable[7][globals.noiseindex++]=noisework;
			globals.Wavetable[7][globals.noiseindex++]=noisework;
			globals.Wavetable[7][globals.noiseindex++]=noisework;
			globals.Wavetable[7][globals.noiseindex++]=noisework;
			globals.Wavetable[7][globals.noiseindex++]=noisework;
			globals.Wavetable[7][globals.noiseindex++]=noisework;
			globals.Wavetable[7][globals.noiseindex++]=noisework;
			globals.Wavetable[7][globals.noiseindex++]=noisework;
			globals.Wavetable[7][globals.noiseindex++]=noisework;
			globals.Wavetable[7][globals.noiseindex++]=noisework;
			globals.Wavetable[7][globals.noiseindex++]=noisework;
			globals.Wavetable[7][globals.noiseindex++]=noisework;
			globals.Wavetable[7][globals.noiseindex++]=noisework;
			globals.Wavetable[7][globals.noiseindex++]=noisework;
			globals.Wavetable[7][globals.noiseindex++]=noisework;
			globals.Wavetable[7][globals.noiseindex++]=noisework;
			globals.Wavetable[7][globals.noiseindex++]=noisework;
			globals.Wavetable[7][globals.noiseindex++]=noisework;
			globals.Wavetable[7][globals.noiseindex++]=noisework;
			globals.Wavetable[7][globals.noiseindex++]=noisework;
			globals.Wavetable[7][globals.noiseindex++]=noisework;
			globals.Wavetable[7][globals.noiseindex++]=noisework;
			globals.Wavetable[7][globals.noiseindex++]=noisework;
			globals.noiseindex=globals.noiseindex&2047;
		}
		globals.shortnoise=noisework;
	}
}

bool mi::DescribeValue(char* txt,int const param, int const value)
{
	if(param<16)
	{				switch(value)
		{
			case 0:sprintf(txt,"---");return true;break;
			default: sprintf(txt,"%i",value-1);return true;break;

		}
	}

	// Oscillators waveform descriptions
	if(param>15 & param<32)
	{
		switch(value)
		{
			case 0:sprintf(txt,"---");return true;break;
			case 1:sprintf(txt,"Sine");return true;break;
			case 2:sprintf(txt,"Triangle");return true;break;
			case 3:sprintf(txt,"Sine'n'back");return true;break;
			case 4:sprintf(txt,"Triangle'n'back");return true;break;
			case 5:sprintf(txt,"Square");return true;break;
			case 6:sprintf(txt,"Pulse");return true;break;
			case 7:sprintf(txt,"Sawtooth");return true;break;
			case 8:sprintf(txt,"Noise");return true;break;
			case 9:sprintf(txt,"Low Noise");return true;break;
		}
	}

	if(param>31 & param<48)
	{
		switch(value)
		{
			case 0:sprintf(txt,"---");return true;break;
			case 1:sprintf(txt,"C-0");return true;break;
			case 2:sprintf(txt,"C#0");return true;break;
			case 3:sprintf(txt,"D-0");return true;break;
			case 4:sprintf(txt,"D#0");return true;break;
			case 5:sprintf(txt,"E-0");return true;break;
			case 6:sprintf(txt,"F-0");return true;break;
			case 7:sprintf(txt,"F#0");return true;break;
			case 8:sprintf(txt,"G-0");return true;break;
			case 9:sprintf(txt,"G#0");return true;break;
			case 10:sprintf(txt,"A-0");return true;break;
			case 11:sprintf(txt,"A#0");return true;break;
			case 12:sprintf(txt,"B-0");return true;break;
			case 13:sprintf(txt,"C-1");return true;break;
			case 14:sprintf(txt,"C#1");return true;break;
			case 15:sprintf(txt,"D-1");return true;break;
			case 16:sprintf(txt,"D#1");return true;break;
			case 17:sprintf(txt,"E-1");return true;break;
			case 18:sprintf(txt,"F-1");return true;break;
			case 19:sprintf(txt,"F#1");return true;break;
			case 20:sprintf(txt,"G-1");return true;break;
			case 21:sprintf(txt,"G#1");return true;break;
			case 22:sprintf(txt,"A-1");return true;break;
			case 23:sprintf(txt,"A#1");return true;break;
			case 24:sprintf(txt,"B-1");return true;break;
			case 25:sprintf(txt,"C-2");return true;break;
			case 26:sprintf(txt,"C#2");return true;break;
			case 27:sprintf(txt,"D-2");return true;break;
			case 28:sprintf(txt,"D#2");return true;break;
			case 29:sprintf(txt,"E-2");return true;break;
			case 30:sprintf(txt,"F-2");return true;break;
			case 31:sprintf(txt,"F#2");return true;break;
			case 32:sprintf(txt,"G-2");return true;break;
			case 33:sprintf(txt,"G#2");return true;break;
			case 34:sprintf(txt,"A-2");return true;break;
			case 35:sprintf(txt,"A#2");return true;break;
			case 36:sprintf(txt,"B-2");return true;break;
			case 37:sprintf(txt,"C-3");return true;break;
			case 38:sprintf(txt,"C#3");return true;break;
			case 39:sprintf(txt,"D-3");return true;break;
			case 40:sprintf(txt,"D#3");return true;break;
			case 41:sprintf(txt,"E-3");return true;break;
			case 42:sprintf(txt,"F-3");return true;break;
			case 43:sprintf(txt,"F#3");return true;break;
			case 44:sprintf(txt,"G-3");return true;break;
			case 45:sprintf(txt,"G#3");return true;break;
			case 46:sprintf(txt,"A-3");return true;break;
			case 47:sprintf(txt,"A#3");return true;break;
			case 48:sprintf(txt,"B-3");return true;break;
			case 49:sprintf(txt,"C-4");return true;break;
			case 50:sprintf(txt,"C#4");return true;break;
			case 51:sprintf(txt,"D-4");return true;break;
			case 52:sprintf(txt,"D#4");return true;break;
			case 53:sprintf(txt,"E-4");return true;break;
			case 54:sprintf(txt,"F-4");return true;break;
			case 55:sprintf(txt,"F#4");return true;break;
			case 56:sprintf(txt,"G-4");return true;break;
			case 57:sprintf(txt,"G#4");return true;break;
			case 58:sprintf(txt,"A-4");return true;break;
			case 59:sprintf(txt,"A#4");return true;break;
			case 60:sprintf(txt,"B-4");return true;break;
			case 61:sprintf(txt,"C-5");return true;break;
			case 62:sprintf(txt,"C#5");return true;break;
			case 63:sprintf(txt,"D-5");return true;break;
			case 64:sprintf(txt,"D#5");return true;break;
			case 65:sprintf(txt,"E-5");return true;break;
			case 66:sprintf(txt,"F-5");return true;break;
			case 67:sprintf(txt,"F#5");return true;break;
			case 68:sprintf(txt,"G-5");return true;break;
			case 69:sprintf(txt,"G#5");return true;break;
			case 70:sprintf(txt,"A-5");return true;break;
			case 71:sprintf(txt,"A#5");return true;break;
			case 72:sprintf(txt,"B-5");return true;break;
			case 73:sprintf(txt,"C-6");return true;break;
			case 74:sprintf(txt,"C#6");return true;break;
			case 75:sprintf(txt,"D-6");return true;break;
			case 76:sprintf(txt,"D#6");return true;break;
			case 77:sprintf(txt,"E-6");return true;break;
			case 78:sprintf(txt,"F-6");return true;break;
			case 79:sprintf(txt,"F#6");return true;break;
			case 80:sprintf(txt,"G-6");return true;break;
			case 81:sprintf(txt,"G#6");return true;break;
			case 82:sprintf(txt,"A-6");return true;break;
			case 83:sprintf(txt,"A#6");return true;break;
			case 84:sprintf(txt,"B-6");return true;break;
			case 85:sprintf(txt,"C-7");return true;break;
			case 86:sprintf(txt,"C#7");return true;break;
			case 87:sprintf(txt,"D-7");return true;break;
			case 88:sprintf(txt,"D#7");return true;break;
			case 89:sprintf(txt,"E-7");return true;break;
			case 90:sprintf(txt,"F-7");return true;break;
			case 91:sprintf(txt,"F#7");return true;break;
			case 92:sprintf(txt,"G-7");return true;break;
			case 93:sprintf(txt,"G#7");return true;break;
			case 94:sprintf(txt,"A-7");return true;break;
			case 95:sprintf(txt,"A#7");return true;break;
			case 96:sprintf(txt,"B-7");return true;break;
		}
	}

	if(param>47 & param<64)
	{
		switch(value)
		{
			case 0:sprintf(txt,"---");return true;break;
			case 1:sprintf(txt,"Fix");return true;break;
			case 2:sprintf(txt,"Retrig");return true;break;
			case 3:sprintf(txt,"Fix'n'Retrig");return true;break;
		}
	}

	if(param>63 & param<80)
	{
		switch(value)
		{
			case 0:sprintf(txt,"---");return true;break;
			case 1:sprintf(txt,"Inc Pitch");return true;break;
			case 2:sprintf(txt,"Dec Pitch");return true;break;
			case 3:sprintf(txt,"Inc Note");return true;break;
			case 4:sprintf(txt,"Dec Note");return true;break;
			case 5:sprintf(txt,"Set Pulse Width");return true;break;
			case 6:sprintf(txt,"Inc Pulse Width");return true;break;
			case 7:sprintf(txt,"Dec Pulse Width");return true;break;
			case 8:sprintf(txt,"Set Osc Phase");return true;break;
			case 9:sprintf(txt,"Filter: Off");return true;break;
			case 10:sprintf(txt,"Filter: Lowpass (Default)");return true;break;
			case 11:sprintf(txt,"Filter: Highpass");return true;break;
			case 15:sprintf(txt,"Gate Off (Release)");return true;break;
			default: sprintf(txt,"Reserved");return true;break;
		}
	}

	if(param>95 & param<112)
	{				switch(value)
		{
			case 0:sprintf(txt,"---");return true;break;
		}
	}
	return false;
}

void mi::SeqTick(int channel, int note, int ins, int cmd, int val)
{
	if (channel < MAX_TRACKS){

	track[channel].InitEffect(cmd,val);
	
	// Global scope synth pattern commands
	switch(cmd)
	{
		case 7: // Change envmod
			globals.EnvMod=val;
			break;
		case 8: // Change cutoff
			globals.Cutoff=val/2;
			break;
		case 9: // Change reso
			globals.Resonance=val/2;
			break;
	}

	// Note Off == 120
	// Empty Note Row == 255
	// Less than note off value??? == NoteON!
	if(note<120) 
	{
		if ( cmd == 0x0C ) track[channel].NoteOn(note-24,&globals,val);
		else track[channel].NoteOn(note-24,&globals,60);
	}

	// Note off
	if(note==120) track[channel].NoteOff();
}
}

void mi::InitWaveTable()
{
	int c=0;
	for(c=0;c<2048;c++)
	{
		double sval=c*0.00306796157577128245943617517898389;

		//Sine
		globals.Wavetable[0][c]=int(sin(sval)*16384.0f);

		//Triangle
		if(c<1024)
		globals.Wavetable[1][c]=(c*32)-16384;
		else
		globals.Wavetable[1][c]=16384-((c-1024)*32);

		//Square
		if(c<1024)
		globals.Wavetable[4][c]=-16384;
		else
		globals.Wavetable[4][c]=16384;

		//Saw
		globals.Wavetable[6][c]=(c*16)-16384;
	}
	for (c=0;c<4096;c++)
	{
		//Sine'n'back
		if(c<1024)
		{
			globals.Wavetable[2][c]=globals.Wavetable[0][c+c];
			globals.Wavetable[2][2047-c]=globals.Wavetable[2][c];
		}

		//Tri'n'back
		if(c<1024)
		{
			globals.Wavetable[3][c]=globals.Wavetable[1][(c+c+512)&2047];
			globals.Wavetable[3][2047-c]=globals.Wavetable[3][c];
		}

		//Pulse
		if(c<2048)
			globals.Wavetable[5][c]=-16384;
		else
			globals.Wavetable[5][c]=16384;
	}
}
