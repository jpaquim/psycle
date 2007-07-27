///\file
///\brief various signal processing utility functions and classes, psycle::host::Cubic amongst others.
#pragma once
#include <cmath>
#include <psycle/helpers/helpers.hpp>
#include <psycle/helpers/math/erase_all_nans_infinities_and_denormals.hpp>
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

		/// various signal processing utility functions.
		/// mixes two signals.
		inline void Add(float *pSrcSamples, float *pDstSamples, int numSamples, float vol)
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
		inline void Mul(float *pDstSamples, int numSamples, float mul)
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
		inline void MovMul(float *pSrcSamples, float *pDstSamples, int numSamples, float mul)
		{
			--pSrcSamples;
			--pDstSamples;
			do
			{
				*++pDstSamples = *++pSrcSamples*mul;
			}
			while (--numSamples);
		}
		inline void Mov(float *pSrcSamples, float *pDstSamples, int numSamples)
		{
			std::memcpy(pDstSamples, pSrcSamples, numSamples * sizeof(float));
		}
		/// zero-out a signal buffer.
		inline void Clear(float *pDstSamples, int numSamples)
		{
			std::memset(pDstSamples, 0, numSamples * sizeof(float));
		}

		/// converts a double to a std::uint32_t
		inline std::int32_t F2I(double d)
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
		inline float GetMaxVol(float *pSamplesL, float *pSamplesR, int numSamples)
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
		inline float GetMaxVSTVol(float *pSamplesL, float *pSamplesR, int numSamples)
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
		inline void Undenormalize(float *pSamplesL,float *pSamplesR, int numsamples)
		{
			#if 1
				common::math::erase_all_nans_infinities_and_denormals(pSamplesL,numsamples);
				common::math::erase_all_nans_infinities_and_denormals(pSamplesR,numsamples);
			#else
				// a 1-bit "sinus" dither
				float id(float(1.0E-18));
				for(unsigned int s(0); s < numSamples; ++s)
				{
					pSamplesL[s] += id;
					pSamplesR[s] += id;
					id = -id;
				}
			#endif
		}

		inline float dB(float amplitude) // amplitude normalized to 1.0f.
		{
			return 20.0f * std::log10f(amplitude);
		}
		inline float dB2Amp(float db)
		{
			return std::pow(10.0f,db/20.0f);
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
		typedef float (*PRESAMPLERFN)(const short *pData,  std::uint64_t offset,  std::uint32_t res,  std::uint64_t length);

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
			static float None(const short *pData,  std::uint64_t offset,  std::uint32_t res,  std::uint64_t length)
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
				case R_BANDLIM:
					_pWorkFn = Bandlimit;
					break;
				}
			}
			virtual ResamplerQuality GetQuality(void) { return _quality; }
		protected:
			/// interpolation work function which does linear interpolation.
			static float Linear(const short *pData,  std::uint64_t offset,  std::uint32_t res,  std::uint64_t length)
			{
				float y0,y1;
				y0 = *pData;
				y1 =(offset+1 == length)?0:*(pData+1);
				return (y0+(y1-y0)*_lTable[res>>21]);
			}
			/// interpolation work function which does spline interpolation.
			static float Spline(const short *pData,  std::uint64_t offset,  std::uint32_t res,  std::uint64_t length)
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


			//either or both of these can be fine-tuned to find a tolerable compromise between quality and memory/cpu usage
			//make sure any changes to SINC_RESOLUTION are reflected in Bandlimit()!
			#define SINC_RESOLUTION 512	//sinc table values per zero crossing -- keep it a power of 2!!
			#define SINC_ZEROS 11	//sinc table zero crossings (per side) -- too low and it aliases, too high uses lots of cpu.
			#define SINC_TABLESIZE SINC_RESOLUTION * SINC_ZEROS

			/// interpolation work function which does band-limited interpolation.
			static float Bandlimit(const short *pData,  std::uint64_t offset,  std::uint32_t res,  std::uint64_t length)
			{
				res = res>>23;		//!!!assumes SINC_RESOLUTION == 512!!!
				int leftExtent(SINC_ZEROS), rightExtent(SINC_ZEROS);
				if(offset<SINC_ZEROS) leftExtent=offset;
				if(length-offset<SINC_ZEROS) rightExtent=length-offset;
				
				const int sincInc(SINC_RESOLUTION);
				float newval(0.0);

				newval += sincTable[res] * *(pData);
				float sincIndex(sincInc+res);
				float weight(sincIndex - floor(sincIndex));
				for(	int i(1);
						i < leftExtent;
						++i, sincIndex+=sincInc
						)
					newval+= (sincTable[(int)sincIndex] + sincDelta[(int)sincIndex]*weight ) * *(pData-i);

				sincIndex = sincInc-res;
				weight = sincIndex - floor(sincIndex);
				for(	int i(1);
						i < rightExtent;
						++i, sincIndex+=sincInc
						)
					newval += ( sincTable[(int)sincIndex] + sincDelta[(int)sincIndex]*weight ) * *(pData+i);

				return newval;
			}

			
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
	}
}
