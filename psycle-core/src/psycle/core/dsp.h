// -*- mode:c++; indent-tabs-mode:t -*-
///\file \brief various signal processing utility functions and classes, psy::core::Cubic amongst others. based on revison  2663
#pragma once
#include "misc.h"
#include <psycle/helpers/math/erase_all_nans_infinities_and_denormals.hpp>
#include <psycle/helpers/math/truncate.hpp>
#include <psycle/helpers/math/round.hpp>
#include <universalis/compiler.hpp>
#include <cstdint>
#if defined DIVERSALIS__PROCESSOR__X86__SSE && defined DIVERSALIS__COMPILER__FEATURE__XMM_INTRINSICS 
	#include <xmmintrin.h>
#endif
#include <cmath>
#include <cstring>

namespace psy { namespace core { namespace dsp {

///\todo doc
/// amplitude normalized to 1.0f.
float inline UNIVERSALIS__COMPILER__CONST
dB(float amplitude) {
	return 20.0f * std::log10(amplitude);
}

///\todo doc
float inline UNIVERSALIS__COMPILER__CONST
dB2Amp(float db) {
	return std::pow(10.0f, db / 20.0f);
}

/// undenormalize (renormalize) samples in a signal buffer.
///\todo make a template version that accept both float and doubles
inline void Undenormalize(float *pSamplesL,float *pSamplesR, int numsamples) {
	psycle::helpers::math::erase_all_nans_infinities_and_denormals(pSamplesL, numsamples);
	psycle::helpers::math::erase_all_nans_infinities_and_denormals(pSamplesR, numsamples);
}

/// Funky denormal check \todo make it a function
#define IS_DENORMAL(f) (!((*(unsigned int *)&f)&0x7f800000))
	
/****************************************************************************/

/// mixes two signals. memory should be aligned by 16 in optimized paths.
static inline void Add(float *pSrcSamples, float *pDstSamples, int numSamples, float vol) {
	#if defined DIVERSALIS__PROCESSOR__X86__SSE && defined DIVERSALIS__COMPILER__FEATURE__XMM_INTRINSICS
		__m128 volps = _mm_set_ps1(vol);
		__m128 *psrc = (__m128*)pSrcSamples;
		__m128 *pdst = (__m128*)pDstSamples;
		do {
			__m128 tmpps = _mm_mul_ps(*psrc,volps);
			*pdst = _mm_add_ps(*pdst,tmpps);
			++psrc;
			++pdst;
			numSamples -= 4;
		} while(numSamples > 0);
	#elif defined DIVERSALIS__PROCESSOR__X86__SSE && defined DIVERSALIS__COMPILER__ASSEMBLER__INTEL
		__asm {
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
static inline void Mul(float *pDstSamples, int numSamples, float multi) {
	#if defined DIVERSALIS__PROCESSOR__X86__SSE && defined DIVERSALIS__COMPILER__FEATURE__XMM_INTRINSICS
		__m128 volps = _mm_set_ps1(multi);
		__m128 *pdst = (__m128*)pDstSamples;
		do {
			*pdst = _mm_mul_ps(*pdst,volps);
			++pdst;
			numSamples -= 4;
		} while(numSamples > 0);
	#elif defined DIVERSALIS__PROCESSOR__X86__SSE && defined DIVERSALIS__COMPILER__ASSEMBLER__INTEL
		// This code assumes aligned memory (to 16) and assigned by powers of 4!
		__asm {
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
static inline void MovMul(float *pSrcSamples, float *pDstSamples, int numSamples, float multi) {
	#if defined DIVERSALIS__PROCESSOR__X86__SSE && defined DIVERSALIS__COMPILER__FEATURE__XMM_INTRINSICS
		__m128 volps = _mm_set_ps1(multi);
		__m128 *psrc = (__m128*)pSrcSamples;
		do {
			__m128 tmpps = _mm_mul_ps(*psrc,volps);
			_mm_storeu_ps(pDstSamples,tmpps);
			++psrc;
			pDstSamples += 4;
			numSamples -= 4;
		} while(numSamples > 0);
	#elif defined DIVERSALIS__PROCESSOR__X86__SSE && defined DIVERSALIS__COMPILER__ASSEMBLER__INTEL
		// This code assumes aligned memory (to 16) and assigned by powers of 4!
		__asm {
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

static inline void Mov(float *pSrcSamples, float *pDstSamples, int numSamples) {
	#if defined DIVERSALIS__PROCESSOR__X86__SSE && defined DIVERSALIS__COMPILER__FEATURE__XMM_INTRINSICS
		do {
			__m128 tmpps = _mm_load_ps(pSrcSamples);
			_mm_storeu_ps(pDstSamples,tmpps);
			pSrcSamples += 4;
			pDstSamples += 4;
			numSamples -= 4;
		} while(numSamples > 0);
	#elif defined DIVERSALIS__PROCESSOR__X86__SSE && defined DIVERSALIS__COMPILER__ASSEMBLER__INTEL
		// This code assumes aligned memory (to 16) and assigned by powers of 4!
		__asm {
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
static inline void Clear(float *pDstSamples, int numSamples) {
	#if defined DIVERSALIS__PROCESSOR__X86__SSE && defined DIVERSALIS__COMPILER__FEATURE__XMM_INTRINSICS
		__m128 zeroval = _mm_set_ps1(0.0f);
		do {
			_mm_store_ps(pDstSamples,zeroval);
			pDstSamples += 4;
			numSamples -= 4;
		} while(numSamples > 0);
	#elif defined DIVERSALIS__PROCESSOR__X86__SSE && defined DIVERSALIS__COMPILER__ASSEMBLER__INTEL
		// This code assumes aligned memory (to 16) and assigned by powers of 4!
		__asm {
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
static inline int F2I(double d) {
	const double magic(6755399441055744.0); /// 2^51 + 2^52
	union tmp_union {
		double d;
		int i;
	} tmp;
	tmp.d = (d-0.5) + magic;
	return tmp.i;
}

extern int numRMSSamples;
struct RMSData {
	int count;
	double AccumLeft, AccumRight;
	float previousLeft, previousRight;
};

/// finds the RMS volume value in a signal buffer.
/// Note: Values are buffered since the standard calculation requires 50ms or data.
inline float GetRMSVol(RMSData &rms,float *pSamplesL, float *pSamplesR, int numSamples) {
	float *pL = pSamplesL;
	float *pR = pSamplesR;
	int ns = numSamples;
	int count(numRMSSamples - rms.count);
	--pL; --pR;
	if(ns >= count) {
		ns -= count;
		{
			double acleft(rms.AccumLeft), acright(rms.AccumRight);
			while(count--) {
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
static inline float GetMaxVol(float *pSamplesL, float *pSamplesR, int numSamples) {
	#if defined DIVERSALIS__PROCESSOR__X86__SSE && defined DIVERSALIS__COMPILER__ASSEMBLER__INTEL
		// If anyone knows better assembler than me improve this variable utilization:
		float volmax = 0.0f, volmin = 0.0f;
		float *volmaxb = &volmax, *volminb = &volmin;
		__asm {
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
		volmin *= -1.0f;
		return volmax > volmin ? volmax : volmin;
	#else
		--pSamplesL;
		--pSamplesR;

		float vol = 0.0f;
		do {
			/// not all waves are symmetrical
			const float volL = fabsf(*++pSamplesL);
			const float volR = fabsf(*++pSamplesR);
			if(volL > vol) vol = volL;
			if(volR > vol) vol = volR;
		} while(--numSamples);
		return vol;
	#endif
}

/****************************************************************************/

/// sample interpolator kinds.
///\todo typdef should be inside the Resampler or Cubic class itself.
enum ResamplerQuality {
	R_NONE  = 0,
	R_LINEAR,
	R_SPLINE,
	R_BANDLIM
};

/// interpolator work function.
///\todo typdef should be inside the Resampler class itself.
typedef float (*PRESAMPLERFN)(const short *pData, std::uint64_t offset, std::uint32_t res, std::uint64_t length);

/// sample interpolator.
class Resampler {
	public:
		/// constructor
		Resampler()
		{ 
			_quality = R_NONE;
			_pWorkFn = T_None;
		}
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
class Cubic : public Resampler {
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

}}}
