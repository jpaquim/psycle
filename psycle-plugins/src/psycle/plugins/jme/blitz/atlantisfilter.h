// CSIDFilter by Jeremy Evers - ripped from Atlantis which is available at 
// http:://jeremyevers.com

#define PIf	3.1415926535897932384626433832795f;

#pragma once
enum eAlgorithm {
	FILTER_ALGO_SID_LPF = 0,
	FILTER_ALGO_SID_HPF,
	FILTER_ALGO_SID_BPF,
	FILTER_ALGO_SID_LPF_HPF,
	FILTER_ALGO_SID_LPF_BPF,
	FILTER_ALGO_SID_LPF_HPF_BPF,
	FILTER_ALGO_SID_HPF_BPF
};

class CSIDFilter {
public:
	CSIDFilter();

	void setAlgorithm(eAlgorithm a_algo);
	void reset();
	void process(float& sample);
	void recalculateCoeffs(const float a_fFrequency, const float a_fFeedback);
private:

	eAlgorithm m_Algorithm;

	// pre-calculated coefficients
	float m_f, m_fb;

	// filter data
	float m_low, m_high, m_band;
};

