/*		CSIDFilter (C)2008 Jeremy Evers, http:://jeremyevers.com

		This program is free software; you can redistribute it and/or modify
		it under the terms of the GNU General Public License as published by
		the Free Software Foundation; either version 2 of the License, or
		(at your option) any later version.\n"\

		This plugin is distributed in the hope that it will be useful,
		but WITHOUT ANY WARRANTY; without even the implied warranty of
		MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
		GNU General Public License for more details.

		You should have received a copy of the GNU General Public License
		along with this program; if not, write to the Free Software
		Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#define PIf	3.1415926535897932384626433832795f;

#pragma once
enum eAlgorithm {
	FILTER_ALGO_SID_LPF = 0,
	FILTER_ALGO_SID_HPF,
	FILTER_ALGO_SID_BPF,
	FILTER_ALGO_SID_LPF_HPF,
	FILTER_ALGO_SID_LPF_BPF,
	FILTER_ALGO_SID_LPF_HPF_BPF,
	FILTER_ALGO_SID_HPF_BPF
};

class CSIDFilter {
public:
	CSIDFilter();

	void setAlgorithm(eAlgorithm a_algo);
	void reset();
	void process(float& sample);
	void recalculateCoeffs(const float a_fFrequency, const float a_fFeedback);
private:

	eAlgorithm m_Algorithm;

	// pre-calculated coefficients
	float m_f, m_fb;

	// filter data
	float m_low, m_high, m_band;
};

