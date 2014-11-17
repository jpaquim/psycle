// This program is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// copyright 2007-2009 members of the psycle project http://psycle.sourceforge.net

///\implementation psycle::helpers::Filter.

#include "filter.hpp"

#include <cmath>
#include <algorithm>

namespace psycle { namespace helpers { namespace dsp {

FilterCoeff FilterCoeff::singleton;

//From Modplug tracker
const float ITResonanceTable[128] =
{
	1.0000000000000000f, 0.9786446094512940f, 0.9577452540397644f, 0.9372922182083130f,
	0.9172759056091309f, 0.8976871371269226f, 0.8785166740417481f, 0.8597555756568909f,
	0.8413951396942139f, 0.8234267830848694f, 0.8058421611785889f, 0.7886331081390381f,
	0.7717915177345276f, 0.7553095817565918f, 0.7391796708106995f, 0.7233941555023193f,
	0.7079457640647888f, 0.6928272843360901f, 0.6780316829681397f, 0.6635520458221436f,
	0.6493816375732422f, 0.6355138421058655f, 0.6219421625137329f, 0.6086603403091431f,
	0.5956621170043945f, 0.5829415321350098f, 0.5704925656318665f, 0.5583094954490662f,
	0.5463865399360657f, 0.5347182154655457f, 0.5232990980148315f, 0.5121238231658936f,
	0.5011872053146362f, 0.4904841780662537f, 0.4800096750259399f, 0.4697588682174683f,
	0.4597269892692566f, 0.4499093294143677f, 0.4403013288974762f, 0.4308985173702240f,
	0.4216965138912201f, 0.4126909971237183f, 0.4038778245449066f, 0.3952528536319733f,
	0.3868120610713959f, 0.3785515129566193f, 0.3704673945903778f, 0.3625559210777283f,
	0.3548133969306946f, 0.3472362160682678f, 0.3398208320140839f, 0.3325638175010681f,
	0.3254617750644684f, 0.3185114264488220f, 0.3117094635963440f, 0.3050527870655060f,
	0.2985382676124573f, 0.2921628654003143f, 0.2859236001968384f, 0.2798175811767578f,
	0.2738419771194458f, 0.2679939568042755f, 0.2622708380222321f, 0.2566699385643005f,
	0.2511886358261108f, 0.2458244115114212f, 0.2405747324228287f, 0.2354371547698975f,
	0.2304092943668366f, 0.2254888117313385f, 0.2206734120845795f, 0.2159608304500580f,
	0.2113489061594009f, 0.2068354636430740f, 0.2024184018373489f, 0.1980956792831421f,
	0.1938652694225311f, 0.1897251904010773f, 0.1856735348701477f, 0.1817083954811096f,
	0.1778279393911362f, 0.1740303486585617f, 0.1703138649463654f, 0.1666767448186874f,
	0.1631172895431519f, 0.1596338599920273f, 0.1562248021364212f, 0.1528885662555695f,
	0.1496235728263855f, 0.1464282870292664f, 0.1433012634515762f, 0.1402409970760346f,
	0.1372461020946503f, 0.1343151479959488f, 0.1314467936754227f, 0.1286396980285645f,
	0.1258925348520279f, 0.1232040524482727f, 0.1205729842185974f, 0.1179980933666229f,
	0.1154781952500343f, 0.1130121126770973f, 0.1105986908078194f, 0.1082368120551109f,
	0.1059253737330437f, 0.1036632955074310f, 0.1014495193958283f, 0.0992830246686935f,
	0.0971627980470657f, 0.0950878411531448f, 0.0930572077631950f, 0.0910699293017387f,
	0.0891250967979431f, 0.0872217938303947f, 0.0853591337800026f, 0.0835362523794174f,
	0.0817523002624512f, 0.0800064504146576f, 0.0782978758215904f, 0.0766257941722870f,
	0.0749894231557846f, 0.0733879879117012f, 0.0718207582831383f, 0.0702869966626167f,
	0.0687859877943993f, 0.0673170387744904f, 0.0658794566988945f, 0.0644725710153580f,
};

FilterCoeff::FilterCoeff() {
	samplerate = -1;
	//table is initialized with Filter::Init()
}

void FilterCoeff::setSampleRate(float samplerate)
{
	if (samplerate != this->samplerate)
	{
		this->samplerate = samplerate;
		for (int t=0; t<F_NUMFILTERS; t++)
		{
			int tdest = t;
			if (t == F_NONE) continue;//Skip filter F_NONE.
			else if (t > F_NONE) tdest--;

			for (int f=0; f<128; f++)
			{
				for (int q=0; q<128; q++)
				{
					ComputeCoeffs(static_cast<FilterType>(t), f, q);
					_coeffs[tdest][f][q][0] = (float)_coeff[0];
					_coeffs[tdest][f][q][1] = (float)_coeff[1];
					_coeffs[tdest][f][q][2] = (float)_coeff[2];
					_coeffs[tdest][f][q][3] = (float)_coeff[3];
					_coeffs[tdest][f][q][4] = (float)_coeff[4];
				}
			}
		}
	}
}

float FilterCoeff::Cutoff(FilterType ft, int freq)
{
	if (ft == F_NONE) {
		return 0.f;
	}
	else if (ft == F_ITLOWPASS) {
		return CutoffIT(freq);
	}
	else if (ft == F_MPTLOWPASSE || ft == F_MPTHIGHPASSE) {
		return CutoffMPTExt(freq);
	}
	else if (ft == F_LOWPASS12E || ft == F_HIGHPASS12E
			|| ft == F_BANDPASS12E || ft == F_BANDREJECT12E) {
		return CutoffInternalExt(freq);
	}
	else { 
		return CutoffInternal(freq);
	}
}
float FilterCoeff::Resonance(FilterType ft, int freq, int r) 
{
	if (ft == F_NONE) {
		return 0.f;
	}
	else if (ft == F_ITLOWPASS) {
		return ResonanceIT(r);
	}
	else if (ft == F_MPTLOWPASSE || ft == F_MPTHIGHPASSE) {
		return ResonanceMPT(r);
	}
	else if (ft == F_LOWPASS12 || ft == F_HIGHPASS12) {
		return ResonanceInternal(r*(freq+70)/(127.0f+70.f))*0.5;
	}
	else if (ft == F_LOWPASS12E || ft == F_HIGHPASS12E) {
		return ResonanceInternalExt(r);
	}
	else if (ft == F_BANDPASS12E || ft == F_BANDREJECT12E ) { 
		return 0.5/sinh( BandwidthInternalExt( r));
	}
	else {
		return 0.5/sinh( BandwidthInternal( r));
	}
}

float FilterCoeff::CutoffInternal(int v)
{
	return float(pow( (v+5)/(127.0+5), 1.7)*13000+30);
}

float FilterCoeff::ResonanceInternal(float v)
{
	return float(pow( v/127.0, 4)*150+0.1);
}

float FilterCoeff::BandwidthInternal(int v)
{
	return float(pow( v/127.0, 4)*4+0.1);
}
float FilterCoeff::CutoffInternalExt(int v)
{
	const float factor = 8.f/132.f;
	const float factor2 = 20000.f/256.f;
	return float((powf(2.0f,(v+5)*factor)-1.f)*factor2);
}

float FilterCoeff::ResonanceInternalExt(float v)
{
	return float(pow( v/127.0, 2)*16+0.1);
}
float FilterCoeff::BandwidthInternalExt(int v)
{
	return float(pow((127-v)/127.0, 2)*2+0.1);
}

double FilterCoeff::CutoffIT(int v)
{
	return 110.0 * pow(2.0,(v+6)/24.0);
}
double FilterCoeff::CutoffMPTExt(int v)
{
	return 110.0 * pow(2.0, (v+5) / 20.0);
}
double FilterCoeff::ResonanceIT(int r)
{
	return 1.0/ITResonanceTable[r];
}
double FilterCoeff::ResonanceMPT(int resonance)
{
	const double dmpfac = pow(10.0, -((24.0 / 128.0) * (double)resonance) / 20.0);
	return 1.0/dmpfac;
}

void FilterCoeff::ComputeCoeffs(FilterType t, int freq, int r)
{
	double frequency =  Cutoff(t, freq);
	const double samplerate_d = static_cast<double>(samplerate);
	if (frequency*2.0 > samplerate_d) { frequency = samplerate_d*0.5; }
	double omega = float(TPI*frequency)/samplerate;
	float sn = (float)sin(omega);
	float cs = (float)cos(omega);
	float alpha;
	if (t == F_LOWPASS12 || t == F_HIGHPASS12) alpha = float(sn / ResonanceInternal(r*(freq+70)/(127.0f+70.f)));
	else if (t == F_LOWPASS12E || t == F_HIGHPASS12E) alpha = float(sn / ResonanceInternalExt(r));
	else if (t == F_BANDPASS12E || t == F_BANDREJECT12E) alpha = float(sinh( BandwidthInternalExt( r) * omega));
	else alpha = float(sinh( BandwidthInternal( r) * omega));
	float a0, a1, a2, b0, b1, b2;
	switch (t)
	{
	case F_LOWPASS12:
	case F_LOWPASS12E:
		b0 =  (1.f - cs)/2.f;
		b1 =   1.f - cs;
		b2 =  (1.f - cs)/2.f;
		a0 =   1.f + alpha;
		a1 =  -2.f*cs;
		a2 =   1.f - alpha;
		break;
	case F_HIGHPASS12:
	case F_HIGHPASS12E:
		b0 =  (1.f + cs)/2.f;
		b1 = -(1.f + cs);
		b2 =  (1.f + cs)/2.f;
		a0 =   1.f + alpha;
		a1 =  -2.f*cs;
		a2 =   1.f - alpha;
		break;
	case F_BANDPASS12:
	case F_BANDPASS12E:
		b0 =   alpha;
		b1 =   0.f;
		b2 =  -alpha;
		a0 =   1.f + alpha;
		a1 =  -2.f*cs;
		a2 =   1.f - alpha;
		break;
	case F_BANDREJECT12:
	case F_BANDREJECT12E:
		b0 =   1.f;
		b1 =  -2.f*cs;
		b2 =   1.f;
		a0 =   1.f + alpha;
		a1 =  -2.f*cs;
		a2 =   1.f - alpha;
		break;
	case F_ITLOWPASS:
	{
		//From modplug
		double d,e;
		if ( r >0 || freq < 127 )
		{
			const double radio = samplerate / (TPI* frequency);
			d = ITResonanceTable[r] * radio + ITResonanceTable[r] - 1.0;
			e = radio * radio;
		}
		else { e = 0; d = 0; }
		b0 = 1.0f;
		b1 = 0.0f; //coeff[1] reused in ITFilter as Lowpass/highpass coeff
		b2 = 0.0f; //coeff[2] not used in ITFilter
		a0 = 1.0f + d + e;
		a1 = -(d + e + e);
		a2 = e;
		break;
	}
	case F_MPTLOWPASSE:
	{
		//From modplug
		double d,e;
		if ( r >0 || freq < 127 )
		{
			const double dmpfac = pow(10.0, -((24.0 / 128.0) * (float)r) / 20.0);
			const double fc = (TPI* frequency) / samplerate;
			d = (1.0f - 2.0f * dmpfac) * fc;
			if (d > 2.0 ) { d = 2.0; }
			d = (2.0 * dmpfac - d) / fc;
			e = 1.0 / (fc * fc);
		}
		else { e = 0; d = 0; }
		b0 = 1.0f;
		b1 = 0.0f; //coeff[1] reused in ITFilter as Lowpass/highpass coeff
		b2 = 0.0f; //coeff[2] not used in ITFilter
		a0 = 1.0f + d + e;
		a1 = -(d + e + e);
		a2 = e;
		break;
	}
	case F_MPTHIGHPASSE:
	{
		//From modplug
		double d,e;
		if ( r >0 || freq < 127 )
		{
			const double dmpfac = pow(10.0, -((24.0 / 128.0) * (float)r) / 20.0);
			const double fc = (TPI* frequency) / samplerate;
			d = (1.0f - 2.0f * dmpfac) * fc;
			if (d > 2.0 ) { d = 2.0; }
			d = (2.0 * dmpfac - d) / fc;
			e = pow(1.0 / fc, 2.0);
		}
		else { e = 0; d = 0; }
		b0 = d + e; //  coeff[0] = 1.0 - (1.0/a0)
		b1 = 1.0f + d + e; //coeff[1] reused in ITFilter as Lowpass/highpass coeff
		b2 = 0.0f; //coeff[2] not used in ITFilter
		a0 = 1.0f + d + e;
		a1 = -(d + e + e);
		a2 = e;
		break;
	}
	default:
		b0 =  1;
		b1 =  0;
		b2 =  0;
		a0 =  1;
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
	//Filtercoef is initialized when calling setSampleRate().
	Init(44100);
}

void Filter::Init(int sampleRate)
{
	FilterCoeff::singleton.setSampleRate(sampleRate);
	Reset();
}
void Filter::Reset(void) {
	_cutoff=127;
	_q=0;
	_type = F_NONE;
	_x1 = _x2 = _y1 = _y2 = 0;
	_a1 = _a2 = _b1 = _b2 = 0;
	Update();
}

void Filter::Update()
{
	if (_type == F_NONE) {
		_coeff0 =  1;
		_coeff1 =  0;
		_coeff2 =  0;
		_coeff3 =  0;
		_coeff4 =  0;
		return;
	}
	else {
		int type = _type;
		if (type > F_NONE) type--;
		_coeff0 = FilterCoeff::singleton._coeffs[type][_cutoff][_q][0];
		_coeff1 = FilterCoeff::singleton._coeffs[type][_cutoff][_q][1];
		_coeff2 = FilterCoeff::singleton._coeffs[type][_cutoff][_q][2];
		_coeff3 = FilterCoeff::singleton._coeffs[type][_cutoff][_q][3];
		_coeff4 = FilterCoeff::singleton._coeffs[type][_cutoff][_q][4];
	}
}

}}}

