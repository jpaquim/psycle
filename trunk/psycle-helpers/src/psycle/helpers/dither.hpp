///\interface psycle::helpers::dsp::Dither

#pragma once

#include <psycle/helpers/mersennetwister.hpp>
namespace psycle { namespace helpers { namespace dsp {

class Dither {
	public:
		Dither();
		virtual ~Dither() {}

		void Process(float * inSamps, unsigned int length);

		struct Pdf {
			enum type {
				triangular = 0,
				rectangular,
				gaussian
			};
		};
		
		struct NoiseShape {
			enum type {
				none = 0,
				highpass
			};
		};

		void SetBitDepth(unsigned int newdepth) { bitdepth = newdepth; }
		void SetPdf(Pdf::type newpdf) { pdf = newpdf; }
		void SetNoiseShaping(NoiseShape::type newns) { noiseshape = newns; }

	private:
		unsigned int bitdepth;
		Pdf::type pdf;
		NoiseShape::type noiseshape;

		MersenneTwister mt;
};

}}}
