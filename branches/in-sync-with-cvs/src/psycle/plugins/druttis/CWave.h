//============================================================================
//
//	CWave.h
//
//============================================================================
#pragma once

class CWave
{
private:
	char	*m_pname;
	float	*m_psamples;
	int		m_nsamples;
	int		m_mask;
public:
	CWave();
	~CWave();
	bool Init(char *pname, float *psamples, int nsamples);
	char *GetName();
	int GetNumberOfSamples();
	//------------------------------------------------------------------------
	//	Quick way of float to int
	//------------------------------------------------------------------------
	static inline int f2i(double d)
	{
		const double magic = 6755399441055744.0;
		double tmp = (d - 0.5) + magic;
		return *(int*) &tmp;
	};
	//------------------------------------------------------------------------
	//	Get one sample
	//------------------------------------------------------------------------
	inline float GetSample(float phase)
	{
		return m_psamples[f2i(phase) & m_mask];
	}
	//------------------------------------------------------------------------
	//	Advance
	//------------------------------------------------------------------------
	inline float Advance(float phase, float speed)
	{
		phase = phase + speed;
		while (phase >= m_nsamples)
			phase -= m_nsamples;
		return phase;
	}
	//------------------------------------------------------------------------
	//	CopySamplesInto
	//------------------------------------------------------------------------
	inline float CopySamplesInto(float phase, float speed, float *buf, int nsamples)
	{
		--buf;
		do {
			*++buf = m_psamples[f2i(phase) & m_mask];
			phase += speed;
		} while (--nsamples);
		while (phase >= m_nsamples)
			phase -= m_nsamples;
		return phase;
	}
};
