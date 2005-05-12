///\file
///\brief implementation file for psycle::host::Filter.
#include <project.private.hpp>
#include "Player.hpp"
#include "Filter.hpp"
namespace psycle
{
	namespace host
	{
		namespace dsp
		{
			FilterCoeff Filter::_coeffs;


			void FilterCoeff::ComputeCoeffs(int freq, int r, int t)
			{
				float omega = float(TPI*Cutoff(freq)/Global::pPlayer->SampleRate());
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
				_coeffs.Init();
				Init();
				_x1 = _x2 = _y1 = _y2 = 0;
				_a1 = _a2 = _b1 = _b2 = 0;
			}
			
			void Filter::Init()
			{
				_cutoff=127;
				_q=0;
				_type = F_LOWPASS12;
				Update();
			}

			void Filter::Update()
			{
				_coeff0 = _coeffs._coeffs[_type][_cutoff][_q][0];
				_coeff1 = _coeffs._coeffs[_type][_cutoff][_q][1];
				_coeff2 = _coeffs._coeffs[_type][_cutoff][_q][2];
				_coeff3 = _coeffs._coeffs[_type][_cutoff][_q][3];
				_coeff4 = _coeffs._coeffs[_type][_cutoff][_q][4];
			}

			void ITFilter::Update()
			{
#define PI 3.1415926535897932384626433832795
				/*				pole[0]=pole[1]=pole[2]=pole[3]=0;
				float fSampleRate = (float)iSampleRate;
				float fCutoff = fSampleRate* (pow(2.0f,iCutoff/127.0f)-1.0f)/2.0f;

				fCoeff[1] =			((fSampleRate) - (fCutoff * TPI))
				/ (	(fSampleRate) + (fCutoff * TPI));
				fCoeff[0] =
				fCoeff[2] = (fCutoff * TPI)
				/ ( (fCutoff * TPI) + ( fSampleRate));
				*/

/*				fc *= (float)(2.0*3.14159265358/fs);
				float dmpfac = pow(10.0f, -((24.0f / 128.0f)*(float)pChn->nResonance) / 20.0f);

*/
					

//				const double dInvAngle = (float)(iSampleRate * pow(0.5, 0.25 + iCutoff/24.0) /(TPI*110.0));
				const double dLoss = (float)exp(iRes*(-LOG10*1.2/128.0));

//				const double dInvAngle = pow(10.0,(127.0-iCutoff)/96.0)-1.0;
//				const double dLoss = pow(10.0f, -((float)iRes / 256.0f));
//				const double dLoss = pow(0.5, iRes/32.0);

//				const double dInvAngle = 1.0 /((pow(2.0f,(iCutoff+1)/128.0f)-1.0f)  * PI);
//				const double dLoss = pow (10.0 , -((3.0*iRes) / 320.0)); // approx [1...0]  | -(iRes/128.0) * (24.0/20.0)
//				const double dInvAngle = pow(10.0,(127.0-iCutoff)/72.0)-0.93; // approx [60..0]
//				const double dInvAngle = pow(10.0,(127.0-iCutoff)/72.0)-1.0; // approx [60..0]
				const double dInvAngle = pow(2.0,(127.0-iCutoff)/22.0)-0.93; // approx [60..0]
//				const double dLoss = pow (2.0,(127.0-iCutoff)/127.0)-1.0;	// [1..0]
			
				const double e = dInvAngle* dInvAngle;
				double d = 1.2*( 1.0- dLoss) / dInvAngle;
				if (d > 2.0f) d = 2.0f;
				d = (dLoss - d) * dInvAngle;
//				if ( d + e*2.0 < 0.0 ) d = -e*2.0;

//				const double d = (dLoss)*(dInvAngle+1.4)-1.4;
				if ( ftFilter == F_LOWPASS12) { fCoeff[0]= 1.0f / (1.0f + d + e); fCoeff[3] = 0; }
				else if (ftFilter == F_HIGHPASS12) { fCoeff[0] = 1.0f - (1.0f / (1.0f + d + e)); fCoeff[3] = -1; }
				fCoeff[2]= -e * fCoeff[0];
				fCoeff[1]= 1.0f - fCoeff[0] - fCoeff[2]; // equals to (d+e+e ) * fCoeff[0]



#undef PI
			}
		}
	}
}