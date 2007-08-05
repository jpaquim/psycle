//////////////////////////////////////////////////////////////////////
//
//	Inertia.h
//
//	druttis@darkface.pp.se
//
//////////////////////////////////////////////////////////////////////
#pragma once
//////////////////////////////////////////////////////////////////////
//
//	Inertia Constants
//
//////////////////////////////////////////////////////////////////////
#define INERTIA_MAXTICKS 32767
//////////////////////////////////////////////////////////////////////
//
//	Inertia Class
//
//////////////////////////////////////////////////////////////////////
class Inertia
{
	//////////////////////////////////////////////////////////////////
	//	Properties
	//////////////////////////////////////////////////////////////////
private:
	int			m_length;
	float		m_target;
	//////////////////////////////////////////////////////////////////
	//	Internal variables
	//////////////////////////////////////////////////////////////////
	int			m_ticks;
	int			m_decr;
	float		m_value;
	float		m_step;
	bool		m_valid;
	//////////////////////////////////////////////////////////////////
	//	Constructor & Destructor
	//////////////////////////////////////////////////////////////////
public:
	Inertia();
	~Inertia();
	//////////////////////////////////////////////////////////////////
	//	Get / Set Methods
	//////////////////////////////////////////////////////////////////
	int GetLength();
	void SetLength(int length);
	float GetTarget();
	void SetTarget(float target);
	float GetValue();
private:
	void Update();
	//////////////////////////////////////////////////////////////////
	//	Methods
	//////////////////////////////////////////////////////////////////
public:
	//////////////////////////////////////////////////////////////////
	//	IsValid
	//////////////////////////////////////////////////////////////////
	inline bool IsValid()
	{
		return m_valid;
	}
	//////////////////////////////////////////////////////////////////
	//	Invalidate
	//////////////////////////////////////////////////////////////////
	inline void Invalidate()
	{
		m_valid = false;
	}
	//////////////////////////////////////////////////////////////////
	//	IsIdle
	//////////////////////////////////////////////////////////////////
	inline bool IsIdle()
	{
		return IsValid() && m_decr == 0;
	}
	//////////////////////////////////////////////////////////////////
	//	Reset
	//////////////////////////////////////////////////////////////////
	inline void Reset()
	{
		Stop();
		m_value = m_target;
	}
	//////////////////////////////////////////////////////////////////
	//	Stop
	//////////////////////////////////////////////////////////////////
	inline void Stop()
	{
		m_ticks = INERTIA_MAXTICKS;
		m_decr = 0;
		m_step = 0.0f;
	}
	//////////////////////////////////////////////////////////////////
	//	Clip
	//////////////////////////////////////////////////////////////////
	inline int Clip(int nsamples)
	{
		m_valid = true;
		if (m_ticks <= 0) Reset();
		return nsamples < m_ticks ? nsamples : m_ticks;
	}
	//////////////////////////////////////////////////////////////////
	//	Next
	//////////////////////////////////////////////////////////////////
	inline float Next()
	{
		const float out = m_value;
		m_ticks -= m_decr;
		m_value += m_step;
		return out;
	}
	//////////////////////////////////////////////////////////////////
	//	Fill
	//////////////////////////////////////////////////////////////////
	void Fill(float *pout, int nsamples);
	//////////////////////////////////////////////////////////////////
	//	Fill & Mult
	//////////////////////////////////////////////////////////////////
	void Fill(float *pbuf, float mul, int nsamples);
};
