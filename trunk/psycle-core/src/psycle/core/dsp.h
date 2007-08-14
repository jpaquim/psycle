///\file
///\brief various signal processing utility functions and classes, psy::core::Cubic amongst others. based on revison  2663
#pragma once
#include <cmath>
#include "helpers.h"
#include "cstdint.h"
namespace psy
{
	namespace core
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
			union tmp_union
			{
				double d;
				int i;
			} tmp;
			tmp.d = (d-0.5) + magic;
			return tmp.i;
		}

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
		#if 0
		/// finds the maximum amplitude in a signal buffer.
		/// It contains "VST" because initially the return type for native machines 
		/// was int. Now, GetMaxVSTVol, and both *Acurate() functions are deprecated.
		static inline float GetMaxVSTVol(float *pSamplesL, float *pSamplesR, int numSamples)
		{
			return GetMaxVol(pSamplesL,pSamplesR,numSamples);
		}
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
					const float volL = std::fabsf(*++pSamplesL); // not all waves are symmetrical
					const float volR = std::fabsf(*++pSamplesR);
					if (volL > vol) vol = volL;
					if (volR > vol) vol = volR;
				}
				while(--numSamples);
				return vol;
			}
		#endif

		/// Cure for malicious samples
		/// Type : Filters Denormals, NaNs, Infinities
		/// References : Posted by urs[AT]u-he[DOT]com
		static void erase_All_NaNs_Infinities_And_Denormals( float* inSamples, int inNumberOfSamples )
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

		/// Cure for malicious samples
		/// Type : Filters Denormals, NaNs, Infinities
		static inline void Undenormalize(float *pSamplesL,float *pSamplesR, int numsamples)
		{
			#if 1
				erase_All_NaNs_Infinities_And_Denormals(pSamplesL,numsamples);
				erase_All_NaNs_Infinities_And_Denormals(pSamplesR,numsamples);
			#else
				// a 1-bit "sinus" dither
				float id(float(1.0E-18));
				pSamplesL[s] += id;
				pSamplesR[s] += id;
				id = -id;  // <magnus> This statement has no effect.
			#endif
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
				R_SPLINE,
				R_BANDLIM
			};

		/// interpolator work function.
		///\todo typdef should be inside the Resampler class itself.
		typedef float (*PRESAMPLERFN)(const short *pData, std::uint64_t offset, std::uint32_t res, std::uint64_t length);

		/// sample interpolator.
		class Resampler
		{
		public:
			/// constructor
			Resampler()
			{ 
				_quality = R_NONE;
				_pWorkFn = T_None;
			};
			virtual ~Resampler() {}

			/// work function corresponding to the selected kind.
			PRESAMPLERFN _pWorkFn;
			/// sets the kind of interpolation.
			virtual void SetQuality(ResamplerQuality quality) = 0;
			virtual ResamplerQuality GetQuality(void) const = 0;
		protected:
			/// kind of interpolation.
			ResamplerQuality _quality;
			/// interpolation work function which does nothing.
			static float T_None(const short *pData, std::uint64_t offset, std::uint32_t res, std::uint64_t length);
		};

		/// cubic sample interpolator.
		class Cubic : public Resampler
		{
		public:
			/// constructor.
			Cubic();
			virtual ~Cubic() {}
			/// refefinition.
			virtual void SetQuality(ResamplerQuality quality);
			virtual ResamplerQuality GetQuality(void) const;
		protected:
			/// interpolation work function which does linear interpolation.
			static float Linear(const short *pData, std::uint64_t offset, std::uint32_t res, std::uint64_t length);
			/// interpolation work function which does spline interpolation.
			static float Spline(const short *pData, std::uint64_t offset, std::uint32_t res, std::uint64_t length);

			//either or both of these can be fine-tuned to find a tolerable compromise between quality and memory/cpu usage
			//make sure any changes to SINC_RESOLUTION are reflected in Bandlimit()!
			
			/// sinc table values per zero crossing -- keep it a power of 2!!
			#define SINC_RESOLUTION 512
			/// sinc table zero crossings (per side) -- too low and it aliases, too high uses lots of cpu.
			#define SINC_ZEROS 11 
			
			#define SINC_TABLESIZE SINC_RESOLUTION * SINC_ZEROS

			/// interpolation work function which does band-limited interpolation.
			static float Bandlimit(const short *pData, std::uint64_t offset, std::uint32_t res, std::uint64_t length);
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

			//sinc function table
			static float sincTable[SINC_TABLESIZE];
			//table of deltas between sincTable indices.. sincDelta[i] = sincTable[i+1] - sincTable[i]
			//used to speed up linear interpolation of sinc table-- this idea stolen from libresampler
			static float sincDelta[SINC_TABLESIZE];
			//note: even with this optimization, interpolating the sinc table roughly doubles the cpu usage on my machine.
			// since we're working in realtime here, it may be best to just make SINC_RESOLUTION a whole lot bigger, and drop
			// the table interpolation altogether..
		};
	}
}}
