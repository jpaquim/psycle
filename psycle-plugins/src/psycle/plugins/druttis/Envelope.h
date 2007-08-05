//////////////////////////////////////////////////////////////////////
//
//	Envelope.h
//
//	druttis@darkface.pp.se
//
//////////////////////////////////////////////////////////////////////
#pragma once
//////////////////////////////////////////////////////////////////////
//
//	Constants
//
//////////////////////////////////////////////////////////////////////
#define ENVELOPE_IDLE 0
#define ENVELOPE_DELAY 1
#define ENVELOPE_ATTACK 2
#define ENVELOPE_DECAY 3
#define ENVELOPE_SUSTAIN 4
#define ENVELOPE_HOLD 5
#define ENVELOPE_RELEASE 6
#define ENVELOPE_MAXTICKS 32767
//////////////////////////////////////////////////////////////////////
//
//	Envelope class
//
//////////////////////////////////////////////////////////////////////
class Envelope
{
	//////////////////////////////////////////////////////////////////
	//	Properties
	//////////////////////////////////////////////////////////////////
private:
	int		m_delay;
	int		m_attack;
	int		m_decay;
	float	m_sustain;
	int		m_length;
	int		m_release;
	//////////////////////////////////////////////////////////////////
	//	Internal variables
	//////////////////////////////////////////////////////////////////
	int		m_state;
	int		m_ticks;
	int		m_decr;
	float	m_value;
	float	m_coeff;
	//////////////////////////////////////////////////////////////////
	//	Constructor / Destructor
	//////////////////////////////////////////////////////////////////
public:
	Envelope()
	{
		m_delay = 0;
		m_attack = 0;
		m_decay = 0;
		m_sustain = 0.0f;
		m_length = 0;
		m_release = 0;
		Reset();
	}
	virtual ~Envelope()
	{
	}
	//////////////////////////////////////////////////////////////////
	//	Get / Set Methods
	//////////////////////////////////////////////////////////////////
	inline int GetDelay()
	{
		return m_delay;
	}

	inline void SetDelay(int delay)
	{
		m_delay = delay;
	}

	inline int GetAttack()
	{
		return m_attack;
	}

	inline void SetAttack(int attack)
	{
		m_attack = attack;
	}

	inline int GetDecay()
	{
		return m_decay;
	}

	inline void SetDecay(int decay)
	{
		m_decay = decay;
	}

	inline float GetSustain()
	{
		return m_sustain;
	}

	inline void SetSustain(float sustain)
	{
		m_sustain = sustain;
	}

	inline int getLength()
	{
		return m_length;
	}

	inline void SetLength(int length)
	{
		m_length = length;
	}

	inline int GetRelease()
	{
		return m_release;
	}

	inline void SetRelease(int release)
	{
		m_release = release;
	}
	//////////////////////////////////////////////////////////////////
	//	Reset
	//	Resets envelope.
	//////////////////////////////////////////////////////////////////
	inline void Reset()
	{
		m_state = ENVELOPE_IDLE;
		m_ticks = ENVELOPE_MAXTICKS;
		m_decr = 0;
		m_value = 0.0f;
		m_coeff = 0.0f;
	}
	//////////////////////////////////////////////////////////////////
	//	Next
	//	Returns next envelope value.
	//////////////////////////////////////////////////////////////////
	inline float Next()
	{
		m_ticks -= m_decr;
		m_value += m_coeff;
		return m_value;
	}
	//////////////////////////////////////////////////////////////////
	//	Start
	//	Starts the envelope from first possible state.
	//	* To restart the envelope, first call Reset()
	//////////////////////////////////////////////////////////////////
	inline void Start()
	{
		if (m_delay > 0)
		{
			if (m_state != ENVELOPE_DELAY)
			{
				m_state = ENVELOPE_DELAY;
				m_ticks = m_delay;
				m_decr = 1;
				m_coeff = 0.0f;
			}
		}
		else if (m_attack > 0)
		{
			if (m_state != ENVELOPE_ATTACK)
			{
				m_state = ENVELOPE_ATTACK;
				m_ticks = m_attack;
				m_decr = 1;
				m_coeff = (1.0f - m_value) / (float) m_ticks;
			}
		}
		else if (m_decay > 0)
		{
			if (m_state != ENVELOPE_DECAY)
			{
				m_state = ENVELOPE_DECAY;
				m_ticks = m_decay;
				m_decr = 1;
				m_value = 1.0f;
				m_coeff = (m_sustain - 1.0f) / (float) m_ticks;
			}
		}
		else if (m_sustain > 0.0f)
		{
			m_value = m_sustain;
			m_coeff = 0.0f;
			if (m_length > 0)
			{
				m_state = ENVELOPE_HOLD;
				m_ticks = m_length;
				m_decr = 1;
			}
			else
			{
				m_state = ENVELOPE_SUSTAIN;
				m_ticks = ENVELOPE_MAXTICKS;
				m_decr = 0;
			}
		}
		else if (m_release > 0)
		{
			if (m_state != ENVELOPE_RELEASE)
			{
				m_state = ENVELOPE_RELEASE;
				m_ticks = m_release;
				m_decr = 1;
				m_coeff = -m_value / (float) m_ticks;
			}
		}
		else
		{
			m_state = ENVELOPE_IDLE;
			m_ticks = ENVELOPE_MAXTICKS;
			m_decr = 0;
			m_value = 0.0f;
			m_coeff = 0.0f;
		}
	}
	//////////////////////////////////////////////////////////////////
	//	Stop
	//	Stops the envelope by setting state ENVELOPE_RELEASE
	//////////////////////////////////////////////////////////////////
	inline void Stop()
	{
		if ((m_state != ENVELOPE_IDLE) && (m_state != ENVELOPE_RELEASE))
		{
			if (m_release > 0)
			{
				m_state = ENVELOPE_RELEASE;
				m_ticks = m_release;
				m_decr = 1;
				m_coeff = -m_value / (float) m_ticks;
			}
			else
			{
				m_state = ENVELOPE_IDLE;
				m_ticks = ENVELOPE_MAXTICKS;
				m_decr = 0;
				m_value = 0.0f;
				m_coeff = 0.0f;
			}
		}
	}
	//////////////////////////////////////////////////////////////////
	//	Clip
	//	Clips number of samples to render.
	//	Also maintains the whole envelope run.
	//////////////////////////////////////////////////////////////////
	inline int Clip(int nsamples)
	{
		if (m_ticks <= 0)
		{
			switch (m_state)
			{
			case ENVELOPE_DELAY :
				if (m_attack > 0)
				{
					m_state = ENVELOPE_ATTACK;
					m_ticks = m_attack;
					m_decr = 1;
					m_coeff = (1.0f - m_value) / (float) m_ticks;
				}
				else if (m_decay > 0)
				{
					m_state = ENVELOPE_DECAY;
					m_ticks = m_decay;
					m_decr = 1;
					m_value = 1.0f;
					m_coeff = (m_sustain - 1.0f) / (float) m_ticks;
				}
				else if (m_sustain > 0.0f)
				{
					m_value = m_sustain;
					m_coeff = 0.0f;
					if (m_length > 0)
					{
						m_state = ENVELOPE_HOLD;
						m_ticks = m_length;
						m_decr = 1;
					}
					else
					{
						m_state = ENVELOPE_SUSTAIN;
						m_ticks = ENVELOPE_MAXTICKS;
						m_decr = 0;
					}
				}
				else
				{
					m_state = ENVELOPE_IDLE;
					m_ticks = ENVELOPE_MAXTICKS;
					m_decr = 0;
					m_value = 0.0f;
					m_coeff = 0.0f;
				}
				break;
			case ENVELOPE_ATTACK :
				if (m_delay > 0)
				{
					m_state = ENVELOPE_DECAY;
					m_ticks = m_decay;
					m_decr = 1;
					m_value = 1.0f;
					m_coeff = (m_sustain - 1.0f) / (float) m_ticks;
				}
				else if (m_sustain > 0.0f)
				{
					m_value = m_sustain;
					m_coeff = 0.0f;
					if (m_length > 0)
					{
						m_state = ENVELOPE_HOLD;
						m_ticks = m_length;
						m_decr = 1;
					}
					else
					{
						m_state = ENVELOPE_SUSTAIN;
						m_ticks = ENVELOPE_MAXTICKS;
						m_decr = 0;
					}
				}
				else
				{
					m_state = ENVELOPE_IDLE;
					m_ticks = ENVELOPE_MAXTICKS;
					m_decr = 0;
					m_value = 0.0f;
					m_coeff = 0.0f;
				}
				break;
			case ENVELOPE_DECAY :
				if (m_sustain > 0.0f)
				{
					m_value = m_sustain;
					m_coeff = 0.0f;
					if (m_length > 0)
					{
						m_state = ENVELOPE_HOLD;
						m_ticks = m_length;
						m_decr = 1;
					}
					else
					{
						m_state = ENVELOPE_SUSTAIN;
						m_ticks = ENVELOPE_MAXTICKS;
						m_decr = 0;
					}
				}
				else
				{
					m_state = ENVELOPE_IDLE;
					m_ticks = ENVELOPE_MAXTICKS;
					m_decr = 0;
					m_value = 0.0f;
					m_coeff = 0.0f;
				}
				break;
			case ENVELOPE_HOLD :
				if (m_release > 0)
				{
					m_state = ENVELOPE_RELEASE;
					m_ticks = m_release;
					m_decr = 1;
					m_coeff = -m_value / (float) m_ticks;
				}
				else
				{
					m_state = ENVELOPE_IDLE;
					m_ticks = ENVELOPE_MAXTICKS;
					m_decr = 0;
					m_value = 0.0f;
					m_coeff = 0.0f;
				}
				break;
			case ENVELOPE_RELEASE :
				m_state = ENVELOPE_IDLE;
				m_ticks = ENVELOPE_MAXTICKS;
				m_decr = 0;
				m_value = 0.0f;
				m_coeff = 0.0f;
				break;
			}
		}
		return (nsamples < m_ticks ? nsamples : m_ticks);
	}
	//////////////////////////////////////////////////////////////////
	//	IsIdle
	//////////////////////////////////////////////////////////////////
	inline bool IsIdle()
	{
		return m_state == ENVELOPE_IDLE;
	}
	//////////////////////////////////////////////////////////////////
	//	Render
	//	Renders a buffer with an amount of samples
	//////////////////////////////////////////////////////////////////
	inline void Render(float *pbuf, int nsamples)
	{
		--pbuf;
		int amt;
		do
		{
			amt = Clip(nsamples);
			nsamples -= amt;
			do
				*++pbuf = Next();
			while (--amt);
		}
		while (nsamples);
	}
};
