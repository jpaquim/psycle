#ifndef _DSP_H
#define _DSP_H

// Various signal processing utility functions and classes
//
#include <math.h>

#define CUBIC_RESOLUTION 2048

#define IS_DENORMAL(f) (!((*(unsigned int *)&f)&0x7f800000))	// Funky denormal check


class Dsp
{
public:
	static inline void Add(float *pSrcSamples, float *pDstSamples, int numSamples, float vol)
	{
		--pSrcSamples;
		--pDstSamples;
		do
		{
			*++pDstSamples += *++pSrcSamples*vol;
		}
		while (--numSamples);
	}
	
	static inline void Mul(float *pDstSamples, int numSamples, float mul)
	{
		--pDstSamples;
		do
		{
			*++pDstSamples *= mul;
		}
		while (--numSamples);
	}
	
	static inline void MovMul(float *pSrcSamples, float *pDstSamples, int numSamples, float mul)
	{
		--pSrcSamples;
		--pDstSamples;
		do
		{
			*++pDstSamples = *++pSrcSamples*mul;
		}
		while (--numSamples);
	}
	
	static inline void Clear(float *pDstSamples, int numSamples)
	{
		memset(pDstSamples,0,numSamples*sizeof(float));
	}

	static inline int F2I(double d)
	{
		const double magic = 6755399441055744.0; // 2^51 + 2^52
		double tmp = (d-0.5) + magic;
		return *(int*) &tmp;
	};

	static inline float GetMaxVSTVol(float *pSamplesL, float *pSamplesR, int numSamples)
	{
		--pSamplesL;
		--pSamplesR;
		
		float vol = 0.0f;
		do
		{
			const float volL = fabsf(*++pSamplesL); // not all waves are symmetrical
			const float volR = fabsf(*++pSamplesR);
			
			if (volL > vol)
			{
				vol = volL;
			}
			if (volR > vol)
			{
				vol = volR;
			}
		}
		while (--numSamples);
		
		return vol;
	}
	static inline int GetMaxVolAccurate(float *pSamplesL, float *pSamplesR, int numSamples)
	{
		return F2I(GetMaxVSTVolAccurate(pSamplesL,pSamplesR,numSamples));
	}
	static inline float GetMaxVSTVolAccurate(float *pSamplesL, float *pSamplesR, int numSamples)
	{
		--pSamplesL;
		--pSamplesR;
		
		float vol = 0.0f;
		do
		{
			const float volL = fabsf(*++pSamplesL); // not all waves are symmetrical
			const float volR = fabsf(*++pSamplesR);
			
			if (volL > vol)
			{
				vol = volL;
			}
			else
			{
				float const nvolL = -volL;
				if (nvolL > vol ) vol = nvolL;
			}
			
			if (volR > vol)
			{
				vol = volR;
			}
			else
			{
				float const nvolR = -volR;
				if (nvolR > vol ) vol = nvolR;
			}
		}
		while (--numSamples);
		
		return vol;
	}
	static inline int GetMaxVol(float *pSamplesL, float *pSamplesR, int numSamples)
	{
		return F2I(GetMaxVSTVol(pSamplesL,pSamplesR,numSamples));
	}
	static inline void Undenormalize(float *pSamplesL,float *pSamplesR, int numsamples)
	{
		for(int s=0;s<numsamples;s++)
		{
			if ( IS_DENORMAL(pSamplesL[s]) ) pSamplesL[s]=0.0f;
			if ( IS_DENORMAL(pSamplesR[s]) ) pSamplesR[s]=0.0f;
//			float const is1=pSamplesL[s];
//			float const is2=pSamplesR[s];
//			pSamplesL[s] = IS_DENORMAL(is1) ? 0.0f : is1;
//			pSamplesR[s] = IS_DENORMAL(is2) ? 0.0f : is2;
		}
	}
};

// CUBIC SPLINE FUNCTIONS
//

typedef enum
{
	RESAMPLE_NONE,
	RESAMPLE_LINEAR,
	RESAMPLE_SPLINE,
}
ResamplerQuality;

typedef float (*PRESAMPLERFN)(float yo,float y0,float y1,float y2,unsigned __int32 res,  unsigned __int64 offset, unsigned __int64 length);

class Resampler
{
public:
	ResamplerQuality _quality;
	PRESAMPLERFN _pWorkFn;

	Resampler()
	{ 
		_quality = RESAMPLE_LINEAR;
		_pWorkFn = None;
	};
	virtual void SetQuality(ResamplerQuality quality) = 0;
protected:
	static float None(float yo,float y0,float y1,float y2,unsigned __int32 res,  unsigned __int64 offset, unsigned __int64 length)
	{
		return y0;
	}
};

class Cubic : public Resampler
{
public:

	Cubic();
	virtual void SetQuality(ResamplerQuality quality)
	{
		_quality = quality;
		switch (quality)
		{
		case RESAMPLE_NONE:
			_pWorkFn = None;
			break;
		case RESAMPLE_LINEAR:
			_pWorkFn = Linear;
			break;
		case RESAMPLE_SPLINE:
			_pWorkFn = Spline;
			break;
		}
	}

protected:
	static float Linear(float yo,float y0,float y1,float y2,unsigned __int32 res,  unsigned __int64 offset, unsigned __int64 length)
	{
		if (offset+2 > length)
		{
			y1 = 0;
		}
		return (y0+(y1-y0)*_lTable[res>>21]);
	}
	
	static float Spline(float yo,float y0,float y1,float y2,unsigned __int32 res,  unsigned __int64 offset, unsigned __int64 length)
	{
		res = res >> 21;
		if (offset == 0)
		{
			yo = 0;
		}
		if (offset+2 > length)
		{
			y1 = 0;
		}
		if (offset+3 > length)
		{
			y2 = 0;
		}
		return (_aTable[res]*yo+_bTable[res]*y0+_cTable[res]*y1+_dTable[res]*y2);
	}
	
	// yo = y[-1] [sample at x-1]
	// y0 = y[0]  [sample at x (input)]
	// y1 = y[1]  [sample at x+1]
	// y2 = y[2]  [sample at x+2]
	
	// res= distance between two neighboughing sample points [y0 and y1] 
	//		,so [0...1.0]. You have to multiply this distance * RESOLUTION used
	//		on the spline conversion table. [2048 by default]
	// If you are using 2048 is asumed you are using 12 bit decimal
	// fixed point offsets for resampling.
	
	// offset = sample offset [info to avoid go out of bounds on sample reading ]
	// length = sample length [info to avoid go out of bounds on sample reading ]
	
private:
	static int _resolution; // Currently is 2048
	static float _aTable[2048];
	static float _bTable[2048];
	static float _cTable[2048];
	static float _dTable[2048];
	static float _lTable[2048];
};

#endif