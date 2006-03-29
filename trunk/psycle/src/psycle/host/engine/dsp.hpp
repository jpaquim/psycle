///\file
///\brief various signal processing utility functions and classes, psycle::host::Cubic amongst others.
#pragma once
#include <cmath>
#include <psycle/host/engine/helpers.hpp>
namespace psycle
{
	namespace host
	{
		namespace dsp
		{
#if PSYCLE__CONFIGURATION__RMS_VUS
			extern int numRMSSamples;
			extern int countRMSSamples;
			extern double RMSAccumulatedLeft;
			extern double RMSAccumulatedRight;
			extern float previousRMSLeft;
			extern float previousRMSRight;
#endif
		/// Funky denormal check
		#define IS_DENORMAL(f) (!((*(unsigned int *)&f)&0x7f800000))	

		/// various signal processing utility functions.
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
		static inline void Mov(float *pSrcSamples, float *pDstSamples, int numSamples)
		{
			std::memcpy(pDstSamples, pSrcSamples, numSamples * sizeof(float));
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
		static inline float GetMaxVol(float *pSamplesL, float *pSamplesR, int numSamples)
		{
#if PSYCLE__CONFIGURATION__RMS_VUS
			// This is just a test to get RMS dB values.
			// Doesn't look that better, and uses more CPU. 
			float *pL = pSamplesL;
			float *pR = pSamplesR;
			int ns = numSamples;
			int count =(numRMSSamples- countRMSSamples);
			--pL;
			--pR;
			if ( ns >= count)
			{
				ns -= count;
				while (count--) {
					RMSAccumulatedLeft +=  *(++pL)**(pL);
					RMSAccumulatedRight +=  *(++pR)**(pR);
				};
				previousRMSLeft =  sqrt(dsp::RMSAccumulatedLeft/dsp::numRMSSamples);
				previousRMSRight =  sqrt(dsp::RMSAccumulatedRight/dsp::numRMSSamples);
				RMSAccumulatedLeft = 0;
				RMSAccumulatedRight = 0;
				countRMSSamples = 0;
			}
			while(ns--) {
				RMSAccumulatedLeft +=  *(++pL)**(pL);
				RMSAccumulatedRight +=  *(++pR)**(pR);
				countRMSSamples++;
			};
			return previousRMSLeft>previousRMSRight?previousRMSLeft:previousRMSRight;

#else
			// This is the usual code, peak value
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
#endif
		}
		/// finds the maximum amplitude in a signal buffer.
		/// It contains "VST" because initially the return type for native machines 
		/// was int. Now, GetMaxVSTVol, and both *Acurate() functions are deprecated.
		static inline float GetMaxVSTVol(float *pSamplesL, float *pSamplesR, int numSamples)
		{
			return GetMaxVol(pSamplesL,pSamplesR,numSamples);
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
		/***********************************************************************
		Cure for malicious samples
		Type : Filters Denormals, NaNs, Infinities
		References : Posted by urs[AT]u-he[DOT]com
		***********************************************************************/
		static void erase_All_NaNs_Infinities_And_Denormals( float* inSamples, int const & inNumberOfSamples )
		{
			unsigned int* inArrayOfFloats = (unsigned int*) inSamples;
			unsigned int sample;
			unsigned int exponent;
			for ( int i = 0; i < inNumberOfSamples; i++ )
			{
				sample = *inArrayOfFloats;
				exponent = sample & 0x7F800000;

				// exponent < 0x7F800000 is 0 if NaN or Infinity, otherwise 1
				// exponent > 0 is 0 if denormalized, otherwise 1

				*inArrayOfFloats++ = sample * ((exponent < 0x7F800000) & (exponent > 0));
			}
		}
		/// undenormalize (renormalize) samples in a signal buffer.
		///\todo make a template version that accept both float and doubles
		static inline void Undenormalize(float *pSamplesL,float *pSamplesR, int numsamples)
		{
/*			float id(float(1.0E-18));
			for(int s(0) ; s < numsamples ; ++s)
			{
//			Old denormal code. Now we use a 1bit sinus.
//				if(IS_DENORMAL(pSamplesL[s])) pSamplesL[s] = 0;
//				if(IS_DENORMAL(pSamplesR[s])) pSamplesR[s] = 0;
//				const float is1=pSamplesL[s];
//				const float is2=pSamplesR[s];
//				pSamplesL[s] = IS_DENORMAL(is1) ? 0 : is1;
//				pSamplesR[s] = IS_DENORMAL(is2) ? 0 : is2;
				pSamplesL[s] += id;
				pSamplesR[s] += id;
				id = - id;
			}
*/
			erase_All_NaNs_Infinities_And_Denormals(pSamplesL,numsamples);
			erase_All_NaNs_Infinities_And_Denormals(pSamplesR,numsamples);

		}

		static inline float dB(float amplitude) // amplitude normalized to 1.0f.
		{
			return 20.0f * log10f(amplitude);
		}
		static inline float dB2Amp(float db)
		{
			return pow(10.0f,db/20.0f);
		}

		/// sample interpolator kinds.
		///\todo typdef should be inside the Resampler or Cubic class itself.
			enum ResamplerQuality
			{
				R_NONE  = 0,
				R_LINEAR,
				R_SPLINE
			};

		/// interpolator work function.
		///\todo typdef should be inside the Resampler class itself.
		typedef float (*PRESAMPLERFN)(const short *pData, unsigned __int64 offset, unsigned __int32 res, unsigned __int64 length);

		/// sample interpolator.
		class Resampler
		{
		public:
			/// constructor
			Resampler()
			{ 
				_quality = R_NONE;
				_pWorkFn = None;
			};
			/// work function corresponding to the selected kind.
			PRESAMPLERFN _pWorkFn;
			/// sets the kind of interpolation.
			virtual void SetQuality(ResamplerQuality quality) = 0;
			virtual ResamplerQuality GetQuality(void) = 0;
		protected:
			/// kind of interpolation.
			ResamplerQuality _quality;
			/// interpolation work function which does nothing.
			static float None(const short *pData, unsigned __int64 offset, unsigned __int32 res, unsigned __int64 length)
			{
				return *pData;
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
					case R_NONE:
					_pWorkFn = None;
					break;
				case R_LINEAR:
					_pWorkFn = Linear;
					break;
				case R_SPLINE:
					_pWorkFn = Spline;
					break;
				}
			}
			virtual ResamplerQuality GetQuality(void) { return _quality; }
		protected:
			/// interpolation work function which does linear interpolation.
			static float Linear(const short *pData, unsigned __int64 offset, unsigned __int32 res, unsigned __int64 length)
			{
				float y0,y1;
				y0 = *pData;
				y1 =(offset+1 == length)?0:*(pData+1);
				return (y0+(y1-y0)*_lTable[res>>21]);
			}
			/// interpolation work function which does spline interpolation.
			static float Spline(const short *pData, unsigned __int64 offset, unsigned __int32 res, unsigned __int64 length)
			{
				float yo, y0,y1, y2;
				res = res >> 21;
			
				yo=(offset==0)?0:*(pData-1);
				y0=*(pData);
				y1=(offset+1 == length)?0:*(pData+1);
				y2=(offset+2 == length)?0:*(pData+2);
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
			/// 
			#define CUBIC_RESOLUTION 2048
			static float _aTable[CUBIC_RESOLUTION];
			static float _bTable[CUBIC_RESOLUTION];
			static float _cTable[CUBIC_RESOLUTION];
			static float _dTable[CUBIC_RESOLUTION];
			static float _lTable[CUBIC_RESOLUTION];
		};
		}
	}
}
