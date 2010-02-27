// blwtbl.h
// druttis@darkface.pp.se
#pragma once
#include "../dsp/Waveform.h"
#include <universalis.hpp>

#ifdef DRUTTIS__BAND_LIMITED_WAVES_TABLES__SOURCE
	#define DRUTTIS__BAND_LIMITED_WAVES_TABLES__DECL UNIVERSALIS__COMPILER__DYN_LINK__EXPORT
#else
	#define DRUTTIS__BAND_LIMITED_WAVES_TABLES__DECL UNIVERSALIS__COMPILER__DYN_LINK__IMPORT
#endif

#if !defined DRUTTIS__BAND_LIMITED_WAVES_TABLES__SOURCE && defined DIVERSALIS__COMPILER__FEATURE__AUTO_LINK
	#pragma comment(lib, "blwtbl")
#endif

DRUTTIS__BAND_LIMITED_WAVES_TABLES__DECL bool EnableWaveform(int index);
DRUTTIS__BAND_LIMITED_WAVES_TABLES__DECL bool DisableWaveform(int index);
DRUTTIS__BAND_LIMITED_WAVES_TABLES__DECL bool GetWaveform(int wavenum, WAVEFORM *pwave);
DRUTTIS__BAND_LIMITED_WAVES_TABLES__DECL bool UpdateWaveforms(int samplingrate);
DRUTTIS__BAND_LIMITED_WAVES_TABLES__DECL float* GetFMTable();
DRUTTIS__BAND_LIMITED_WAVES_TABLES__DECL float* GetPMTable();
