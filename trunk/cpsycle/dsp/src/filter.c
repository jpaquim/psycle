// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "filter.h"

#include <math.h>

FilterCoeff filtercoeff;
int filtercoeff_inititalized = 0;

//From Modplug tracker
const double ITResonanceTable[128] =
{
	1.0000000000000000, 0.9786446094512940, 0.9577452540397644, 0.9372922182083130,
	0.9172759056091309, 0.8976871371269226, 0.8785166740417481, 0.8597555756568909,
	0.8413951396942139, 0.8234267830848694, 0.8058421611785889, 0.7886331081390381,
	0.7717915177345276, 0.7553095817565918, 0.7391796708106995, 0.7233941555023193,
	0.7079457640647888, 0.6928272843360901, 0.6780316829681397, 0.6635520458221436,
	0.6493816375732422, 0.6355138421058655, 0.6219421625137329, 0.6086603403091431,
	0.5956621170043945, 0.5829415321350098, 0.5704925656318665, 0.5583094954490662,
	0.5463865399360657, 0.5347182154655457, 0.5232990980148315, 0.5121238231658936,
	0.5011872053146362, 0.4904841780662537, 0.4800096750259399, 0.4697588682174683,
	0.4597269892692566, 0.4499093294143677, 0.4403013288974762, 0.4308985173702240,
	0.4216965138912201, 0.4126909971237183, 0.4038778245449066, 0.3952528536319733,
	0.3868120610713959, 0.3785515129566193, 0.3704673945903778, 0.3625559210777283,
	0.3548133969306946, 0.3472362160682678, 0.3398208320140839, 0.3325638175010681,
	0.3254617750644684, 0.3185114264488220, 0.3117094635963440, 0.3050527870655060,
	0.2985382676124573, 0.2921628654003143, 0.2859236001968384, 0.2798175811767578,
	0.2738419771194458, 0.2679939568042755, 0.2622708380222321, 0.2566699385643005,
	0.2511886358261108, 0.2458244115114212, 0.2405747324228287, 0.2354371547698975,
	0.2304092943668366, 0.2254888117313385, 0.2206734120845795, 0.2159608304500580,
	0.2113489061594009, 0.2068354636430740, 0.2024184018373489, 0.1980956792831421,
	0.1938652694225311, 0.1897251904010773, 0.1856735348701477, 0.1817083954811096,
	0.1778279393911362, 0.1740303486585617, 0.1703138649463654, 0.1666767448186874,
	0.1631172895431519, 0.1596338599920273, 0.1562248021364212, 0.1528885662555695,
	0.1496235728263855, 0.1464282870292664, 0.1433012634515762, 0.1402409970760346,
	0.1372461020946503, 0.1343151479959488, 0.1314467936754227, 0.1286396980285645,
	0.1258925348520279, 0.1232040524482727, 0.1205729842185974, 0.1179980933666229,
	0.1154781952500343, 0.1130121126770973, 0.1105986908078194, 0.1082368120551109,
	0.1059253737330437, 0.1036632955074310, 0.1014495193958283, 0.0992830246686935,
	0.0971627980470657, 0.0950878411531448, 0.0930572077631950, 0.0910699293017387,
	0.0891250967979431, 0.0872217938303947, 0.0853591337800026, 0.0835362523794174,
	0.0817523002624512, 0.0800064504146576, 0.0782978758215904, 0.0766257941722870,
	0.0749894231557846, 0.0733879879117012, 0.0718207582831383, 0.0702869966626167,
	0.0687859877943993, 0.0673170387744904, 0.0658794566988945, 0.0644725710153580
};

void filtercoeff_init(FilterCoeff* self)
{
	if (!filtercoeff_inititalized) {
		self->samplerate = -1;
		filtercoeff_inititalized = 1;
	}
	//table is initialized with Filter::Init()
}

void filtercoeff_dispose(FilterCoeff* self)
{

}

void filtercoeff_setsamplerate(FilterCoeff* self, float samplerate)
{
	if (samplerate != self->samplerate)
	{
		int t;

		self->samplerate = samplerate;
		for (t = 0; t < F_NUMFILTERS; t++)
		{
			int tdest = t;
			int f;

			if (t == F_NONE) continue;//Skip filter F_NONE.
			else if (t > F_NONE) tdest--;

			for (f = 0; f < 128; f++)
			{
				int q;

				for (q = 0; q < 128; q++)
				{
					filtercoeff_computecoeffs(self, (psy_dsp_FilterType)(t), f, q);
					self->_coeffs[tdest][f][q][0] = (float)self->_coeff[0];
					self->_coeffs[tdest][f][q][1] = (float)self->_coeff[1];
					self->_coeffs[tdest][f][q][2] = (float)self->_coeff[2];
					self->_coeffs[tdest][f][q][3] = (float)self->_coeff[3];
					self->_coeffs[tdest][f][q][4] = (float)self->_coeff[4];
				}
			}
		}
	}
}

float filtercoeff_cutoff(psy_dsp_FilterType ft, int freq)
{
	if (ft == F_NONE) {
		return 0.f;
	} else if (ft == F_ITLOWPASS) {
		return (float)filtercoeff_cutoffit(freq);
	} else if (ft == F_MPTLOWPASSE || ft == F_MPTHIGHPASSE) {
		return (float)filtercoeff_cutoffmptext(freq);
	} else if (ft == F_LOWPASS12E || ft == F_HIGHPASS12E
		|| ft == F_BANDPASS12E || ft == F_BANDREJECT12E) {
		return (float)filtercoeff_cutoffinternalext(freq);
	} else {
		return (float)filtercoeff_cutoffinternal(freq);
	}
}

float filtercoeff_resonance(psy_dsp_FilterType ft, int freq, int r)
{
	if (ft == F_NONE) {
		return 0.f;
	} else if (ft == F_ITLOWPASS) {
		return (float)filtercoeff_resonanceit(r);
	} else if (ft == F_MPTLOWPASSE || ft == F_MPTHIGHPASSE) {
		return (float)filtercoeff_resonancempt(r);
	} else if (ft == F_LOWPASS12 || ft == F_HIGHPASS12) {
		return (float)filtercoeff_resonanceinternal(r * (freq + 70) / (127.0f + 70.f));
	} else if (ft == F_LOWPASS12E || ft == F_HIGHPASS12E) {
		return (float)filtercoeff_resonanceinternalext((float)r);
	} else if (ft == F_BANDPASS12E || ft == F_BANDREJECT12E) {
		return (float)(0.5 / sinh(filtercoeff_bandwidthinternalext(r)));
	} else {
		return (float)(0.5 / sinh(filtercoeff_bandwidthinternal(r)));
	}
}

float filtercoeff_cutoffinternal(int v)
{
	return (float)(pow((v + 5) / (127.0 + 5), 1.7) * 13000 + 30);
}

float filtercoeff_resonanceinternal(float v)
{
	return (float)((pow(v / 127.0, 4) * 150 + 0.1) * 0.5);
}

float filtercoeff_bandwidthinternal(int v)
{
	return (float)(pow(v / 127.0, 4) * 4 + 0.1);
}

float filtercoeff_cutoffinternalext(int v)
{
	const float factor = 8.f / 132.f;
	const float factor2 = 20000.f / 256.f;
	return (float)((pow(2.0f, (v + 5) * factor) - 1.f) * factor2);
}

float filtercoeff_resonanceinternalext(float v)
{
	return (float)((pow((v + 28) / 155.0, 2.0) * 32.0) * 0.5);
}

float filtercoeff_bandwidthinternalext(int v)
{
	return (float)(pow((128 - v) / 127.0 * 0.5 + 0.12, 2.0) * 2.0);
}

double filtercoeff_cutoffit(int v)
{
	return 110.0 * pow(2.0, (v + 6) / 24.0);
}

double filtercoeff_cutoffmptext(int v)
{
	return 110.0 * pow(2.0, (v + 5) / 20.0);
}

double filtercoeff_resonanceit(int r)
{
	return 1.0 / ITResonanceTable[r];
}

double filtercoeff_resonancempt(int resonance)
{
	const double dmpfac = pow(10.0, -((24.0 / 128.0) * (double)resonance) / 20.0);
	return 1.0 / dmpfac;
}

void filtercoeff_computecoeffs(FilterCoeff* self, psy_dsp_FilterType t, int freq, int r)
{
	double frequency;
	double samplerate_d;
	double omega;
	float sn;
	float cs;
	float alpha;
	float a0, a1, a2, b0, b1, b2;

	frequency = filtercoeff_cutoff(t, freq);
	samplerate_d = (double)(self->samplerate);
	if (frequency * 2.0 > samplerate_d) { frequency = samplerate_d * 0.5; }
	omega = (float)(TPI * frequency) / self->samplerate;
	sn = (float)sin(omega);
	cs = (float)cos(omega);
	if (t == F_LOWPASS12 || t == F_HIGHPASS12) alpha = (float)(sn * 0.5f / filtercoeff_resonanceinternal(r * (freq + 70) / (127.0f + 70.f)));
	else if (t == F_LOWPASS12E || t == F_HIGHPASS12E) alpha = (float)(sn * 0.5f / filtercoeff_resonanceinternalext((float)r));
	else if (t == F_BANDPASS12E || t == F_BANDREJECT12E) alpha = (float)(sinh(filtercoeff_bandwidthinternalext(r) * omega));
	else alpha = (float)(sinh(filtercoeff_bandwidthinternal(r) * omega));	
	switch (t)
	{
	case F_LOWPASS12:
	case F_LOWPASS12E:
		b0 = (1.f - cs) / 2.f;
		b1 = 1.f - cs;
		b2 = (1.f - cs) / 2.f;
		a0 = 1.f + alpha;
		a1 = -2.f * cs;
		a2 = 1.f - alpha;
		break;
	case F_HIGHPASS12:
	case F_HIGHPASS12E:
		b0 = (1.f + cs) / 2.f;
		b1 = -(1.f + cs);
		b2 = (1.f + cs) / 2.f;
		a0 = 1.f + alpha;
		a1 = -2.f * cs;
		a2 = 1.f - alpha;
		break;
	case F_BANDPASS12:
	case F_BANDPASS12E:
		b0 = alpha;
		b1 = 0.f;
		b2 = -alpha;
		a0 = 1.f + alpha;
		a1 = -2.f * cs;
		a2 = 1.f - alpha;
		break;
	case F_BANDREJECT12:
	case F_BANDREJECT12E:
		b0 = 1.f;
		b1 = -2.f * cs;
		b2 = 1.f;
		a0 = 1.f + alpha;
		a1 = -2.f * cs;
		a2 = 1.f - alpha;
		break;
	case F_ITLOWPASS:
	{
		//From modplug
		double d, e;
		if (r > 0 || freq < 127)
		{
			const double radio = self->samplerate / (TPI * frequency);
			d = ITResonanceTable[r] * radio + ITResonanceTable[r] - 1.0;
			e = radio * radio;
		} else { e = 0; d = 0; }
		b0 = 1.0f;
		b1 = 0.0f; //coeff[1] reused in ITFilter as Lowpass/highpass coeff
		b2 = 0.0f; //coeff[2] not used in ITFilter
		a0 = (float)(1.0f + d + e);
		a1 = (float)(-(d + e + e));
		a2 = (float)e;
		break;
	}
	case F_MPTLOWPASSE:
	{
		//From modplug
		double d, e;
		if (r > 0 || freq < 127)
		{
			const double dmpfac = pow(10.0, -((24.0 / 128.0) * (float)r) / 20.0);
			const double fc = (TPI * frequency) / self->samplerate;
			d = (1.0f - 2.0f * dmpfac) * fc;
			if (d > 2.0) { d = 2.0; }
			d = (2.0 * dmpfac - d) / fc;
			e = 1.0 / (fc * fc);
		} else { e = 0; d = 0; }
		b0 = 1.0f;
		b1 = 0.0f; //coeff[1] reused in ITFilter as Lowpass/highpass coeff
		b2 = 0.0f; //coeff[2] not used in ITFilter
		a0 = (float)(1.0f + d + e);
		a1 = (float)(-(d + e + e));
		a2 = (float)e;
		break;
	}
	case F_MPTHIGHPASSE:
	{
		//From modplug
		double d, e;
		if (r > 0 || freq < 127)
		{
			const double dmpfac = pow(10.0, -((24.0 / 128.0) * (float)r) / 20.0);
			const double fc = (TPI * frequency) / self->samplerate;
			d = (1.0f - 2.0f * dmpfac) * fc;
			if (d > 2.0) { d = 2.0; }
			d = (2.0 * dmpfac - d) / fc;
			e = pow(1.0 / fc, 2.0);
		} else { e = 0; d = 0; }
		b0 = (float)(d + e); //  coeff[0] = 1.0 - (1.0/a0)
		b1 = (float)(1.0f + d + e); //coeff[1] reused in ITFilter as Lowpass/highpass coeff
		b2 = 0.0f; //coeff[2] not used in ITFilter
		a0 = (float)(1.0f + d + e);
		a1 = (float)(-(d + e + e));
		a2 = (float)(e);
		break;
	}
	default:
		b0 = 1;
		b1 = 0;
		b2 = 0;
		a0 = 1;
		a1 = 0;
		a2 = 0;
	}
	self->_coeff[0] = b0 / a0;
	self->_coeff[1] = b1 / a0;
	self->_coeff[2] = b2 / a0;
	self->_coeff[3] = -a1 / a0;
	self->_coeff[4] = -a2 / a0;
}


// Filter

float filter_work(Filter*, float x);
void filter_workstereo(Filter*, float* l, float* r);

FilterVtable filtervtable;
static filtervtable_initialized = 0;

void filtervtable_init(Filter* self)
{
	if (!filtervtable_initialized) {
		filtervtable.work = filter_work;
		filtervtable.workstereo = filter_workstereo;
		filtervtable_initialized = 1;
	}		
}

void filter_init(Filter* self)
{
	filtervtable_init(self);
	self->vtable = &filtervtable;
	filter_init_samplerate(self, 44100);
}

void filter_init_samplerate(Filter* self, int sampleRate)
{
	filtervtable_init(self);
	self->vtable = &filtervtable;
	filtercoeff_init(&filtercoeff);
	filtercoeff_setsamplerate(&filtercoeff, (float)sampleRate);
	filter_reset(self);
}

void filter_reset(Filter* self) //Same as init, without samplerate
{
	self->cutoff = 127;
	self->_q = 0;
	self->_type = F_NONE;
	self->_x1 = self->_x2 = self->_y1 = self->_y2 = 0;
	self->_a1 = self->_a2 = self->_b1 = self->_b2 = 0;
	filter_update(self);
}

void filter_dispose(Filter* self)
{

}

void filter_update(Filter* self)
{
	if (self->_type == F_NONE) {
		self->_coeff0 = 1;
		self->_coeff1 = 0;
		self->_coeff2 = 0;
		self->_coeff3 = 0;
		self->_coeff4 = 0;
		return;
	} else {
		int type = self->_type;
		if (type > F_NONE) type--;
		self->_coeff0 = filtercoeff._coeffs[type][self->cutoff][self->_q][0];
		self->_coeff1 = filtercoeff._coeffs[type][self->cutoff][self->_q][1];
		self->_coeff2 = filtercoeff._coeffs[type][self->cutoff][self->_q][2];
		self->_coeff3 = filtercoeff._coeffs[type][self->cutoff][self->_q][3];
		self->_coeff4 = filtercoeff._coeffs[type][self->cutoff][self->_q][4];
	}
}

float filter_work(Filter* self, float x)
{
	float y = x * self->_coeff0 + self->_x1 * self->_coeff1 + self->_x2 * self->_coeff2 + self->_y1 * self->_coeff3 + self->_y2 * self->_coeff4;
	self->_y2 = self->_y1;  self->_y1 = y;
	self->_x2 = self->_x1;  self->_x1 = x;
	return y;
}

void filter_workstereo(Filter* self, float* l, float* r)
{
	float y;
	float b;

	y = *l * self->_coeff0 + self->_x1 * self->_coeff1 + self->_x2 * self->_coeff2 + self->_y1 * self->_coeff3 + self->_y2 * self->_coeff4;
	self->_y2 = self->_y1;  self->_y1 = y;
	self->_x2 = self->_x1;  self->_x1 = *l;
	*l = y;
	b = *r * self->_coeff0 + self->_a1 * self->_coeff1 + self->_a2 * self->_coeff2 + self->_b1 * self->_coeff3 + self->_b2 * self->_coeff4;
	self->_b2 = self->_b1;  self->_b1 = b;
	self->_a2 = self->_a1;  self->_a1 = *r;
	*r = b;
}

/*Code from Modplug */
//This means clip to twice the range (Psycle works in float, but with the -32768 to 32768 range)
#define ClipFilter(x) x //math::clip<float>(-65535.f, x, 65535.f)

static float itfilter_work(ITFilter* self, float sample);
static void itfilter_workstereo(ITFilter* self, float* left, float* right);

FilterVtable itfiltervtable;
static itfiltervtable_initialized = 0;

void itfiltervtable_init(ITFilter* self)
{
	if (!itfiltervtable_initialized) {
		itfiltervtable.work = filter_work;
		itfiltervtable.workstereo = filter_workstereo;
		itfiltervtable_initialized = 1;
	}
}

void itfilter_init(ITFilter* self)
{
	filter_init(&self->filter);
	itfiltervtable_init(self);
	self->filter.vtable = &itfiltervtable;
}

float itfilter_work(ITFilter* self, float sample) {
	float y = sample * self->filter._coeff0 + ClipFilter(self->filter._y1) * self->filter._coeff3 +
		ClipFilter(self->filter._y2) * self->filter._coeff4;
	self->filter._y2 = self->filter._y1;
	self->filter._y1 = y - (sample * self->filter._coeff1);
	return y;
}

void itfilter_workstereo(ITFilter* self, float* left, float* right)
{
	float y = *left * self->filter._coeff0 + ClipFilter(self->filter._y1) * self->filter._coeff3 +
		ClipFilter(self->filter._y2) * self->filter._coeff4;
	self->filter._y2 = self->filter._y1;
	self->filter._y1 = y - (*left * self->filter._coeff1);
	*left = y;

	y = *right * self->filter._coeff0 + ClipFilter(self->filter._b1) * self->filter._coeff3 +
		ClipFilter(self->filter._b2) * self->filter._coeff4;
	self->filter._b2 = self->filter._b1;
	self->filter._b1 = y - (*right * self->filter._coeff1);
	*right = y;
}

const char* filter_name(psy_dsp_FilterType type)
{
	static const char* names[] = {
		"LowPass 2p (old)",
		"HighPass 2p (old)",
		"BandPass 2p (old)",
		"NotchBand 2p (old)",
		"None",
		"LowPass/IT",
		"LowPass/MPT (Ext)",
		"LowPass 2p",
		"LowPass 2p",
		"HighPass 2p",
		"BandPass 2p",
		"NotchBand 2p"};
	if ((int)type < F_NUMFILTERS) {
		return names[type];
	} else {
		return "Unused";
	}
}

uintptr_t filter_numfilters(void)
{
	return (uintptr_t) F_NUMFILTERS;
}
