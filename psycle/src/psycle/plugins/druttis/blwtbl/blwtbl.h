//////////////////////////////////////////////////////////////////////
//
//	blwtbl.h
//
//	druttis@darkface.pp.se
//
//////////////////////////////////////////////////////////////////////
#pragma once
#include "../dsp/Waveform.h"
//////////////////////////////////////////////////////////////////////
//
//	DLL - Macros
//
//////////////////////////////////////////////////////////////////////
#include <universalis/compiler.hpp>
#ifdef BLWTBL__SOURCE
#define BLWTBL__DYNAMIC_LINK  UNIVERSALIS__COMPILER__DYNAMIC_LINK__EXPORT
#else
#define BLWTBL__DYNAMIC_LINK  UNIVERSALIS__COMPILER__DYNAMIC_LINK__IMPORT
#endif
//////////////////////////////////////////////////////////////////////
//
//	DLL - Functions
//
//////////////////////////////////////////////////////////////////////
BLWTBL__DYNAMIC_LINK bool EnableWaveform(int index);
BLWTBL__DYNAMIC_LINK bool DisableWaveform(int index);
BLWTBL__DYNAMIC_LINK bool GetWaveform(int wavenum, WAVEFORM *pwave);
BLWTBL__DYNAMIC_LINK bool UpdateWaveforms(int samplingrate);
BLWTBL__DYNAMIC_LINK float* GetFMTable();
BLWTBL__DYNAMIC_LINK float* GetPMTable();
