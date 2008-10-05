// CSIDFilter by Jeremy Evers - ripped from Atlantis which is available at 
// http:://jeremyevers.com

#include <packageneric/pre-compiled.private.hpp>
#include "atlantisfilter.h"
#define piX2		 6.283185307179586476925286766559f
#define ANTIDENORMAL 1e-15f // could be 1e-18f, but this is still way below audible noise threshold


CSIDFilter::CSIDFilter()
{
	m_f = m_fb = 0;
	reset();
};


void CSIDFilter::reset()
{
	m_low=m_high=m_band=0;
};

void CSIDFilter::setAlgorithm(eAlgorithm a_algo)
{
	m_Algorithm = a_algo;
}

void CSIDFilter::recalculateCoeffs(const float a_fFrequency, const float a_fFeedback)
{
	//m_f = (200.0f+(17800.0f*6*a_fFrequency))*44100/sampleRate*2*PIf/985248.0f;
	m_f =(1.0f+(534.0f*a_fFrequency))*0.00127545253726566f; 
	float f2 = 2.0f*(a_fFeedback-(a_fFrequency*a_fFrequency)); 
	if (f2 < 0.0f) f2 = 0.0f;
	m_fb = 1.0f/(0.707f+f2);
}

void CSIDFilter::process(float& sample)
{
	const float f = m_f;
	const float fb = m_fb;
	float low = m_low;
	float band = m_band;
	float high = m_high;

	low -= (f*band);
	band -= (f*high);
	high = (band*fb) - low - sample + ANTIDENORMAL;

	switch (m_Algorithm)
	{
		case FILTER_ALGO_SID_LPF:
		{
			sample = low;
			break;
		}
		case FILTER_ALGO_SID_HPF:
		{
			sample = high;
			break;
		}
		case FILTER_ALGO_SID_BPF:
		{
			sample = band;
			break;
		}
		case FILTER_ALGO_SID_LPF_HPF:
		{
			sample = low + high;
			break;
		}
		case FILTER_ALGO_SID_LPF_BPF:
		{
			sample = low + band;
			break;
		}
		case FILTER_ALGO_SID_LPF_HPF_BPF:
		{
			sample = low + band + high;
			break;
		}
		case FILTER_ALGO_SID_HPF_BPF:
		{
			sample = band + high;
			break;
		}
		default:
			break;
	}
	m_low = low;
	m_band = band;
	m_high = high;
}
