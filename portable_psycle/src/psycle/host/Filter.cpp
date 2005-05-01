///\file
///\brief implementation file for psycle::host::Filter.
#include <project.private.hpp>
#include "Filter.hpp"
namespace psycle
{
	namespace host
	{
		namespace dsp
		{
			FilterCoeff Filter::_coeffs;

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
				const double dInvAngle = (float)(iSampleRate * pow(0.5, 0.25 + iCutoff/24.0) /(TPI*110.0));
				const double dLoss = (float)exp(iRes*(-LOG10*1.2/128.0));

				const double d = dLoss*(dInvAngle+1.0) - 1;
				const double e = dInvAngle* dInvAngle;
				fCoeff[0]= 1.0f / (1.0f + d + e);
				fCoeff[2]= -e * fCoeff[0];
				fCoeff[1]= 1.0f - fCoeff[0] - fCoeff[2];
			}
		}
	}
}