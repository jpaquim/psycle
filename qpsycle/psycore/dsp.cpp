//\file
///\brief implementation file for psycle::host::Cubic.
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


#include "dsp.h"
namespace psycle
{
	namespace host
	{
		namespace dsp
		{
			// GNU Radio's code
			//#define IzeroEPSILON 1E-21               /* Max error acceptable in Izero */
			//static double Izero(double x)
			//{
			//	double sum, u, halfx, temp;
			//	int n;

			//	sum = u = n = 1;
			//	halfx = x/2.0;
			//	do {
			//		temp = halfx/(double)n;
			//		n += 1;
			//		temp *= temp;
			//		u *= temp;
			//		sum += u;
			//	} while (u >= IzeroEPSILON*sum);
			//	return(sum);
			//}

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

				//one-sided-- the function is symmetrical, one wing of the sinc will suffice.
				sincTable[0] = 1;	//save the trouble of evaluating 0/0
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
		}
	}
}
