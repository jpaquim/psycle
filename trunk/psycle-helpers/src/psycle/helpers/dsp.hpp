///\file
///\brief various signal processing utility functions and classes, psycle::helpers::dsp::Cubic amongst others.
#pragma once
#include "math/erase_all_nans_infinities_and_denormals.hpp"
#if defined DIVERSALIS__CPU__X86__SSE && defined DIVERSALIS__COMPILER__FEATURE__XMM_INTRINSICS 
	#include <xmmintrin.h>
#endif
#include <cmath>
#include <cstring>
#if defined BOOST_AUTO_TEST_CASE
	#include <universalis/os/aligned_alloc.hpp>
	#include <universalis/os/clocks.hpp>
	#include <sstream>
#endif
namespace psycle { namespace helpers { /** various signal processing utility functions. */ namespace dsp {

	/// linear -> deciBell
	/// amplitude normalized to 1.0f.
	float inline UNIVERSALIS__COMPILER__CONST
	dB(float amplitude)
	{
		return 20.0f * std::log10(amplitude);
	}

	/// deciBell -> linear
	float inline UNIVERSALIS__COMPILER__CONST
	dB2Amp(float db)
	{
		return std::pow(10.0f, db / 20.0f);
	}

	/// undenormalize (renormalize) samples in a signal buffer.
	///\todo make a template version that accept both float and doubles
	inline void Undenormalize(float * UNIVERSALIS__COMPILER__RESTRICT pSamplesL,float * UNIVERSALIS__COMPILER__RESTRICT pSamplesR, int numsamples)
	{
		math::erase_all_nans_infinities_and_denormals(pSamplesL, numsamples);
		math::erase_all_nans_infinities_and_denormals(pSamplesR, numsamples);
	}

	/****************************************************************************/

	/// mixes two signals. memory should be aligned by 16 in optimized paths.
	inline void Add(float * UNIVERSALIS__COMPILER__RESTRICT pSrcSamples, float * UNIVERSALIS__COMPILER__RESTRICT pDstSamples, int numSamples, float vol)
	{
		#if defined DIVERSALIS__CPU__X86__SSE && defined DIVERSALIS__COMPILER__FEATURE__XMM_INTRINSICS
			__m128 volps = _mm_set_ps1(vol);
			__m128 *psrc = (__m128*)pSrcSamples;
			__m128 *pdst = (__m128*)pDstSamples;
			do
			{
				__m128 tmpps = _mm_mul_ps(*psrc, volps);
				*pdst = _mm_add_ps(*pdst, tmpps);
				++psrc;
				++pdst;
				numSamples -= 4;
			} while(numSamples > 0);
		#elif defined DIVERSALIS__CPU__X86__SSE && defined DIVERSALIS__COMPILER__ASSEMBLER__INTEL
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
	inline void Mul(float * pDstSamples, int numSamples, float multi)
	{
		#if defined DIVERSALIS__CPU__X86__SSE && defined DIVERSALIS__COMPILER__FEATURE__XMM_INTRINSICS
			__m128 volps = _mm_set_ps1(multi);
			__m128 *pdst = (__m128*)pDstSamples;
			do
			{
				*pdst = _mm_mul_ps(*pdst,volps);
				pdst++;
				numSamples-=4;
			} while(numSamples>0);
		#elif defined DIVERSALIS__CPU__X86__SSE && defined DIVERSALIS__COMPILER__ASSEMBLER__INTEL
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
	inline void MovMul(float * UNIVERSALIS__COMPILER__RESTRICT pSrcSamples, float * UNIVERSALIS__COMPILER__RESTRICT pDstSamples, int numSamples, float multi)
	{
		#if defined DIVERSALIS__CPU__X86__SSE && defined DIVERSALIS__COMPILER__FEATURE__XMM_INTRINSICS
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
		#elif defined DIVERSALIS__CPU__X86__SSE && defined DIVERSALIS__COMPILER__ASSEMBLER__INTEL
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

	inline void Mov(float * UNIVERSALIS__COMPILER__RESTRICT pSrcSamples, float * UNIVERSALIS__COMPILER__RESTRICT pDstSamples, int numSamples)
	{
		#if defined DIVERSALIS__CPU__X86__SSE && defined DIVERSALIS__COMPILER__FEATURE__XMM_INTRINSICS
			do
			{
				__m128 tmpps = _mm_load_ps(pSrcSamples);
				_mm_storeu_ps(pDstSamples,tmpps);
				pSrcSamples+=4;
				pDstSamples+=4;
				numSamples-=4;
			} while(numSamples>0);
		#elif defined DIVERSALIS__CPU__X86__SSE && defined DIVERSALIS__COMPILER__ASSEMBLER__INTEL
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
			std::memcpy(pDstSamples, pSrcSamples, numSamples * sizeof *pSrcSamples);
		#endif
	}

	/// zero-out a signal buffer.
	inline void Clear(float *pDstSamples, int numSamples)
	{
		#if defined DIVERSALIS__CPU__X86__SSE && defined DIVERSALIS__COMPILER__FEATURE__XMM_INTRINSICS
			__m128 zeroval = _mm_set_ps1(0.0f);
			do
			{
				_mm_store_ps(pDstSamples,zeroval);
				pDstSamples+=4;
				numSamples-=4;
			}while(numSamples>0);
		#elif defined DIVERSALIS__CPU__X86__SSE && defined DIVERSALIS__COMPILER__ASSEMBLER__INTEL
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
			std::memset(pDstSamples, 0, numSamples * sizeof *pDstSamples);
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
	inline float GetRMSVol(RMSData &rms,float * UNIVERSALIS__COMPILER__RESTRICT pSamplesL, float * UNIVERSALIS__COMPILER__RESTRICT pSamplesR, int numSamples)
	{
		float * pL = pSamplesL;
		float * pR = pSamplesR;
		int ns = numSamples;
		int count(numRMSSamples - rms.count);
		// this can happen when changing the samplerate.
		if (count < 0) { count = 0;}
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
			rms.previousLeft  = std::sqrt(rms.AccumLeft  / numRMSSamples);
			rms.previousRight = std::sqrt(rms.AccumRight / numRMSSamples);
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
	inline float GetMaxVol(float * UNIVERSALIS__COMPILER__RESTRICT pSamplesL, float * UNIVERSALIS__COMPILER__RESTRICT pSamplesR, int numSamples)
	{
		///\todo: Implementation with Intrinsics.
		#if defined DIVERSALIS__CPU__X86__SSE && defined DIVERSALIS__COMPILER__FEATURE__XMM_INTRINSICS
			__m128 minVol = _mm_set_ps1(0.0f);
			__m128 maxVol = _mm_set_ps1(0.0f);
			__m128 *psrcl = (__m128*)pSamplesL;
			__m128 *psrcr = (__m128*)pSamplesR;
			while(numSamples > 0) {
				maxVol =  _mm_max_ps(maxVol,*psrcl);
				maxVol =  _mm_max_ps(maxVol,*psrcr);
				minVol =  _mm_min_ps(minVol,*psrcl);
				minVol =  _mm_min_ps(minVol,*psrcr);
				psrcl++;
				psrcr++;
				numSamples-=4;
			}
			__m128 highTmp = _mm_movehl_ps(maxVol, maxVol);
			maxVol =  _mm_max_ps(maxVol,highTmp);
			highTmp = _mm_move_ss(highTmp,maxVol);
			maxVol = _mm_shuffle_ps(maxVol, highTmp, 0x11);
			maxVol =  _mm_max_ps(maxVol,highTmp);
			
			__m128 lowTmp = _mm_movehl_ps(minVol, minVol);
			minVol =  _mm_max_ps(minVol,lowTmp);
			lowTmp = _mm_move_ss(lowTmp,minVol);
			minVol = _mm_shuffle_ps(minVol, lowTmp, 0x11);
			minVol =  _mm_max_ps(minVol,lowTmp);

			__m128 minus1 = _mm_set_ps1(-1.0f);
			minVol = _mm_mul_ss(minVol, minus1);
			maxVol = _mm_max_ps(maxVol,minVol);
			float result;
			_mm_store_ss(&result, maxVol);
			return result;
		#elif defined DIVERSALIS__CPU__X86__SSE && defined DIVERSALIS__COMPILER__ASSEMBLER__INTEL
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
				const float volL = std::fabs(*++pSamplesL);
				const float volR = std::fabs(*++pSamplesR);
				if (volL > vol) vol = volL;
				if (volR > vol) vol = volR;
			} while (--numSamples);
			return vol;
		#endif
	}

	/****************************************************************************/

	/// sample interpolator.
	class resampler {
		public:
			/// interpolator work function type
			typedef float (*work_func_type)(int16_t const * data, uint64_t offset, uint32_t res, uint64_t length);
			typedef float (*work_float_func_type)(float const * data, float offset, uint64_t length);
			
	/// sample interpolator kinds.
			struct quality { enum type {
				none,
				linear,
				spline,
				band_limited
			};};

			/// constructor
			resampler() : work(none), work_float(none), quality_(quality::none) {}

			/// work function corresponding to the selected kind.
			work_func_type work;
			work_float_func_type work_float;
			
			/// sets the kind of interpolation.
			virtual void quality(quality::type) = 0;
			virtual quality::type quality() const = 0;
			
		protected:
			/// kind of interpolation.
			quality::type quality_;
			
			/// interpolation work function which does nothing.
			static float none(int16_t const * data, uint64_t /*offset*/, uint32_t /*res*/, uint64_t /*length*/) {
				return *data;
			}
			static float none(float const * data, float /*offset*/, uint64_t /*length*/) {
				return *data;
			}
	};

	/// cubic sample interpolator.
	class cubic_resampler : public resampler {
		///
		#define CUBIC_RESOLUTION 2048
		public:
			/// constructor.
			cubic_resampler();

			/*override*/ quality::type quality() const { return quality_; }

			/*override*/ void quality(quality::type quality) {
				quality_ = quality;
				switch(quality) {
					case quality::none:
					work = none;
					break;
				case quality::linear:
					work = linear;
					break;
				case quality::spline:
					work = spline;
					work_float = spline_float;
					break;
				case quality::band_limited:
					work = band_limited;
					break;
				}
			}

		protected:
			/// interpolation work function which does linear interpolation.
			static float linear(int16_t const * data, uint64_t offset, uint32_t res, uint64_t length) {
				float y0 = *data;
				float y1 = offset + 1 == length ? 0 : *(data + 1);
				return y0 + (y1 - y0) * l_table_[res >> 21];
			}
			
			/// interpolation work function which does spline interpolation.
			static float spline(int16_t const * data, uint64_t offset, uint32_t res, uint64_t length) {
				res = res >> 21;
				float yo = offset == 0 ? 0 : *(data - 1);
				float y0 = *data;
				float y1 = offset + 1 == length ? 0 : *(data + 1);
				float y2 = offset + 2 == length ? 0 : *(data + 2);
				return a_table_[res] * yo + b_table_[res] * y0 + c_table_[res] * y1 + d_table_[res] * y2;
			}
			
			/// interpolation work function which does spline interpolation.
			static float spline_float(float const * data, float offset, uint64_t length) {
				int iOsc = std::floor(offset);
				uint32_t fractpart = (offset - iOsc) * CUBIC_RESOLUTION;

				float d0;
				float d1 = data[iOsc];
				float d2 = data[iOsc + 1];
				float d3 = data[iOsc + 2];
				if(iOsc == 0)
					d0 = data[length - 1];
				else
					d0 = data[iOsc - 1];
				return a_table_[fractpart] * d0 + b_table_[fractpart] * d1 + c_table_[fractpart] * d2 + d_table_[fractpart] * d3;
			}
			
			// yo = y[-1] [sample at x-1]
			// y0 = y[0]  [sample at x (input)]
			// y1 = y[1]  [sample at x+1]
			// y2 = y[2]  [sample at x+2]
			
			// res = distance between two neighboughing sample points [y0 and y1], so [0...1.0].
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
			static float band_limited(int16_t const * data, uint64_t offset, uint32_t res, uint64_t length) {
				res = res >> 23; // TODO assumes SINC_RESOLUTION == 512
				int leftExtent(SINC_ZEROS), rightExtent(SINC_ZEROS);
				if(offset < SINC_ZEROS) leftExtent = offset;
				if(length - offset < SINC_ZEROS) rightExtent = length - offset;
				
				const int sincInc(SINC_RESOLUTION);

				float newval = sinc_table_[res] * *data;
				float sincIndex(sincInc + res);
				float weight(sincIndex - std::floor(sincIndex));
				for(int i(1); i < leftExtent; ++i, sincIndex += sincInc)
					newval += (sinc_table_[int(sincIndex)] + sinc_delta_[int(sincIndex)] * weight) * *(data - i);

				sincIndex = sincInc - res;
				weight = sincIndex - std::floor(sincIndex);
				for(int i(1); i < rightExtent; ++i, sincIndex += sincInc)
					newval += (sinc_table_[int(sincIndex)] + sinc_delta_[int(sincIndex)] * weight) * *(data + i);

				return newval;
			}

			/****************************************************************************/
			#if 0
				static float FIRResampling(int16_t const * pData, uint64_t offset, uint32_t res, uint64_t length)

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
			static int resolution_;

			static float a_table_[CUBIC_RESOLUTION];
			static float b_table_[CUBIC_RESOLUTION];
			static float c_table_[CUBIC_RESOLUTION];
			static float d_table_[CUBIC_RESOLUTION];
			static float l_table_[CUBIC_RESOLUTION];

			/// sinc function table
			static float sinc_table_[SINC_TABLESIZE];

			/// table of deltas between sincTable indices.. sincDelta[i] = sincTable[i+1] - sincTable[i]
			/// used to speed up linear interpolation of sinc table-- this idea stolen from libresampler
			static float sinc_delta_[SINC_TABLESIZE];
	};

	#if defined BOOST_AUTO_TEST_CASE
		BOOST_AUTO_TEST_CASE(dsp_test) {
			std::size_t const alignment = 16;
			std::vector<float, universalis::os::aligned_alloc<float, alignment> > v1, v2;
			for(std::size_t s = 0; s < 10000; ++s) {
				v1.push_back(s);
				v2.push_back(s);
			}

			using namespace universalis::stdlib;
			typedef universalis::os::clocks::monotonic clock;
			int const iterations = 10000;
			float const vol = 0.5;

			{ // add
				nanoseconds const t1(clock::current());
				for(int i(0); i < iterations; ++i) Add(&v1[0], &v2[0], v1.size(), vol);
				nanoseconds const t2(clock::current());
				for(int i(0); i < iterations; ++i) {
					float const * in = &v1[0]; --in;
					float * out = &v2[0]; --out;
					std::size_t count = v1.size();
					do { *++out += *++in * vol; } while(--count);
				}
				nanoseconds const t3(clock::current());
				{
					std::ostringstream s;
					s << "add: " << (t2 - t1).get_count() * 1e-9 << "s < " << (t3 - t2).get_count() * 1e-9 << "s";
					BOOST_MESSAGE(s.str());
				}
				BOOST_CHECK(t2 - t1 < t3 - t2);
			}
		}
	#endif

}}}
