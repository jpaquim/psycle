//////////////////////////////////////////////////////////////////////
//
//	Waveform.cpp
//
//	druttis@darkface.pp.se
//
//////////////////////////////////////////////////////////////////////
#include <packageneric/pre-compiled.private.hpp>
#include "Waveform.h"
#include "../blwtbl/blwtbl.h"
//////////////////////////////////////////////////////////////////////
//
//	Increment to Frequency factor
//
//////////////////////////////////////////////////////////////////////
float incr2freq = 0.0f;
//////////////////////////////////////////////////////////////////////
//
//	Constructor
//
//////////////////////////////////////////////////////////////////////
Waveform::Waveform()
{
	m_wave.index = -1;
	Get(-1);
}
//////////////////////////////////////////////////////////////////////
//
//	Destructor
//
//////////////////////////////////////////////////////////////////////
Waveform::~Waveform()
{
	Get(-1);
}
//////////////////////////////////////////////////////////////////////
//
//	GetWaveform
//
//////////////////////////////////////////////////////////////////////
bool Waveform::Get(int index)
{
	return GetWaveform(index, &m_wave);
}
