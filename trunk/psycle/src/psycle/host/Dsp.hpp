///\file
///\brief various signal processing utility functions and classes, psycle::host::Cubic amongst others.
#pragma once
#include <cmath>
#include "Helpers.hpp"
//#include <dspguru\resamp.h>
//#include <dspguru\interp25.inc>
namespace psycle
{
	namespace host
	{
		namespace dsp
		{
#if defined PSYCLE__CONFIGURATION__RMS_VUS
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
		#if defined DIVERSALIS__PROCESSOR__X86 && defined DIVERSALIS__COMPILER__MICROSOFT
			// This code assumes aligned memory (to 16) and assigned by powers of 4!
			_asm
			{
				movss xmm2, vol
				shufps xmm2, xmm2, 0H
				mov esi, pSrcSamples
				mov edi, pDstSamples
				mov eax, [numSamples]
			LOOPSTART:
				cmp eax, 0
				jle END
				movaps xmm0, [esi]
				movaps xmm1, [edi]
				mulps xmm0, xmm2
				addps xmm0, xmm1
				movaps [edi], xmm0
				add esi, 10H
				add edi, 10H
				sub eax, 4
				jmp LOOPSTART
			END:
			}
		#else
			--pSrcSamples;
			--pDstSamples;
			do
			{
				*++pDstSamples += *++pSrcSamples * vol;
			}
			while (--numSamples);
		#endif
		}
		/// multiply a signal by a ratio, inplace.
		///\see MovMul()
		static inline void Mul(float *pDstSamples, int numSamples, float multi)
		{
		#if defined DIVERSALIS__PROCESSOR__X86 && defined DIVERSALIS__COMPILER__MICROSOFT
			// This code assumes aligned memory (to 16) and assigned by powers of 4!
			_asm
			{
				movss xmm2, multi
				shufps xmm2, xmm2, 0H
				mov edi, pDstSamples
				mov eax, [numSamples]
			LOOPSTART:
				cmp eax, 0
				jle END
				movaps xmm0, [edi]
				mulps xmm0, xmm2
				movaps [edi], xmm0
				add edi, 10H
				sub eax, 4
				jmp LOOPSTART
			END:
			}
		#else
			--pDstSamples;
			do
			{
				*++pDstSamples *= mul;
			}
			while (--numSamples);
		#endif
		}
		/// multiply a signal by a ratio.
		///\see Mul()
		static inline void MovMul(float *pSrcSamples, float *pDstSamples, int numSamples, float multi)
		{
		#if defined DIVERSALIS__PROCESSOR__X86 && defined DIVERSALIS__COMPILER__MICROSOFT
			// This code assumes aligned memory (to 16) and assigned by powers of 4!
			_asm
			{
				movss xmm2, multi
				shufps xmm2, xmm2, 0H
				mov esi, pSrcSamples
				mov edi, pDstSamples
				mov eax, [numSamples]
			LOOPSTART:
				cmp eax, 0
				jle END
				movaps xmm0, [esi]
				mulps xmm0, xmm2
				movaps [edi], xmm0
				add esi, 10H
				add edi, 10H
				sub eax, 4
				jmp LOOPSTART
			END:
			}
		#else
			--pSrcSamples;
			--pDstSamples;
			do
			{
				*++pDstSamples = *++pSrcSamples*mul;
			}
			while (--numSamples);
		#endif
		}
		static inline void Mov(float *pSrcSamples, float *pDstSamples, int numSamples)
		{
		#if defined DIVERSALIS__PROCESSOR__X86 && defined DIVERSALIS__COMPILER__MICROSOFT
			// This code assumes aligned memory (to 16) and assigned by powers of 4!
			_asm
			{
				mov esi, pSrcSamples
				mov edi, pDstSamples
				mov eax, [numSamples]
			LOOPSTART:
				cmp eax, 0
				jle END
				movaps xmm0, [esi]
				movaps [edi], xmm0
				add esi, 10H
				add edi, 10H
				sub eax, 4
				jmp LOOPSTART
			END:
			}
		#else
			std::memcpy(pDstSamples, pSrcSamples, numSamples * sizeof(float));
		#endif

		}
		/// zero-out a signal buffer.
		static inline void Clear(float *pDstSamples, int numSamples)
		{
		#if defined DIVERSALIS__PROCESSOR__X86 && defined DIVERSALIS__COMPILER__MICROSOFT
			// This code assumes aligned memory (to 16) and assigned by powers of 4!
			_asm
			{
				xorps xmm0, xmm0
				mov edi, pDstSamples
				mov eax, [numSamples]
			LOOPSTART:
				cmp eax, 0
				jle END
				movaps [edi], xmm0
				add edi, 10H
				sub eax, 4
				jmp LOOPSTART
			END:
			}
		#else
			std::memset(pDstSamples, 0, numSamples * sizeof(float));
		#endif
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
#if defined PSYCLE__CONFIGURATION__RMS_VUS
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
/*			--pSamplesL;
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
*/

		// If anyone knows better assembler than me improve this variable utilization:
		float volmax = 0.0f, volmin = 0.0f;
		float *volmaxb = &volmax, *volminb = &volmin;
		_asm
		{
			// we store the max in xmm0 and the min in xmm1
			xorps xmm0, xmm0
			xorps xmm1, xmm1
			mov esi, [pSamplesL]
			mov edi, [pSamplesR]
			mov eax, [numSamples]
			// Loop does: get the 4 max values and 4 min values in xmm0 and xmm1 respct.
		LOOPSTART:
			cmp eax, 0
			jle END
			maxps xmm0,[esi]
			maxps xmm0,[edi]
			minps xmm1,[esi]
			minps xmm1,[edi]
			add esi, 10H
			add edi, 10H
			sub eax, 4
			jmp LOOPSTART
		END:
			// to finish, get the max and of each of the four values.
			// put 02 and 03 to 20 and 21
			movhlps xmm2, xmm0
			// find max of 00 and 20 (02) and of 01 and 21 (03)
			maxps xmm0, xmm2
			// put 00 (result of max(00,02)) to 20
			movss xmm2, xmm0
			// put 01 (result of max(01,03)) into 00 (that's the only one we care about)
			shufps xmm0, xmm2, 11H
			// and find max of 00 (01) and 20 (00)
			maxps xmm0, xmm2

			movhlps xmm2, xmm1
			minps xmm1, xmm2
			movss xmm2, xmm1
			shufps xmm1, xmm2, 11H
			minps xmm1, xmm2

			mov edi, volmaxb
			movss [edi], xmm0
			mov edi, volminb
			movss [edi], xmm1
		}
		volmin*=-1.0f;
		return (volmax>volmin)?volmax:volmin;

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
				R_SPLINE,
				R_BANDLIM
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
				case R_BANDLIM:
					_pWorkFn = Bandlimit;
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
			

			//either or both of these can be fine-tuned to find a tolerable compromise between quality and memory/cpu usage
			//make sure any changes to SINC_RESOLUTION are reflected in Bandlimit()!
			#define SINC_RESOLUTION 512	//sinc table values per zero crossing -- keep it a power of 2!!
			#define SINC_ZEROS 12	//sinc table zero crossings (per side) -- too low and it aliases, too high uses lots of cpu.
			#define SINC_TABLESIZE SINC_RESOLUTION * SINC_ZEROS

			/// interpolation work function which does band-limited interpolation.
			static float Bandlimit(const short *pData, unsigned __int64 offset, unsigned __int32 res, unsigned __int64 length)
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


			/****************************************************************************/
/*
			static float FIRResampling(const short *pData, unsigned __int64 offset, unsigned __int32 res, unsigned __int64 length)

			static float FIRResampling(
				int interp_factor, int decim_factor, int inp_size, const double *p_H,
				int H_size)
			{
				double *p_inp_real, *p_out_real, *p_Z_real,

				int ii, num_out = 0, current_phase;
				int num_phases = H_size / interp_factor;
				int out_size = inp_size * interp_factor / decim_factor + 1;

				// enforce input parameter assumptions
				assert(interp_factor > 0);
				assert(decim_factor > 0);
				assert(inp_size > 0);
				assert(p_H);
				assert(H_size > 0);
				assert(H_size % interp_factor == 0);

				// allocate storage for inputs, outputs, and delay line
				p_inp_real = calloc(inp_size, sizeof(double));
				p_out_real = calloc(out_size, sizeof(double));
				p_Z_real = calloc(num_phases, sizeof(double));

				if (test_method == SINE_TEST) {
					gen_complex_sine(inp_size, interp_factor, 1.0, inp_size, p_inp_real,
						p_inp_imag);
				} else {
					gen_complex_impulse(inp_size, p_inp_real, p_inp_imag);
				}

				// clear Z delay line
				for (ii = 0; ii < num_phases; ii++) {
					p_Z_real[ii] = 0.0;
				}

			// set current_phase to interp_factor so that resampler will
			//load new data into the delay line prior to calculating any
			//outputs
			current_phase = interp_factor;   
			resamp_complex(interp_factor, decim_factor, num_phases,
				&current_phase, p_H, p_Z_real, inp_size,
				p_inp_real, p_out_real,
				&num_out);

				// free allocated storage
				free(p_inp_real);
				free(p_inp_imag);
				free(p_out_real);
				free(p_out_imag);
				free(p_Z_real);
				free(p_Z_imag);
			}


*/








			
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
