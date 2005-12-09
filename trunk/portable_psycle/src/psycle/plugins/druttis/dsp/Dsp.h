//////////////////////////////////////////////////////////////////////
//
//	Dsp.h
//
//	druttis@darkface.pp.se
//
//////////////////////////////////////////////////////////////////////
#pragma once
#include "DspMath.h"
#include "Envelope.h"
#include "Filter.h"
#include "Formant.h"
#include "Inertia.h"
#include "Waveform.h"

extern float* pfmtable;
extern float* ppmtable;

//////////////////////////////////////////////////////////////////////
//
//	Initialization & Destruction functions
//
//////////////////////////////////////////////////////////////////////
void __cdecl InitializeDSP();
void __cdecl DestroyDSP();
void __fastcall Fill(float *pbuf, float value, int nsamples);
void __fastcall Copy(float *pbuf1, float *pbuf2, int nsamples);
void __fastcall Add(float *pbuf1, float *pbuf2, int nsamples);
void __fastcall Sub(float *pbuf1, float *pbuf2, int nsamples);
void __fastcall Mul(float *pbuf1, float *pbuf2, int nsamples);
