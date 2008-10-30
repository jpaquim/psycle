//////////////////////////////////////////////////////////////////////
//
//				Shared.h
//
//				druttis@darkface.pp.se
//
//////////////////////////////////////////////////////////////////////
#pragma once
#include "../dsp/Dsp.h"
#include <psycle/plugin_interface.hpp>
//////////////////////////////////////////////////////////////////////
//
//				Various buffers
//
//////////////////////////////////////////////////////////////////////
//				Monophonics
extern float				m_out[MAX_BUFFER_LENGTH];
//				Polyphonics
extern float				m_osc1_fm_out[MAX_BUFFER_LENGTH];
extern float				m_osc2_fm_out[MAX_BUFFER_LENGTH];
extern float				m_mod_out[MAX_BUFFER_LENGTH];
extern float				m_lfo1_out[MAX_BUFFER_LENGTH];
extern float				m_lfo2_out[MAX_BUFFER_LENGTH];
extern float				m_out1[MAX_BUFFER_LENGTH];
extern float				m_out2[MAX_BUFFER_LENGTH];
extern float				m_osc1_phase_out[MAX_BUFFER_LENGTH];
extern float				m_osc2_phase_out[MAX_BUFFER_LENGTH];
