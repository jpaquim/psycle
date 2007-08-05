//============================================================================
//
//	CEnvelope.h
//
//============================================================================
#pragma once
//============================================================================
//	Defines
//============================================================================
#define ENVELOPE_DONE		0
#define ENVELOPE_ATTACK		1
#define ENVELOPE_DECAY		2
#define ENVELOPE_SUSTAIN	3
#define ENVELOPE_RELEASE	4
//============================================================================
//	Class
//============================================================================
class CEnvelope
{
private:
	float	m_attack;
	float	m_decay;
	float	m_sustain;
	float	m_release;
	int		m_stage;
	float	m_value;
	float	m_coeff;
public:
	CEnvelope();
	~CEnvelope();
	virtual bool IsFinished();
	void Begin();
	void Stop();
	void Kill();
	float GetAttack();
	bool SetAttack(float attack);
	float GetDecay();
	bool SetDecay(float decay);
	float GetSustain();
	bool SetSustain(float sustain);
	float GetRelease();
	bool SetRelease(float release);
	inline float Next()
	{
		switch (m_stage)
		{
			case ENVELOPE_ATTACK:
				m_value += m_coeff;
				if (m_value >= 1.0f) {
					if (m_decay > 0.0f) {
						m_stage = ENVELOPE_DECAY;
						m_value = 1.0f;
						m_coeff = (1.0f - m_sustain) / m_decay;
					} else if (m_sustain > 0.0f) {
						m_stage = ENVELOPE_SUSTAIN;
						m_value = m_sustain;
						m_coeff = 0.0f;
					} else {
						m_stage = ENVELOPE_DONE;
						m_value = 0.0f;
						m_coeff = 0.0f;
					}
				}
				break;

			case ENVELOPE_DECAY:
				m_value -= m_coeff;
				if (m_value <= m_sustain) {
					m_stage = ENVELOPE_SUSTAIN;
					m_value = m_sustain;
					m_coeff = 0.0f;
				}
				break;

			case ENVELOPE_SUSTAIN:
				m_value = m_sustain;
				break;

			case ENVELOPE_RELEASE:
				m_value -= m_coeff;
				if (m_value <= 0.0f) {
					m_stage = ENVELOPE_DONE;
					m_value = 0.0f;
					m_coeff = 0.0f;
				}
				break;

			default:
				m_value = 0.0f;
				break;

		}
		return m_value;
	}
};
