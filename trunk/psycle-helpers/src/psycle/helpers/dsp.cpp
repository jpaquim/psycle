///\implementation psycle::helpers::dsp::Cubic.
#include "dsp.hpp"
namespace psycle { namespace helpers { namespace dsp {

	int numRMSSamples = 1;

	int cubic_resampler::resolution_;
	float cubic_resampler::a_table_[CUBIC_RESOLUTION];
	float cubic_resampler::b_table_[CUBIC_RESOLUTION];
	float cubic_resampler::c_table_[CUBIC_RESOLUTION];
	float cubic_resampler::d_table_[CUBIC_RESOLUTION];
	float cubic_resampler::l_table_[CUBIC_RESOLUTION];

	float cubic_resampler::sinc_table_[SINC_TABLESIZE];
	float cubic_resampler::sinc_delta_[SINC_TABLESIZE];

	cubic_resampler::cubic_resampler() {
		resolution_ = CUBIC_RESOLUTION;
		// Initialize tables
		for(int i = 0; i < resolution_; ++i) {
			double x = (double)i / (double)resolution_;
			a_table_[i] = float(-0.5 * x * x * x +       x * x - 0.5 * x);
			b_table_[i] = float( 1.5 * x * x * x - 2.5 * x * x           + 1.0);
			c_table_[i] = float(-1.5 * x * x * x + 2.0 * x * x + 0.5 * x);
			d_table_[i] = float( 0.5 * x * x * x - 0.5 * x * x);
			l_table_[i] = x;
		}

		int sincSize = SINC_TABLESIZE;
		double pi = 3.14159265358979323846;

		// one-sided-- the function is symmetrical, one wing of the sinc will suffice.
		sinc_table_[0] = 1; // save the trouble of evaluating 0/0
		for(int i(1); i < sincSize; ++i) {
			//sinc runs at half speed of SINC_RESOLUTION (i.e. two zero crossing points per period).
			sinc_table_[i] = std::sin(i * pi / (float)SINC_RESOLUTION) / float(i * pi / (float)SINC_RESOLUTION); // equivalent to i * pi * SINC_ZEROS / sincSize

			///\todo decide which window we like best.
			///\todo kaiser windows might be our best option, but i have no clue how to calculate one :)

			// we also only apply half window (from pi..2pi instead of 0..2pi, going half speed) because of the way the sinc is generated.
			#if 1
				// blackman window
				  sinc_table_[i] *= 0.42659f - 0.49656f * std::cos(pi+ pi * i/ ((float)sincSize -1.f)) + 0.076849f * std::cos(2.f * pi * i / ((float)sincSize-1.f));
			#elif 0
				// hann(ing) window
				sinc_table_[i] *= .5f * (1.f - std::cos(pi + pi * i / (float)sincSize));
			#elif 0
				// hamming window
				sinc_table_[i] *= 0.53836f - 0.46164f * std::cos(pi +  pi * i / (float)sincSize);
			#endif
		}
		for(int i(0); i < sincSize - 1; ++i) sinc_delta_[i] = sinc_table_[i + 1] - sinc_table_[i];
		sinc_delta_[sincSize - 1] = 0 - sinc_table_[sincSize - 1];
	}
}}}
