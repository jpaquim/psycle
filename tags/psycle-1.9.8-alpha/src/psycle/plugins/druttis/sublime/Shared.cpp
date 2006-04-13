//////////////////////////////////////////////////////////////////////
//
//	Shared.cpp
//
//	druttis@darkface.pp.se
//
//////////////////////////////////////////////////////////////////////
#include <packageneric/pre-compiled.private.hpp>
#include "Shared.h"
//////////////////////////////////////////////////////////////////////
//
//	Various buffers
//
//////////////////////////////////////////////////////////////////////
//	Monophonics
float	m_out[MAX_BUFFER_LENGTH];
//	Polyphonics
float	m_osc1_fm_out[MAX_BUFFER_LENGTH];
float	m_osc2_fm_out[MAX_BUFFER_LENGTH];
float	m_mod_out[MAX_BUFFER_LENGTH];
float	m_lfo1_out[MAX_BUFFER_LENGTH];
float	m_lfo2_out[MAX_BUFFER_LENGTH];
float	m_out1[MAX_BUFFER_LENGTH];
float	m_out2[MAX_BUFFER_LENGTH];
float	m_osc1_phase_out[MAX_BUFFER_LENGTH];
float	m_osc2_phase_out[MAX_BUFFER_LENGTH];
