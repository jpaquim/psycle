///\file
///\brief implementation file for psycle::host::dsp::Dither

#include <psycle/project.private.hpp>
#include "dither.hpp"

namespace psycle
{
	namespace host
	{
		namespace dsp
		{
			Dither::Dither() : bitdepth(16), pdf(triangular), noiseshape(none)
			{
				//copied verbatim from mt19937 demo
				unsigned long init[4]={0x123, 0x234, 0x345, 0x456}, length=4;
				mt.init_by_array(init, length);
			}


			void Dither::Process(float *inSamps, int length)
			{
				bool newgauss(true);				//gaussian rand returns two values, this tells us which we're on
				double gauss(0.0), randval(0.0);	//our random number, and an extra variable for the gaussian distribution
				float prevError(0.0f);				//quantization error of the last sample, used for noise shaping
				float bdMultiplier;				//the amount the sample will eventually be multiplied by for integer quantization
				float bdQ;						//the inverse.. i.e., the number that will eventually become the quantization interval

				switch (bitdepth)
				{
				case 8:
					bdMultiplier = 1/256.0f;
					bdQ = 256.0f;
					break;
				default:
				case 16:
					bdMultiplier = 1.0f;
					bdQ = 1.0f;
					break;
				case 24:
					bdMultiplier = 256.0f;
					bdQ = 1/256.0f;
					break;
				case 32:
					bdMultiplier = 65536.0f;
					bdQ = 1/65536.0f;
					break;
				}

				for(int i(0);i<length;++i)
				{
					switch(pdf)
					{
					case rectangular:
						randval = mt.genrand_real1()-0.5;
						break;
					case gaussian:
						if(newgauss)
							mt.genrand_gaussian(randval, gauss);
						else
							randval=gauss;
						newgauss = !newgauss;	//mt.genrand_gaussian() has a standard deviation (rms) of 1..
						randval*=0.5;			//we need it to be one-half the quantizing interval (which is 1), so we just halve it
						break;	
					case triangular:
						randval = (mt.genrand_real1()-0.5) + (mt.genrand_real1()-0.5);
						break;
					}

					*(inSamps+i) += randval*bdQ;

					//todo: this seems inefficient.. we're essentially quantizing twice, once to test the error, and again
					//		for real when we write to the wave file.
					if(noiseshape==highpass)
					{
						*(inSamps+i) += prevError;
						//the only way to determine the quantization error of rounding to int after scaling by a given factor
						//is to do the actual scaling, and then divide the resulting error by the same factor to keep it in
						//the correct scale (until it's re-multiplied on wave-writing (this is pretty ridiculous))
						prevError = (*(inSamps+i)*bdMultiplier - (int)(*(inSamps+i)*bdMultiplier) ) * bdQ;
					}

				}
			}


		}
	}
}