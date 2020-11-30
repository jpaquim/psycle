/**********************************************************************
	This file contains two different implementations of the FFT algorithm
	to calculate Power spectrums.
	Function based one is original from Dominic Mazzoni.
	Class based one is made from sources of schismtracker.
	FFTClass is a bit faster, but with a few more spikes.

	Original copyright follows:

	FFT.cpp

	Dominic Mazzoni

	September 2000

	This file contains a few FFT routines, including a real-FFT
	routine that is almost twice as fast as a normal complex FFT,
	and a power spectrum routine when you know you don't care
	about phase information.

	Some of this code was based on a free implementation of an FFT
	by Don Cross, available on the web at:

	http://www.intersrv.com/~dcross/fft.html

	The basic algorithm for his code was based on Numerican Recipes
	in Fortran.  I optimized his code further by reducing array
	accesses, caching the bit reversal table, and eliminating
	float-to-double conversions, and I added the routines to
	calculate a real FFT and a real power spectrum.

**********************************************************************/
#include "fft.h"
#include "operations.h"
#include "quantize.h"
#include "convert.h"

#include <stdlib.h>
#include <stdio.h>

#include "../../detail/portable.h"

int **gFFTBitTable = NULL;
//const int MaxFastBits = 16;
const int MaxFastBits = 12;

/* Declare Static functions */
static bool IsPowerOfTwo(int x);
static int NumberOfBitsNeeded(int PowerOfTwo);
static int ReverseBits(int index, int NumBits);
static void InitFFT();

bool IsPowerOfTwo(int x)
{
	return ((x & (x - 1))==0);
}

int NumberOfBitsNeeded(int PowerOfTwo)
{
	int i=0;
	while((PowerOfTwo >>=1)> 0) i++;
	return i;
}

int ReverseBits(int index, int NumBits)
{
	int i, rev;

	for (i = rev = 0; i < NumBits; i++) {
		rev = (rev << 1) | (index & 1);
		index >>= 1;
	}

	return rev;
}

void InitFFT()
{
	int len = 2;
	int b;
	gFFTBitTable = (int**) malloc(sizeof(int*) * MaxFastBits);

	
	for (b = 1; b <= MaxFastBits; b++) {
		int i;

		gFFTBitTable[b - 1] = (int*) malloc(sizeof(int) * len);
		for (i = 0; i < len; i++)
			gFFTBitTable[b - 1][i] = ReverseBits(i, b);
		len <<= 1;
	}
}

/*
	* Complex Fast Fourier Transform
	*/

void FFT(int NumSamples,
			bool InverseTransform,
			float *RealIn, float *ImagIn, float *RealOut, float *ImagOut)
{
	int NumBits;                 /* Number of bits needed to store indices */
	int i, j, k, n;
	int BlockSize, BlockEnd;

	double angle_numerator = 2.0 * psy_dsp_PI;
	double tr, ti;                /* temp real, temp imaginary */

	if (!IsPowerOfTwo(NumSamples)) {
		// std::ostringstream s;
		// s << "FFT called with size "<< NumSamples;
		// throw universalis::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION__NO_CLASS);
	}

	NumBits = NumberOfBitsNeeded(NumSamples);

	if (!gFFTBitTable)
		InitFFT();

	if (InverseTransform)
		angle_numerator = -angle_numerator;


	/*
	**   Do simultaneous data copy and bit-reversal ordering into outputs...
	*/
	if (NumBits <= MaxFastBits) {
		for (i = 0; i < NumSamples; i++) {
			j = gFFTBitTable[NumBits - 1][i];
			RealOut[j] = RealIn[i];
			ImagOut[j] = (float)((ImagIn == NULL) ? 0.0 : ImagIn[i]);
		}
	}
	else {
		for (i = 0; i < NumSamples; i++) {
				j = ReverseBits(i, NumBits);
			RealOut[j] = RealIn[i];
			ImagOut[j] = (float)((ImagIn == NULL) ? 0.0 : ImagIn[i]);
		}
	}

	/*
	**   Do the FFT itself...
	*/

	BlockEnd = 1;
	for (BlockSize = 2; BlockSize <= NumSamples; BlockSize <<= 1) {

		double delta_angle = angle_numerator / (double) BlockSize;

		double sm2 = sin(-2 * delta_angle);
		double sm1 = sin(-delta_angle);
		double cm2 = cos(-2 * delta_angle);
		double cm1 = cos(-delta_angle);
		double w = 2 * cm1;
		double ar0, ar1, ar2, ai0, ai1, ai2;

		for (i = 0; i < NumSamples; i += BlockSize) {
			ar2 = cm2;
			ar1 = cm1;

			ai2 = sm2;
			ai1 = sm1;

			for (j = i, n = 0; n < BlockEnd; j++, n++) {
			ar0 = w * ar1 - ar2;
			ar2 = ar1;
			ar1 = ar0;

			ai0 = w * ai1 - ai2;
			ai2 = ai1;
			ai1 = ai0;

			k = j + BlockEnd;
			tr = ar0 * RealOut[k] - ai0 * ImagOut[k];
			ti = ar0 * ImagOut[k] + ai0 * RealOut[k];

			RealOut[k] = (float)(RealOut[j] - tr);
			ImagOut[k] = (float)(ImagOut[j] - ti);

			RealOut[j] += (float) tr;
			ImagOut[j] += (float) ti;
			}
		}

		BlockEnd = BlockSize;
	}

	/*
		**   Need to normalize if inverse transform...
	*/

	if (InverseTransform) {
		float denom = (float) NumSamples;

		for (i = 0; i < NumSamples; i++) {
			RealOut[i] /= denom;
			ImagOut[i] /= denom;
		}
	}
}

/*
	* Real Fast Fourier Transform
	*
	* This function was based on the code in Numerical Recipes in C.
	* In Num. Rec., the inner loop is based on a single 1-based array
	* of interleaved real and imaginary numbers.  Because we have two
	* separate zero-based arrays, our indices are quite different.
	* Here is the correspondence between Num. Rec. indices and our indices:
	*
	* i1  <->  real[i]
	* i2  <->  imag[i]
	* i3  <->  real[n/2-i]
	* i4  <->  imag[n/2-i]
	*/

void RealFFT(int NumSamples, float *RealIn, float *RealOut, float *ImagOut)
{
	int Half = NumSamples / 2;
	int i;

	float theta = (float) (psy_dsp_PI_F / Half);

	float* tmpReal = (float*) malloc(sizeof(float) * Half);
	float *tmpImag = (float*) malloc(sizeof(float) * Half);

	for (i = 0; i < Half; i++) {
		tmpReal[i] = RealIn[2 * i];
		tmpImag[i] = RealIn[2 * i + 1];
	}

	FFT(Half, 0, tmpReal, tmpImag, RealOut, ImagOut);

	{
	float wtemp = (float) (sin(0.5 * theta));

	float wpr = (float)(-2.0 * wtemp * wtemp);
	float wpi = (float)(sin(theta));
	float wr = 1.f + wpr;
	float wi = wpi;

	int i3;

	float h1r, h1i, h2r, h2i;

	for (i = 1; i < Half / 2; i++) {

		i3 = Half - i;

		h1r = (float)(0.5f * (RealOut[i] + RealOut[i3]));
		h1i = (float)(0.5f * (ImagOut[i] - ImagOut[i3]));
		h2r = (float)(0.5f * (ImagOut[i] + ImagOut[i3]));
		h2i = (float)(-0.5f * (RealOut[i] - RealOut[i3]));

		RealOut[i] = h1r + wr * h2r - wi * h2i;
		ImagOut[i] = h1i + wr * h2i + wi * h2r;
		RealOut[i3] = h1r - wr * h2r + wi * h2i;
		ImagOut[i3] = -h1i + wr * h2i + wi * h2r;

		wr = (wtemp = wr) * wpr - wi * wpi + wr;
		wi = wi * wpr + wtemp * wpi + wi;
	}

	RealOut[0] = (h1r = RealOut[0]) + ImagOut[0];
	ImagOut[0] = h1r - ImagOut[0];

	}
	free(tmpReal);
	free(tmpImag);
}

/*
	* PowerSpectrum
	*
	* This function computes the same as RealFFT, above, but
	* adds the squares of the real and imaginary part of each
	* coefficient, extracting the power and throwing away the
	* phase.
	*
	* For speed, it does not call RealFFT, but duplicates some
	* of its code.
	*/

void PowerSpectrum(int NumSamples, float* In, float* Out)
{
	int Half = NumSamples / 2;
	int i;

	float theta = (float)(psy_dsp_PI_F / Half);

	float* tmpReal = (float*)malloc(sizeof(float) * Half);
	float* tmpImag = (float*)malloc(sizeof(float) * Half);
	float* RealOut = (float*)malloc(sizeof(float) * Half);
	float* ImagOut = (float*)malloc(sizeof(float) * Half);

	for (i = 0; i < Half; i++) {
		tmpReal[i] = In[2 * i];
		tmpImag[i] = In[2 * i + 1];
	}

	FFT(Half, 0, tmpReal, tmpImag, RealOut, ImagOut);
	{
	float wtemp = (float)(sin(0.5 * theta));

	float wpr = -2.f * wtemp * wtemp;
	float wpi = (float)(sin(theta));
	float wr = 1.f + wpr;
	float wi = wpi;

	int i3;

	float h1r, h1i, h2r, h2i, rt, it;

	for (i = 1; i < Half / 2; i++) {

		i3 = Half - i;

		h1r = 0.5f * (RealOut[i] + RealOut[i3]);
		h1i = 0.5f * (ImagOut[i] - ImagOut[i3]);
		h2r = 0.5f * (ImagOut[i] + ImagOut[i3]);
		h2i = -0.5f * (RealOut[i] - RealOut[i3]);

		rt = h1r + wr * h2r - wi * h2i;
		it = h1i + wr * h2i + wi * h2r;

		Out[i] = rt * rt + it * it;

		rt = h1r - wr * h2r + wi * h2i;
		it = -h1i + wr * h2i + wi * h2r;

		Out[i3] = rt * rt + it * it;

		wr = (wtemp = wr) * wpr - wi * wpi + wr;
		wi = wi * wpr + wtemp * wpi + wi;
	}

	rt = (h1r = RealOut[0]) + ImagOut[0];
	it = h1r - ImagOut[0];
	Out[0] = rt * rt + it * it;

	rt = RealOut[Half / 2];
	it = ImagOut[Half / 2];
	Out[Half / 2] = rt * rt + it * it;

	}
	free(tmpReal);
	free(tmpImag);
	free(RealOut);
	free(ImagOut);
}

/*
	* Windowing Functions
	*/

int NumWindowFuncs()
{
	return 4;
}

const char* WindowFuncName(int whichFunction)
{
	switch (whichFunction) {
	default:
	case 0:
		return "Rectangular";
	case 1:
		return "Bartlett";
	case 2:
		return "Hamming";
	case 3:
		return "Hanning";
	}
}

void WindowFunc(int whichFunction, int NumSamples, float *in)
{
	int i;

	if (whichFunction == 1) {
		// Bartlett (triangular) window
		for (i = 0; i < NumSamples / 2; i++) {
			in[i] *= (i / (float) (NumSamples / 2));
			in[i + (NumSamples / 2)] *=
				(1.f - (i / (float) (NumSamples / 2)));
		}
	}

	if (whichFunction == 2) {
		// Hamming
		for (i = 0; i < NumSamples; i++)
			in[i] *= (float)(0.54f - 0.46 * cos(2 * psy_dsp_PI * i / (NumSamples - 1)));
	}

	if (whichFunction == 3) {
		// Hanning
		for (i = 0; i < NumSamples; i++)
			in[i] *= (float)(0.50 - 0.50 * cos(2 * psy_dsp_PI * i / (NumSamples - 1)));
	}
}

//
///////////////////////////////////////////////////
//
	void fftclass_init(FFTClass* self)
	{
		self->bit_reverse = 0;
		self->window = 0;
		self->precos = 0;
		self->presin = 0;
		self->state_real = 0;
		self->state_imag = 0;
		self->fftLog = 0;
		psy_dsp_linearresampler_init(&self->resampler);
	}

	void fftclass_dispose(FFTClass* self)
	{
		fftclass_reset(self);
	}

	void fftclass_reset(FFTClass* self)
	{
		free(self->bit_reverse);
		dsp.memory_dealloc(self->window);
		dsp.memory_dealloc(self->precos);
		dsp.memory_dealloc(self->presin);

		dsp.memory_dealloc(self->state_real);
		dsp.memory_dealloc(self->state_imag);

		free(self->fftLog);
	}

	void fftclass_fillrectangularwindow(FFTClass* self, float window[], size_t size, float scale)
	{
		size_t n;

        for (n = 0; n < size; n++) {
			window[n] = scale;
        }
	}

	void fftclass_fillcosinewindow(FFTClass* self, float window[], size_t size, float scale)
	{
		size_t sizem1 = size-1;
		size_t n;

        for (n = 0; n < size; n++) {
			window[n] = (float)(sin(psy_dsp_PI_F * n/ sizem1) * scale);
        }
	}

	void fftclass_fillhannwindow(FFTClass* self, float window[], size_t size, float scale)
	{
		size_t sizem1 = size-1;
		const float twopi = (float)2.f*psy_dsp_PI_F;
		size_t n;

        for (n = 0; n < size; n++) {
			window[n] = (float)(0.50f - 0.50f * cos( twopi* n / sizem1)) * scale;
        }
	}

	void fftclass_fillhammingwindow(FFTClass* self, float window[], size_t size, float scale)
	{
		size_t sizem1 = size-1;
		const float twopi = 2.0f*psy_dsp_PI_F;
		size_t n;

        for (n = 0; n < size; n++) {
			window[n] = (float)(0.54f - 0.46f * cos(twopi * n / sizem1)) * scale;
        }
	}

	void fftclass_fillgaussianwindow(FFTClass* self, float window[], size_t size, float scale)
	{
		size_t sizem1 = size-1;
		size_t n;

        for (n = 0; n < size; n++) {
			window[n] = (float)(pow((float)psy_dsp_e, -0.5f *
				pow((n-sizem1/2.f)/(0.4f*sizem1/2.f),2.f)) * scale);
        }
	}

	void fftclass_fillblackmannwindow(FFTClass* self, float window[],
		size_t size, float scale)
	{
		size_t sizem1 = size-1;
		size_t n;

        for (n = 0; n < size; n++) {
			window[n] = (float)((0.42659 - 0.49656 * cos
				(2.f*psy_dsp_PI_F * n / sizem1) +
				0.076849 * cos(4.f*psy_dsp_PI_F * n /sizem1)) * scale);
        }
	}

	void fftclass_fillblackmannharriswindow(FFTClass* self, float window[],
		size_t size, float scale)
	{
		size_t sizem1 = size-1;
		size_t n;

        for (n = 0; n < size; n++) {
			window[n] = (float)((0.35875 - 0.48829 * cos(2.0f*psy_dsp_PI_F * n/ sizem1) +
				0.14128 * cos(4.f*psy_dsp_PI_F * n /sizem1) -
				0.01168 * cos(6.f*psy_dsp_PI_F * n /sizem1)) * scale);
        }
	}

	size_t fftclass_reverse_bits(FFTClass* self, size_t in)
	{
		size_t r = 0;
		size_t n = 0;

	    for (n = 0; n < self->bufferSizeLog; n++) {
			r = (r << 1) | (in & 1);
            in >>= 1;
		}
		return r;
	}
	bool fftclass_ispoweroftwo(FFTClass* self, size_t x)
	{
		return ((x & (x - 1))==0);
	}
	void fftclass_setup(FFTClass* self, FftWindowType type, size_t sizeBuf, size_t sizeBands)
	{
		if (!IsPowerOfTwo(sizeBuf)) {
			// std::ostringstream s;
			// s << "FFT called with size "<< sizeBuf;
			// throw universalis::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION__NO_CLASS);
		}
		self->bands = sizeBands;
		if (self->window == NULL || sizeBuf != self->bufferSize) {
			size_t sizetmp;
			size_t n;

			fftclass_reset(self);
			self->bufferSize = sizeBuf;
			self->outputSize = sizeBuf >> 1;
			self->bit_reverse = (size_t*) malloc(sizeof(size_t) * self->bufferSize);
			self->window = dsp.memory_alloc(self->bufferSize, sizeof(float));
			self->precos = dsp.memory_alloc(self->outputSize, sizeof(float));
			self->presin = dsp.memory_alloc(self->outputSize, sizeof(float));
			self->state_real = dsp.memory_alloc(self->bufferSize, sizeof(float));
			self->state_imag = dsp.memory_alloc(self->bufferSize, sizeof(float));
			self->fftLog = (float*) malloc(sizeof(float) * self->bands);

			self->bufferSizeLog = 0;
			for(sizetmp = self->bufferSize; sizetmp > 1; sizetmp>>=1) { self->bufferSizeLog++; } 
			for (n = 0; n < self->bufferSize; n++) {
				self->bit_reverse[n] = fftclass_reverse_bits(self, n);
			}
			for (n = 0; n < self->outputSize; n++) {
				float j = 2.0f * psy_dsp_PI_F* n / self->bufferSize;
				self->precos[n] = (float)cos(j);
				self->presin[n] = (float)sin(j);
			}
		}
		switch(type){
			case rectangular: fftclass_fillrectangularwindow(self, self->window, self->bufferSize, 1.0f);break;
			case cosine: fftclass_fillcosinewindow(self, self->window, self->bufferSize, 1.0f);break;
			case hann: fftclass_fillhannwindow(self, self->window, self->bufferSize, 1.0f);break;
			case hamming: fftclass_fillhammingwindow(self, self->window, self->bufferSize, 1.0f);break;
			case gaussian: fftclass_fillgaussianwindow(self, self->window, self->bufferSize, 1.0f);break;
			case blackmann: fftclass_fillblackmannwindow(self, self->window, self->bufferSize, 1.0f);break;
			case blackmannHarris: fftclass_fillblackmannharriswindow(self, self->window, self->bufferSize, 1.0f);break;
			default:break;
		}
		if ( self->outputSize/self->bands == 1 ) {
			size_t n;

			for (n = 0; n < self->bands; n++ ) {
				self->fftLog[n]=(float)n;
			}
		}
		else if (self->outputSize/self->bands <= 4 ) {
			//exponential.
			const float factor = (float)self->outputSize/(self->bands*self->bands);
			size_t n;

			for (n = 0; n < self->bands; n++ ) {
				self->fftLog[n]=n*n*factor;
			}
		}
		else {
			//constant note scale.
			//factor -> set range from 2^0 to 2^8.
			//factor2 -> scale the result to the FFT output size
			//Note: x^(y*z) = (x^z)^y
			const float factor = 8.f/(float)self->bands;
			const float factor2 = (float)self->outputSize/256.f;
			size_t n;

			for (n = 0; n < self->bands; n++ ) {
				self->fftLog[n]= (float)(pow(2.0f,n*factor)-1.f)*factor2;
			}
		}
	}

	void fftclass_calculatespectrum(FFTClass* self, float samplesIn[], float samplesOut[])
	{
        unsigned int n, k, y;
        unsigned int ex, ff;
        float fr, fi;
        float tr, ti;
        int yp;

        /* fft */
        float *rp = self->state_real;
        float *ip = self->state_imag;
        for (n = 0; n < self->bufferSize; n++) {
			const size_t nr = self->bit_reverse[n];
            *rp++ = samplesIn[ nr ] * self->window[nr];
            *ip++ = 0;
        }
        ex = 1;
        ff = self->outputSize;
        for (n = self->bufferSizeLog; n != 0; n--) {
			for (k = 0; k != ex; k++) {
				fr = self->precos[k * ff];
				fi = self->presin[k * ff];
                for (y = k; y < self->bufferSize; y += ex << 1) {
                   yp = y + ex;
                   tr = fr * self->state_real[yp] - fi * self->state_imag[yp];
                   ti = fr * self->state_imag[yp] + fi * self->state_real[yp];
				   self->state_real[yp] = self->state_real[y] - tr;
				   self->state_imag[yp] = self->state_imag[y] - ti;
				   self->state_real[y] += tr;
				   self->state_imag[y] += ti;
                }
            }
            ex <<= 1;
            ff >>= 1;
        }

        /* collect fft */
        rp = self->state_real; rp++;
        ip = self->state_imag; ip++;
        for (n = 0; n < self->outputSize; n++) {
            samplesOut[n] = ((*rp) * (*rp)) + ((*ip) * (*ip));
            rp++;ip++;
        }
	}

	void fftclass_fillbandsfromfft(FFTClass* self, float calculatedfftIn[], float banddBOut[])
	{

		float j=0.0f;
		const float dbinvSamples = psy_dsp_convert_amp_to_db(1.0f/(self->bufferSize>>2));
		unsigned int h;
		unsigned int a;
		dsp.erase_all_nans_infinities_and_denormals(calculatedfftIn, self->outputSize);
        for (h=0, a=0;h< self->bands;h++)
		{
			float afloat = self->fftLog[h];
			/*if (afloat < 1.f) {
				//This was intended as a DC bar, but it isn't only DC.
				j = calculatedfftIn[a];
			}
			else*/ if (afloat +1.0f > self->fftLog[h+1]) {
				j = psy_dsp_resampler_work_float(
					psy_dsp_linearresampler_base(&self->resampler),
					calculatedfftIn, afloat, self->outputSize,
					calculatedfftIn, calculatedfftIn + self->outputSize - 1);				
				j = calculatedfftIn[(int)afloat];
				a = psy_dsp_fround(afloat);
			}
			else {
				j = calculatedfftIn[a];
				while(a<=afloat){
					j = psy_max(j,calculatedfftIn[a]);
					a++;
				}
			}//+0.0000000001f is -100dB of power. Used to prevent evaluating powerdB(0.0)
			banddBOut[h] = (float)(10 * log10(j + 0.0000000001f) + dbinvSamples);
		}
	}

	float fftclass_resample(FFTClass* self, float const * data, float offset, uint64_t length)
	{
		return psy_dsp_resampler_work_float(
			psy_dsp_linearresampler_base(&self->resampler),
			data, offset, length, data, data + length - 1);
	}
