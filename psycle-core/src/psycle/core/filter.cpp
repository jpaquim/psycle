// -*- mode:c++; indent-tabs-mode:t -*-
///\file
///\brief implementation file for psy::core::Filter. based on Revision 2623


#include "filter.h"
#include <cmath>
#include <algorithm>

namespace psy
{
	namespace core
	{
		namespace dsp
		{
			FilterCoeff FilterCoeff::singleton;

			FilterCoeff::FilterCoeff() {
				samplerate = -1;
				//table is initialized with Filter::Init()
			}

			void FilterCoeff::setSampleRate(float samplerate)
			{
				if (samplerate != this->samplerate)
				{
					this->samplerate = samplerate;
					for (int r=0; r<5; r++)
					{
						for (int f=0; f<128; f++)
						{
							for (int q=0; q<128; q++)
							{
								ComputeCoeffs(f, q, r);
								_coeffs[r][f][q][0] = (float)_coeff[0];
								_coeffs[r][f][q][1] = (float)_coeff[1];
								_coeffs[r][f][q][2] = (float)_coeff[2];
								_coeffs[r][f][q][3] = (float)_coeff[3];
								_coeffs[r][f][q][4] = (float)_coeff[4];
							}
						}
					}
				}
			}

			float FilterCoeff::Cutoff(int v)
			{
				return float(pow( (v+5)/(127.0+5), 1.7)*13000+30);
			}
			
			float FilterCoeff::Resonance(float v)
			{
				return float(pow( v/127.0, 4)*150+0.1);
			}
			
			float FilterCoeff::Bandwidth(int v)
			{
				return float(pow( v/127.0, 4)*4+0.1);
			}

			void FilterCoeff::ComputeCoeffs(int freq, int r, int t)
			{
				float omega = float(TPI*Cutoff(freq)/ samplerate);
				float sn = (float)sin(omega);
				float cs = (float)cos(omega);
				float alpha;
				if (t < 2) alpha = float(sn / Resonance( r *(freq+70)/(127.0f+70)));
				else alpha = float(sn * sinh( Bandwidth( r) * omega/sn));
				float a0, a1, a2, b0, b1, b2;
				switch (t)
				{
				case F_LOWPASS12:
					b0 =  (1 - cs)/2;
					b1 =   1 - cs;
					b2 =  (1 - cs)/2;
					a0 =   1 + alpha;
					a1 =  -2*cs;
					a2 =   1 - alpha;
					break;
				case F_HIGHPASS12:
					b0 =  (1 + cs)/2;
					b1 = -(1 + cs);
					b2 =  (1 + cs)/2;
					a0 =   1 + alpha;
					a1 =  -2*cs;
					a2 =   1 - alpha;
					break;
				case F_BANDPASS12:
					b0 =   alpha;
					b1 =   0;
					b2 =  -alpha;
					a0 =   1 + alpha;
					a1 =  -2*cs;
					a2 =   1 - alpha;
					break;
				case F_BANDREJECT12:
					b0 =   1;
					b1 =  -2*cs;
					b2 =   1;
					a0 =   1 + alpha;
					a1 =  -2*cs;
					a2 =   1 - alpha;
					break;
				default:
					b0 =  0;
					b1 =  0;
					b2 =  0;
					a0 =  0;
					a1 =  0;
					a2 =  0;
				}
				_coeff[0] = b0/a0;
				_coeff[1] = b1/a0;
				_coeff[2] = b2/a0;
				_coeff[3] = -a1/a0;
				_coeff[4] = -a2/a0;
			}

			Filter::Filter()
			{
				//Filtercoef is automatially initialized.
				//FilterCoeff::singleton.Init();
				Init(44100);
				_x1 = _x2 = _y1 = _y2 = 0;
				_a1 = _a2 = _b1 = _b2 = 0;
			}
			
			void Filter::Init(int sampleRate)
			{
				_cutoff=127;
				_q=0;
				_type = F_LOWPASS12;
				FilterCoeff::singleton.setSampleRate(sampleRate);
				Update();
			}

			void Filter::Update()
			{
				_coeff0 = FilterCoeff::singleton._coeffs[_type][_cutoff][_q][0];
				_coeff1 = FilterCoeff::singleton._coeffs[_type][_cutoff][_q][1];
				_coeff2 = FilterCoeff::singleton._coeffs[_type][_cutoff][_q][2];
				_coeff3 = FilterCoeff::singleton._coeffs[_type][_cutoff][_q][3];
				_coeff4 = FilterCoeff::singleton._coeffs[_type][_cutoff][_q][4];
			}

			void ITFilter::Update()
			{
				#define PI 3.1415926535897932384626433832795
				double fc;
				if ( iRes >0 || iCutoff < 127 )
				{
					//double fc = 440.0*pow(2.0, (iCutoff - 69.0)/12.0); // suggested
					//double fc = 440.0*pow(2.0, (iCutoff - 57.0)/12.0); // suggested, shifted one octave.
					//double fc = 440.0*pow(2.0, (iCutoff - 57.0)/16.0); // variation in 16th's, not in octaves..
					fc = 110.0 * pow(2.0,(iCutoff+6)/24.0); // ModPlug.
					//double fc = 100.0* (pow(2.0,(iCutoff+10)/24.0));
				}
				else { fc = iSampleRate/2.0; }
				// Attempt at Self-filtered samples. It filters too much.
				//else { fc = iSampleCurrentSpeed/2.0; }

				const double frequ = 2.0*sin(PI*std::min(0.25, fc/(iSampleRate*2)));  // the fs*2 is because it's double sampled
				//fCoeff[damp]  = min(2.0*(1.0 - pow(iRes*0.007874, 0.25)), min(2.0, 2.0/frequ - frequ*0.5));// original.
				fCoeff[damp]  = std::min(exp(iRes/128.0*(-LN10*1.2)), std::min(2.0, 2.0/frequ - frequ*0.5));// exp instead of 1-pow.
				//fCoeff[damp]  = min(2.0*(1.0 - pow(iRes*0.0072, 0.25)), min(2.0, 2.0/frequ - frequ*0.5)); // lowered Q a bit.
				fCoeff[freq] = frequ;

				#undef PI
			}
			void ITFilter::UpdateOld()
			{
				double d,e;
				if ( iRes >0 || iCutoff < 127 )
				{
					#if 0
					#define PI 3.1415926535897932384626433832795
					pole[0]=pole[1]=pole[2]=pole[3]=0;
					float fSampleRate = (float)iSampleRate;
					float fCutoff = fSampleRate* (pow(2.0f,iCutoff/127.0f)-1.0f)/2.0f;
						
					fCoeff[1] =
						((fSampleRate) - (fCutoff * TPI)) /
						((fSampleRate) + (fCutoff * TPI));
					fCoeff[0] =
					fCoeff[2] =
						(fCutoff * TPI) /
						((fCutoff * TPI) + ( fSampleRate));
					#endif
	
					#if 0
						fc *= (float)(2.0*3.14159265358/fs);
						float dmpfac = pow(10.0f, -((24.0f / 128.0f)*(float)pChn->nResonance) / 20.0f);
					#endif
						
					// sample_freq*pow(0.5, 0.25 + cutoff*1/factor)*1/(2*pi*110.0)))
					// loss = exp(resonance*(-LOG10*1.2/128))

					//fc = 110.0f * pow(2.0, 0.25f+((double)(pChn->nCutOff*(flt_modifier+256)))/(fx*512.0f));
					//<@mrsbrisby> fc *= (double)(2.0*PI/fs);
					//<@mrsbrisby> where pChn->nCutoff is the "cutoff" as set by channel
					//<@mrsbrisby> and flt_modifier is the envelope value (after having -1 translated to 0x7C; although I think IT used 0x7F)
					//<@mrsbrisby> fx is 0.21 in libmodplug's "extended filter range" and 0.24 for "it modules"
					//<@mrsbrisby> but i think that's wrong, i think IT uses closer to 20.0 (but not exactly)

					const double dInvAngle = (float)(iSampleRate * pow(0.5, 0.25 + iCutoff/24.0) /(TPI*110.0));
					const double dLoss = (float)exp(iRes*(-LN10*1.2/128.0));
	
					//const double dInvAngle = pow(10.0,(127.0-iCutoff)/96.0)-1.0;
					//const double dLoss = pow(10.0f, -((float)iRes / 256.0f));
					//const double dLoss = pow(0.5, iRes/32.0);
	
					//const double dInvAngle = 1.0 /((pow(2.0f,(iCutoff+1)/128.0f)-1.0f)  * PI);
					//const double dLoss = pow (10.0 , -((3.0*iRes) / 320.0)); // approx [1...0]  | -(iRes/128.0) * (24.0/20.0)
					//const double dInvAngle = pow(10.0,(127.0-iCutoff)/72.0)-0.93; // approx [60..0]
					//const double dInvAngle = pow(10.0,(127.0-iCutoff)/72.0)-1.0; // approx [60..0]
					//const double dInvAngle = pow(2.0,(127.0-iCutoff)/22.0)-0.93; // approx [60..0]
					//const double dLoss = pow (2.0,(127.0-iCutoff)/127.0)-1.0; // [1..0]
				
					e = dInvAngle* dInvAngle;
					d = 1.0*( 1.0- dLoss) / dInvAngle;
					if (d > 2.0f) d = 2.0f;
					d = (dLoss - d) * dInvAngle;
					//if ( d + e*2.0 < 0.0 ) d = -e*2.0;
					//const double d = (dLoss)*(dInvAngle+1.4)-1.4;
				}
				else { e = 0; d = 0; }

				if ( ftFilter == F_LOWPASS12) { fCoeff[0]= 1.0f / (1.0f + d + e); fCoeff[3] = 0; }
				else if (ftFilter == F_HIGHPASS12) { fCoeff[0] = 1.0f - (1.0f / (1.0f + d + e)); fCoeff[3] = -1; }
				fCoeff[2]= -e * fCoeff[0];
				fCoeff[1]= 1.0f - fCoeff[0] - fCoeff[2]; // equals to (d+e+e ) * fCoeff[0]

			}
		}
	}
}
