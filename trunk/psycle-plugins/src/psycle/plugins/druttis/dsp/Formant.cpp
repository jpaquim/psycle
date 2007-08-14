//////////////////////////////////////////////////////////////////////
//
//				Formant.cpp
//
//				druttis@darkface.pp.se
//
//////////////////////////////////////////////////////////////////////
#include <packageneric/pre-compiled.private.hpp>
#include "Formant.h"
//////////////////////////////////////////////////////////////////////
//
//				Filter coeffs
//
//////////////////////////////////////////////////////////////////////
double Formant::m_coeff[5][11] =
{
	//				A
	{
		8.11044e-06,
		8.943665402, -36.83889529, 92.01697887, -154.337906, 181.6233289,
		-151.8651235, 89.09614114, -35.10298511, 8.388101016, -0.923313471
	},
	//				E
	{
		4.36215e-06,
		8.90438318, -36.55179099, 91.05750846, -152.422234, 179.1170248,
		-149.6496211, 87.78352223, -34.60687431, 8.282228154, -0.914150747
	},
	//				I
	{
		4.33819e-06,
		8.893102966, -36.49532826, 90.96543286, -152.4545478, 179.4835618,
		-150.315433, 88.43409371, -34.98612086, 8.407803364, -0.932568035
	},
	//				O
	{
		1.13572e-06,
		8.994734087, -37.2084849, 93.22900521, -156.6929844, 184.596544,
		-154.3755513, 90.49663749, -35.58964535, 8.478996281, -0.929252233
	},
	//				U
	{
		4.09431e-07,
		8.997322763, -37.20218544, 93.11385476, -156.2530937, 183.7080141,
		-153.2631681, 89.59539726, -35.12454591, 8.338655623, -0.910251753
	}
};
//////////////////////////////////////////////////////////////////////
//
//				Vowel names
//
//////////////////////////////////////////////////////////////////////
char *Formant::m_names[5] =
{
	"A",
	"E",
	"I",
	"O",
	"U"
};
//////////////////////////////////////////////////////////////////////
//				Formant constructor
//////////////////////////////////////////////////////////////////////
Formant::Formant()
{
	Reset();
}
//////////////////////////////////////////////////////////////////////
//				Formant destructor
//////////////////////////////////////////////////////////////////////
Formant::~Formant()
{
}
//////////////////////////////////////////////////////////////////////
//				Reset
//////////////////////////////////////////////////////////////////////
void Formant::Reset()
{
	m_buff[0] = 0.0;
	m_buff[1] = 0.0;
	m_buff[2] = 0.0;
	m_buff[3] = 0.0;
	m_buff[4] = 0.0;
	m_buff[5] = 0.0;
	m_buff[6] = 0.0;
	m_buff[7] = 0.0;
	m_buff[8] = 0.0;
	m_buff[9] = 0.0;
}
