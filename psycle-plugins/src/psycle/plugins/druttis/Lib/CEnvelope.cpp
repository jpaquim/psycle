// -*- mode:c++; indent-tabs-mode:t -*-
//============================================================================
//
//				CEnvelope.cpp
//
//============================================================================
#include "CEnvelope.h"
//============================================================================
//
//				CEnvelope
//
//============================================================================
CEnvelope::CEnvelope()
{
	m_attack = 1.0f;
	m_decay = 1.0f;
	m_sustain = 1.0f;
	m_release = 1.0f;
	m_stage = ENVELOPE_DONE;
	m_value = 0.0f;
	m_coeff = 0.0f;
}

CEnvelope::~CEnvelope()
{
}
//============================================================================
//				IsActive
//
//				Returns wether the envelope is active or is stated done.
//============================================================================
bool CEnvelope::IsActive()
{
	return m_stage != ENVELOPE_DONE;
}
//============================================================================
//				NoteOn
//
//				Tells the envelope to
//============================================================================
void CEnvelope::NoteOn(float attackModifier)
{
	float attack = m_attack + attackModifier;
	if (attack <= 0.0f)
		attack = 1.0f;
	m_stage = ENVELOPE_ATTACK;
	m_value = 0.0f;
	m_coeff = 1.0f / attack;
}
//============================================================================
//				NoteOff
//
//				Tells the envelope to release it's activity
//============================================================================
void CEnvelope::NoteOff()
{
	if (m_stage != ENVELOPE_DONE) {
		if (m_release > 0.0f) {
			m_stage = ENVELOPE_RELEASE;
			m_coeff = m_value / m_release;
		} else {
			m_stage = ENVELOPE_DONE;
			m_value = 0.0f;
			m_coeff = 0.0f;
		}
	}
}
//============================================================================
//				Stop
//
//				Inactivates the envelope and resets stage as done
//============================================================================
void CEnvelope::Stop()
{
	m_stage = ENVELOPE_DONE;
	m_value = 0.0f;
	m_coeff = 0.0f;
}
//============================================================================
//				Properties
//============================================================================
float CEnvelope::GetAttack()
{
	return m_attack;
}

void CEnvelope::SetAttack(float attack)
{
	if (attack <= 0.0f)
		attack = 1.0f;
	m_attack = attack;
}

float CEnvelope::GetDecay()
{
	return m_decay;
}

void CEnvelope::SetDecay(float decay)
{
	if (decay <= 0.0f)
		decay = 1.0f;
	m_decay = decay;
}

float CEnvelope::GetSustain()
{
	return m_sustain;
}

void CEnvelope::SetSustain(float sustain)
{
	if (sustain < 0.0f)
		sustain = 0.0f;
	if (sustain > 1.0f)
		sustain = 1.0f;
	m_sustain = sustain;
}

float CEnvelope::GetRelease()
{
	return m_release;
}

void CEnvelope::SetRelease(float release)
{
	if (m_release <= 0.0f)
		m_release = 1.0f;
	m_release = release;
}
