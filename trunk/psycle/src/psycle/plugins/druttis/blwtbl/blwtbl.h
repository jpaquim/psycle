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
#ifdef DRUTTIS__BAND_LIMITED_WAVES_TABLES__SOURCE
#define DRUTTIS__BAND_LIMITED_WAVES_TABLES__DYNAMIC_LINK  UNIVERSALIS__COMPILER__DYNAMIC_LINK__EXPORT
#else
#define DRUTTIS__BAND_LIMITED_WAVES_TABLES__DYNAMIC_LINK  UNIVERSALIS__COMPILER__DYNAMIC_LINK__IMPORT
#endif
//////////////////////////////////////////////////////////////////////
//
//	DLL - Functions
//
//////////////////////////////////////////////////////////////////////
DRUTTIS__BAND_LIMITED_WAVES_TABLES__DYNAMIC_LINK bool EnableWaveform(int index);
DRUTTIS__BAND_LIMITED_WAVES_TABLES__DYNAMIC_LINK bool DisableWaveform(int index);
DRUTTIS__BAND_LIMITED_WAVES_TABLES__DYNAMIC_LINK bool GetWaveform(int wavenum, WAVEFORM *pwave);
DRUTTIS__BAND_LIMITED_WAVES_TABLES__DYNAMIC_LINK bool UpdateWaveforms(int samplingrate);
DRUTTIS__BAND_LIMITED_WAVES_TABLES__DYNAMIC_LINK float* GetFMTable();
DRUTTIS__BAND_LIMITED_WAVES_TABLES__DYNAMIC_LINK float* GetPMTable();
