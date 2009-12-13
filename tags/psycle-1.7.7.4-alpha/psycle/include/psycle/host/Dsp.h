#pragma once
#include <cmath>
#include "Helpers.h"
///\file
///\brief various signal processing utility functions and classes, psycle::host::Cubic amongst others.
namespace psycle
{
	namespace host
	{
		/// Funky denormal check
		#define IS_DENORMAL(f) (!((*(unsigned int *)&f)&0x7f800000))	

		/// various signal processing utility functions.
		class Dsp
		{
		public:
			/// mixes two signals.
			static inline void Add(float *pSrcSamples, float *pDstSamples, int numSamples, float vol)
			{
				--pSrcSamples;
				--pDstSamples;
				do
				{
					*++pDstSamples += *++pSrcSamples * vol;
				}
				while (--numSamples);
			}
			/// multiply a signal by a ratio, inplace.
			///\see MovMul()
			static inline void Mul(float *pDstSamples, int numSamples, float mul)
			{
				--pDstSamples;
				do
				{
					*++pDstSamples *= mul;
				}
				while (--numSamples);
			}
			/// multiply a signal by a ratio.
			///\see Mul()
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
			/// zero-out a signal buffer.
			static inline void Clear(float *pDstSamples, int numSamples)
			{
				std::memset(pDstSamples, 0, numSamples * sizeof(float));
			}
			/// converts a double to an int.
			static inline int F2I(double d)
			{
				const double magic(6755399441055744.0); /// 2^51 + 2^52
				/*const*/ double tmp((d-0.5) + magic);
				return *reinterpret_cast<int*>(&tmp);
			};
			/// finds the maximum amplitude in a signal buffer.
			/// why is that specific to vst?
			static inline float GetMaxVSTVol(float *pSamplesL, float *pSamplesR, int numSamples)
			{
				--pSamplesL;
				--pSamplesR;
				
				float vol = 0.0f;
				do
				{
					/// not all waves are symmetrical
					const float volL = fabsf(*++pSamplesL);
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
			/*
			static inline int GetMaxVolAccurate(float *pSamplesL, float *pSamplesR, int numSamples)
			{
				return f2i(GetMaxVSTVolAccurate(pSamplesL,pSamplesR,numSamples));
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
					
					if (volR > vol)
					{
						vol = volR;
					}
				}
				while (--numSamples);
				
				return vol;
			}
			*/
			/// finds the maximum amplitude in a signal buffer.
			static inline float GetMaxVol(float *pSamplesL, float *pSamplesR, int numSamples)
			{
				return GetMaxVSTVol(pSamplesL,pSamplesR,numSamples); // F2I is for doubles, f2i is for floats
			}
			/// undenormalize (renormalize) samples in a signal buffer.
			///\todo make a template version that accept both float and doubles
			static inline void Undenormalize(float *pSamplesL,float *pSamplesR, int numsamples)
			{
				float id(float(1.0E-20));
				for(int s(0) ; s < numsamples ; ++s)
				{
					/*
					if(IS_DENORMAL(pSamplesL[s])) pSamplesL[s] = 0;
					if(IS_DENORMAL(pSamplesR[s])) pSamplesR[s] = 0;
					const float is1=pSamplesL[s];
					const float is2=pSamplesR[s];
					pSamplesL[s] = IS_DENORMAL(is1) ? 0 : is1;
					pSamplesR[s] = IS_DENORMAL(is2) ? 0 : is2;
					*/
					pSamplesL[s] += id;
					pSamplesR[s] += id;
					id = - id;
				}
			}
		};

		/// sample interpolator kinds.
		///\todo typdef should be inside the Resampler or Cubic class itself.
		typedef enum
		{
			RESAMPLE_NONE,
			RESAMPLE_LINEAR,
			RESAMPLE_SPLINE,
		}
		ResamplerQuality;

		/// interpolator work function.
		///\todo typdef should be inside the Resampler class itself.
		typedef float (*PRESAMPLERFN)(float yo,float y0,float y1,float y2,unsigned __int32 res,  unsigned __int64 offset, unsigned __int64 length);

		/// sample interpolator.
		class Resampler
		{
		public:
			/// constructor
			Resampler()
			{ 
				_quality = RESAMPLE_NONE;
				_pWorkFn = None;
			};
			/// kind of interpolation.
			ResamplerQuality _quality;
			/// work function corresponding to the selected kind.
			PRESAMPLERFN _pWorkFn;
			/// sets the kind of interpolation.
			virtual void SetQuality(ResamplerQuality quality) = 0;
		protected:
			/// interpolation work function which does nothing.
			static float None(float yo,float y0,float y1,float y2,unsigned __int32 res,  unsigned __int64 offset, unsigned __int64 length)
			{
				return y0;
			}
		};

		/// cubic sample interpolator.
		class Cubic : public Resampler
		{
		public:
			/// constructor.
			Cubic();
			/// refefinition.
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
			/// interpolation work function which does linear interpolation.
			static float Linear(float yo,float y0,float y1,float y2,unsigned __int32 res,  unsigned __int64 offset, unsigned __int64 length)
			{
				if (offset+2 > length)
				{
					y1 = 0;
				}
				return (y0+(y1-y0)*_lTable[res>>21]);
			}
			/// interpolation work function which does spline interpolation.
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
			/// Currently is 2048
			static int _resolution;
			/// we like global space pollution, we also like to shout in capitals.
			#define CUBIC_RESOLUTION 2048
			static float _aTable[CUBIC_RESOLUTION];
			static float _bTable[CUBIC_RESOLUTION];
			static float _cTable[CUBIC_RESOLUTION];
			static float _dTable[CUBIC_RESOLUTION];
			static float _lTable[CUBIC_RESOLUTION];
		};
	}
}