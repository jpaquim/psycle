///\file
///\brief various signal processing utility functions and classes, psycle::helpers::dsp::Cubic amongst others.
#pragma once
#include "helpers.hpp"
#include "math/erase_all_nans_infinities_and_denormals.hpp"
#include "math/truncate.hpp"
#include "math/round.hpp"
#include <universalis/compiler.hpp>
#if defined DIVERSALIS__PROCESSOR__X86__SSE && defined DIVERSALIS__COMPILER__FEATURE__XMM_INTRINSICS 
	#include <xmmintrin.h>
	#error
#endif
#include <cmath>
#include <cstdint>
namespace psycle { namespace helpers { /** various signal processing utility functions. */ namespace dsp {

	///\todo doc
	/// amplitude normalized to 1.0f.
	float inline UNIVERSALIS__COMPILER__CONST
	dB(float amplitude)
	{
		return 20.0f * std::log10(amplitude);
	}

	///\todo doc
	float inline UNIVERSALIS__COMPILER__CONST
	dB2Amp(float db)
	{
		return std::pow(10.0f, db / 20.0f);
	}

	/// undenormalize (renormalize) samples in a signal buffer.
	///\todo make a template version that accept both float and doubles
	inline void Undenormalize(float *pSamplesL,float *pSamplesR, int numsamples)
	{
		math::erase_all_nans_infinities_and_denormals(pSamplesL,numsamples);
		math::erase_all_nans_infinities_and_denormals(pSamplesR,numsamples);
	}

	/// Funky denormal check \todo make it a function
	#define IS_DENORMAL(f) (!((*(unsigned int *)&f)&0x7f800000))

	/****************************************************************************/

	/// mixes two signals. memory should be aligned by 16 in optimized paths.
	inline void Add(float *pSrcSamples, float *pDstSamples, int numSamples, float vol)
	{
		#if defined DIVERSALIS__PROCESSOR__X86__SSE && defined DIVERSALIS__COMPILER__FEATURE__XMM_INTRINSICS
			__m128 volps = _mm_set_ps1(vol);
			__m128 *psrc = (__m128*)pSrcSamples;
			__m128 *pdst = (__m128*)pDstSamples;
			do
			{
				__m128 tmpps = _mm_mul_ps(*psrc,volps);
				*pdst = _mm_add_ps(*pdst,tmpps);
				psrc++;
				pdst++;
				numSamples-=4;
			} while(numSamples>0);
		#elif defined DIVERSALIS__PROCESSOR__X86__SSE && defined DIVERSALIS__COMPILER__ASSEMBLER__INTEL
			__asm
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
			--pSrcSamples; --pDstSamples;
			do { *++pDstSamples += *++pSrcSamples * vol; } while(--numSamples);
		#endif
	}

	/// multiply a signal by a ratio, inplace.
	///\see MovMul()
	inline void Mul(float *pDstSamples, int numSamples, float multi)
	{
		#if defined DIVERSALIS__PROCESSOR__X86__SSE && defined DIVERSALIS__COMPILER__FEATURE__XMM_INTRINSICS
			__m128 volps = _mm_set_ps1(multi);
			__m128 *pdst = (__m128*)pDstSamples;
			do
			{
				*pdst = _mm_mul_ps(*pdst,volps);
				pdst++;
				numSamples-=4;
			} while(numSamples>0);
		#elif defined DIVERSALIS__PROCESSOR__X86__SSE && defined DIVERSALIS__COMPILER__ASSEMBLER__INTEL
			// This code assumes aligned memory (to 16) and assigned by powers of 4!
			__asm
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
			do { *++pDstSamples *= multi; } while (--numSamples);
		#endif
	}

	/// multiply a signal by a ratio.
	///\see Mul()
	inline void MovMul(float *pSrcSamples, float *pDstSamples, int numSamples, float multi)
	{
		#if defined DIVERSALIS__PROCESSOR__X86__SSE && defined DIVERSALIS__COMPILER__FEATURE__XMM_INTRINSICS
			__m128 volps = _mm_set_ps1(multi);
			__m128 *psrc = (__m128*)pSrcSamples;
			do
			{
				__m128 tmpps = _mm_mul_ps(*psrc,volps);
				_mm_storeu_ps(pDstSamples,tmpps);
				psrc++;
				pDstSamples+=4;
				numSamples-=4;
			} while(numSamples>0);
		#elif defined DIVERSALIS__PROCESSOR__X86__SSE && defined DIVERSALIS__COMPILER__ASSEMBLER__INTEL
			// This code assumes aligned memory (to 16) and assigned by powers of 4!
			__asm
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
			--pSrcSamples; --pDstSamples;
			do { *++pDstSamples = *++pSrcSamples*multi; } while (--numSamples);
		#endif
	}

	inline void Mov(float *pSrcSamples, float *pDstSamples, int numSamples)
	{
		#if defined DIVERSALIS__PROCESSOR__X86__SSE && defined DIVERSALIS__COMPILER__FEATURE__XMM_INTRINSICS
			do
			{
				__m128 tmpps = _mm_load_ps(pSrcSamples);
				_mm_storeu_ps(pDstSamples,tmpps);
				pSrcSamples+=4;
				pDstSamples+=4;
				numSamples-=4;
			} while(numSamples>0);
		#elif defined DIVERSALIS__PROCESSOR__X86__SSE && defined DIVERSALIS__COMPILER__ASSEMBLER__INTEL
			// This code assumes aligned memory (to 16) and assigned by powers of 4!
			__asm
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
	inline void Clear(float *pDstSamples, int numSamples)
	{
		#if defined DIVERSALIS__PROCESSOR__X86__SSE && defined DIVERSALIS__COMPILER__FEATURE__XMM_INTRINSICS
			__m128 zeroval = _mm_set_ps1(0.0f);
			do
			{
				_mm_store_ps(pDstSamples,zeroval);
				pDstSamples+=4;
				numSamples-=4;
			}while(numSamples>0);
		#elif defined DIVERSALIS__PROCESSOR__X86__SSE && defined DIVERSALIS__COMPILER__ASSEMBLER__INTEL
			// This code assumes aligned memory (to 16) and assigned by powers of 4!
			__asm
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

	extern int numRMSSamples;
	struct RMSData {
		int count;
		double AccumLeft, AccumRight;
		float previousLeft, previousRight;
	};

	/// finds the RMS volume value in a signal buffer.
	/// Note: Values are buffered since the standard calculation requires 50ms or data.
	inline float GetRMSVol(RMSData &rms,float *pSamplesL, float *pSamplesR, int numSamples)
	{
		float * pL = pSamplesL;
		float * pR = pSamplesR;
		int ns = numSamples;
		int count(numRMSSamples - rms.count);
		--pL; --pR;
		if ( ns >= count)
		{
			ns -= count;
			{
				double acleft(rms.AccumLeft),acright(rms.AccumRight);
				while (count--) {
					++pL; acleft  += *pL * *pL;
					++pR; acright += *pR * *pR;
				};
				rms.AccumLeft = acleft;
				rms.AccumRight = acright;
			}
			rms.previousLeft  = std::sqrt(rms.AccumLeft  / dsp::numRMSSamples);
			rms.previousRight = std::sqrt(rms.AccumRight / dsp::numRMSSamples);
			rms.AccumLeft = 0;
			rms.AccumRight = 0;
			rms.count = 0;
		}
		{
			double acleft(rms.AccumLeft), acright(rms.AccumRight);
			while(ns--) {
				++pL; acleft  += *pL * *pL;
				++pR; acright += *pR * *pR;
				++rms.count;
			};
			rms.AccumLeft  = acleft;
			rms.AccumRight = acright;
		}
		return rms.previousLeft > rms.previousRight ? rms.previousLeft : rms.previousRight;
	}

	/// finds the maximum amplitude in a signal buffer.
	inline float GetMaxVol(float *pSamplesL, float *pSamplesR, int numSamples)
	{
		#if defined DIVERSALIS__PROCESSOR__X86__SSE && defined DIVERSALIS__COMPILER__ASSEMBLER__INTEL
			// If anyone knows better assembler than me improve this variable utilization:
			float volmax = 0.0f, volmin = 0.0f;
			float *volmaxb = &volmax, *volminb = &volmin;
			__asm
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
		#else
			--pSamplesL; --pSamplesR;
			float vol = 0.0f;
			do { /// not all waves are symmetrical
				const float volL = fabsf(*++pSamplesL);
				const float volR = fabsf(*++pSamplesR);
				if (volL > vol) vol = volL;
				if (volR > vol) vol = volR;
			} while (--numSamples);
			return vol;
		#endif
	}

	/****************************************************************************/

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
	typedef float (*PRESAMPLERFN)(std::int16_t const * pData, std::uint64_t offset, std::uint32_t res, std::uint64_t length);

	/// sample interpolator.
	class Resampler
	{
		public:
			/// constructor
			Resampler() : _quality(R_NONE), _pWorkFn(None) {}
			/// work function corresponding to the selected kind.
			PRESAMPLERFN _pWorkFn;
			/// sets the kind of interpolation.
			virtual void SetQuality(ResamplerQuality quality) = 0;
			virtual ResamplerQuality GetQuality() = 0;
		protected:
			/// kind of interpolation.
			ResamplerQuality _quality;
			/// interpolation work function which does nothing.
			static float None(std::int16_t const * pData, std::uint64_t offset, std::uint32_t res, std::uint64_t length)
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

			virtual ResamplerQuality GetQuality() { return _quality; }

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

		protected:
			/// interpolation work function which does linear interpolation.
			static float Linear(std::int16_t const * pData, std::uint64_t offset, std::uint32_t res, std::uint64_t length)
			{
				float y0,y1;
				y0 = *pData;
				y1 =(offset+1 == length)?0:*(pData+1);
				return (y0+(y1-y0)*_lTable[res>>21]);
			}
			/// interpolation work function which does spline interpolation.
			static float Spline(const short *pData, std::uint64_t offset, std::uint32_t res, std::uint64_t length)
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
			
			// res= distance between two neighboughing sample points [y0 and y1], so [0...1.0].
			// You have to multiply this distance * RESOLUTION used
			// on the spline conversion table. [2048 by default]
			// If you are using 2048 is asumed you are using 12 bit decimal
			// fixed point offsets for resampling.
			
			// offset = sample offset [info to avoid go out of bounds on sample reading ]
			// length = sample length [info to avoid go out of bounds on sample reading ]

			// either or both of these can be fine-tuned to find a tolerable compromise between quality and memory/cpu usage
			// make sure any changes to SINC_RESOLUTION are reflected in Bandlimit()!
			
			/// sinc table values per zero crossing -- keep it a power of 2!!
			// note: even with this optimization, interpolating the sinc table roughly doubles the cpu usage on my machine.
			// since we're working in realtime here, it may be best to just make SINC_RESOLUTION a whole lot bigger, and drop
			// the table interpolation altogether..
			#define SINC_RESOLUTION 512
			/// sinc table zero crossings (per side) -- too low and it aliases, too high uses lots of cpu.
			#define SINC_ZEROS 11
			#define SINC_TABLESIZE SINC_RESOLUTION * SINC_ZEROS

			/// interpolation work function which does band-limited interpolation.
			static float Bandlimit(std::int16_t const * pData, std::uint64_t offset, std::uint32_t res, std::uint64_t length)
			{
				res = res>>23; //!!!assumes SINC_RESOLUTION == 512!!!
				int leftExtent(SINC_ZEROS), rightExtent(SINC_ZEROS);
				if(offset<SINC_ZEROS) leftExtent=offset;
				if(length-offset<SINC_ZEROS) rightExtent=length-offset;
				
				const int sincInc(SINC_RESOLUTION);
				float newval(0.0);

				newval += sincTable[res] * *(pData);
				float sincIndex(sincInc+res);
				float weight(sincIndex - floor(sincIndex));
				for(int i(1); i < leftExtent; ++i, sincIndex+=sincInc)
					newval+= (sincTable[(int)sincIndex] + sincDelta[(int)sincIndex]*weight ) * *(pData-i);

				sincIndex = sincInc-res;
				weight = sincIndex - floor(sincIndex);
				for(int i(1); i < rightExtent; ++i, sincIndex+=sincInc)
					newval += ( sincTable[(int)sincIndex] + sincDelta[(int)sincIndex]*weight ) * *(pData+i);

				return newval;
			}

			/****************************************************************************/
			#if 0
				static float FIRResampling(std::int16_t const * pData, std::uint64_t offset, std::uint32_t res, std::uint64_t length)

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
					std::free(p_inp_real);
					std::free(p_inp_imag);
					std::free(p_out_real);
					std::free(p_out_imag);
					std::free(p_Z_real);
					std::free(p_Z_imag);
				}
			#endif

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

			/// sinc function table
			static float sincTable[SINC_TABLESIZE];

			/// table of deltas between sincTable indices.. sincDelta[i] = sincTable[i+1] - sincTable[i]
			/// used to speed up linear interpolation of sinc table-- this idea stolen from libresampler
			static float sincDelta[SINC_TABLESIZE];
	};
}}}
