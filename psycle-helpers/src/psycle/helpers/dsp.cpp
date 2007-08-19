///\implementation psycle::helpers::dsp::Cubic.
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include "dsp.hpp"
namespace psycle { namespace helpers { namespace dsp {
	#if defined PSYCLE__CONFIGURATION__RMS_VUS
		int numRMSSamples=1;
		int countRMSSamples=0;
		double RMSAccumulatedLeft=0;
		double RMSAccumulatedRight=0;
		float previousRMSLeft=0;
		float previousRMSRight=0;
	#endif

	int Cubic::_resolution;
	float Cubic::_aTable[CUBIC_RESOLUTION];
	float Cubic::_bTable[CUBIC_RESOLUTION];
	float Cubic::_cTable[CUBIC_RESOLUTION];
	float Cubic::_dTable[CUBIC_RESOLUTION];
	float Cubic::_lTable[CUBIC_RESOLUTION];

	float Cubic::sincTable[SINC_TABLESIZE];
	float Cubic::sincDelta[SINC_TABLESIZE];

	Cubic::Cubic()
	{
		_resolution = CUBIC_RESOLUTION;
		// Initialize tables
		for(int i=0; i<_resolution; i++)
		{
			float x = (float)i/(float)_resolution;
			_aTable[i] = float(-0.5*x*x*x +     x*x - 0.5*x);
			_bTable[i] = float( 1.5*x*x*x - 2.5*x*x         + 1);
			_cTable[i] = float(-1.5*x*x*x + 2.0*x*x + 0.5*x);
			_dTable[i] = float( 0.5*x*x*x - 0.5*x*x);
			_lTable[i] = x;
		}

		int sincSize = SINC_TABLESIZE;
		double pi = 3.14159265358979323846;

		// one-sided-- the function is symmetrical, one wing of the sinc will suffice.
		sincTable[0] = 1; // save the trouble of evaluating 0/0
		for(int i(1); i<sincSize; ++i)
		{
			sincTable[i] = sin(i * pi / (float)SINC_RESOLUTION) / float(i * pi / (float)SINC_RESOLUTION); //equivalent to i * pi * SINC_ZEROS / sincSize

			///\todo decide which window we like best.
			///\todo kaiser windows might be our best option, but i have no clue how to calculate one :)

			#if 1
				// blackman window
				sincTable[i] *= 0.42f - 0.5f * cos(2*pi*i/(float)sincSize*2 + pi) + 0.08f * cos(4*pi*i/(float)sincSize*2 + 2*pi);
			#elif 0
				// hann(ing) window
				sincTable[i] *= .5f * (1 - cos(2*pi*i/(float)sincSize*2 + pi));
			#else
				// hamming window
				sincTable[i] *= 0.53836f - 0.46164f * cos(2*pi*i/(float)sincSize*2 + pi);
			#endif
		}
		for(int i(0); i<sincSize-1; ++i) sincDelta[i] = sincTable[i+1]-sincTable[i];
		sincDelta[sincSize-1] = 0 - sincTable[sincSize-1];
	}
}}}
