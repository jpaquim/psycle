//////////////////////////////////////////////////////////////////////
//
//				Formant.h
//
//				druttis@darkface.pp.se
//
//////////////////////////////////////////////////////////////////////
#pragma once
//////////////////////////////////////////////////////////////////////
//
//				Formant Class
//
//////////////////////////////////////////////////////////////////////
class Formant
{
	//////////////////////////////////////////////////////////////////
	//				Filter coeffs
	//////////////////////////////////////////////////////////////////
private:
	static const double m_coeff[5][11];
	static char const *m_names[5];
	//////////////////////////////////////////////////////////////////
	//				Buffer
	//////////////////////////////////////////////////////////////////
	double				m_buff[10];
	//////////////////////////////////////////////////////////////////
	//				Constructor / Destructor
	//////////////////////////////////////////////////////////////////
public:
	Formant();
	~Formant();
	//////////////////////////////////////////////////////////////////
	//				Reset
	//////////////////////////////////////////////////////////////////
	void Reset();
	//////////////////////////////////////////////////////////////////
	//				GetVowelName
	//////////////////////////////////////////////////////////////////
	inline static char const * GetVowelName(int num)
	{
		return m_names[num];
	}
	//////////////////////////////////////////////////////////////////
	//				Next(float in)
	//////////////////////////////////////////////////////////////////
	inline float Next(float in, int vowelnum)
	{
		float res = (float)
		(
			m_coeff[vowelnum][0] * in +
			m_coeff[vowelnum][1] * m_buff[0] +
			m_coeff[vowelnum][2] * m_buff[1] +
			m_coeff[vowelnum][3] * m_buff[2] +
			m_coeff[vowelnum][4] * m_buff[3] +
			m_coeff[vowelnum][5] * m_buff[4] +
			m_coeff[vowelnum][6] * m_buff[5] +
			m_coeff[vowelnum][7] * m_buff[6] +
			m_coeff[vowelnum][8] * m_buff[7] +
			m_coeff[vowelnum][9] * m_buff[8] +
			m_coeff[vowelnum][10] * m_buff[9]
		);
		m_buff[9] = m_buff[8];
		m_buff[8] = m_buff[7];
		m_buff[7] = m_buff[6];
		m_buff[6] = m_buff[5];
		m_buff[5] = m_buff[4];
		m_buff[4] = m_buff[3];
		m_buff[3] = m_buff[2];
		m_buff[2] = m_buff[1];
		m_buff[1] = m_buff[0];
		m_buff[0] = (double) res;
		return res;
	}
};
