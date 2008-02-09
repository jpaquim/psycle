/* -*- mode:c++, indent-tabs-mode:t -*- */
///\file
///\brief interface file for psy::core::dsp::Dither

#include "mersennetwister.h"

namespace psy
{
	namespace core
	{
		namespace dsp
		{

			class Dither
			{
			public:
				Dither();
				virtual ~Dither() {}

				void Process(float* inSamps, int length);

				enum Pdf
				{
					triangular = 0,
					rectangular,
					gaussian
				};
				enum NoiseShape
				{
					none = 0,
					highpass
				};

				void SetBitDepth(int newdepth) { bitdepth = newdepth; }
				void SetPdf(Pdf newpdf) { pdf = newpdf; }
				void SetNoiseShaping(NoiseShape newns) { noiseshape = newns; }

			private:
				int bitdepth;
				Pdf pdf;
				NoiseShape noiseshape;

				MersenneTwister mt;
			};
		}
	}
}
