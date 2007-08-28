//\file
///\brief implementation file for psy::core::Cubic.
//#include <packageneric/pre-compiled.private.hpp>
//#include PACKAGENERIC

// code for kaiser window is taken from GNU Radio

/* -*- c++ -*- */
/*
* Copyright 2002 Free Software Foundation, Inc.
*
* This file is part of GNU Radio
*
* GNU Radio is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2, or (at your option)
* any later version.
*
* GNU Radio is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with GNU Radio; see the file COPYING.  If not, write to
* the Free Software Foundation, Inc., 51 Franklin Street,
* Boston, MA 02110-1301, USA.
*/
#include "psycleCorePch.hpp"

#include "dsp.h"

namespace psy
{
	namespace core
	{
		namespace dsp
		{
			#if 0
			// GNU Radio's code
			/// Max error acceptable in Izero
			double const IzeroEPSILON = 1e-21
			static double Izero(double x)
			{
				double sum, u, halfx, temp;
				int n;
				sum = u = n = 1;
				halfx = x/2.0;
				do {
					temp = halfx/(double)n;
					n += 1;
					temp *= temp;
					u *= temp;
					sum += u;
				} while (u >= IzeroEPSILON*sum);
				return(sum);
			}
			#endif

		#if PSYCLE__CONFIGURATION__RMS_VUS
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

				// kaiser window

				////double alpha = 10.0; //todo: what value is the best? 
				//double beta = 6.76; //todo: must be pi * alpha
				//double IBeta = 1.0/Izero(beta);
				//double inm1 = 1.0/((double)(sincSize*2)); //+pi;
				//double temp;

				// one-sided-- the function is symmetrical, one wing of the sinc will suffice.
				sincTable[0] = 1; // save the trouble of evaluating 0/0
				for(int i(1); i<sincSize; ++i)
				{
					sincTable[i] = sin(i * pi / (float)SINC_RESOLUTION) / float(i * pi / (float)SINC_RESOLUTION);

					//todo: decide which window we like best.
					//also todo: kaiser windows might be our best option, but i have no clue how to calculate one :)
					
					//kaiser window
					//temp = i * inm1;
					//sincTable[i] *= float(Izero(beta*sqrt(1.0-temp*temp)) * IBeta);

					//blackman window
					sincTable[i] *= 0.42f - 0.5f * cos(2*pi*i/(float)sincSize*2 + pi) + 0.08f * cos(4*pi*i/(float)sincSize*2 + 2*pi);

					//hann(ing) window
					//sincTable[i] *= .5f * (1 - cos(2*pi*i/(float)sincSize*2 + pi));

					//hamming window
					//sincTable[i] *= 0.53836f - 0.46164f * cos(2*pi*i/(float)sincSize*2 + pi);
				}
				
				for(int i(0); i<sincSize-1; ++i)
				{
					sincDelta[i] = sincTable[i+1]-sincTable[i];
				}
				sincDelta[sincSize-1] = 0 - sincTable[sincSize-1];

			}

			float Resampler::T_None(const short *pData, std::uint64_t offset, std::uint32_t res, std::uint64_t length)
			{
				return *pData;
			}

			void Cubic::SetQuality(ResamplerQuality quality)
			{
				_quality = quality;
				switch (quality)
				{
					case R_NONE:
					_pWorkFn = T_None;
					break;
				case R_LINEAR:
					_pWorkFn = Linear;
					break;
				case R_SPLINE:
					_pWorkFn = Spline;
					break;
				case R_BANDLIM:
					_pWorkFn = Bandlimit;
					break;
				}
			}
			ResamplerQuality Cubic::GetQuality(void) const { return _quality; }


			/// interpolation work function which does linear interpolation.
			float Cubic::Linear(const short *pData, std::uint64_t offset, std::uint32_t res, std::uint64_t length)
			{
				float y0,y1;
				y0 = *pData;
				y1 = static_cast<float>( ( offset+1 == length )?0:*(pData+1) );
				return (y0+(y1-y0)*_lTable[res>>21]);
			}
			/// interpolation work function which does spline interpolation.
			float Cubic::Spline(const short *pData, std::uint64_t offset, std::uint32_t res, std::uint64_t length)
			{
				float yo, y0,y1, y2;
				res = res >> 21;
			
				yo = static_cast<float>( (offset==0)?0:*(pData-1) );
				y0=*(pData);
				y1= static_cast<float>( (offset+1 == length)?0:*(pData+1) );
				y2= static_cast<float>( (offset+2 == length)?0:*(pData+2) );
				return (_aTable[res]*yo+_bTable[res]*y0+_cTable[res]*y1+_dTable[res]*y2);
			}
			
			// yo = y[-1] [sample at x-1]
			// y0 = y[0]  [sample at x (input)]
			// y1 = y[1]  [sample at x+1]
			// y2 = y[2]  [sample at x+2]
			
			// res= distance between two neighboughing sample points [y0 and y1] 
			// ,so [0...1.0]. You have to multiply this distance * RESOLUTION used
			// on the spline conversion table. [2048 by default]
			// If you are using 2048 is asumed you are using 12 bit decimal
			// fixed point offsets for resampling.
			
			// offset = sample offset [info to avoid go out of bounds on sample reading ]
			// length = sample length [info to avoid go out of bounds on sample reading ]

			/// interpolation work function which does band-limited interpolation.
			float Cubic::Bandlimit(const short *pData, std::uint64_t offset, std::uint32_t res, std::uint64_t length)
			{
				res = res >> 23; // !!! assumes SINC_RESOLUTION == 512 !!!
				int leftExtent(SINC_ZEROS), rightExtent(SINC_ZEROS);
				if(offset<SINC_ZEROS) leftExtent=(int)(offset);
				if(length-offset<SINC_ZEROS) rightExtent=(int)(length-offset);
				
				const int sincInc(SINC_RESOLUTION);
				float newval(0.0);

				newval += sincTable[res] * *(pData);
				///\todo: Will weight be different than zero using the current code?
				float sincIndex(sincInc+res);
				float weight(sincIndex - floor(sincIndex));
				for
				(
					int i(1);
					i < leftExtent;
					++i, sincIndex+=sincInc
				)
					newval+= (sincTable[(int)sincIndex] + sincDelta[(int)sincIndex]*weight ) * *(pData-i);

				sincIndex = sincInc-res;
				weight = sincIndex - floor(sincIndex);
				for
				(
					int i(1);
					i < rightExtent;
					++i, sincIndex += sincInc
				)
					newval += ( sincTable[(int)sincIndex] + sincDelta[(int)sincIndex]*weight ) * *(pData+i);

				return newval;
			}

		}
	}
}
