///\file
///\brief interface file for psycle::host::CValueMapper.
#pragma once
//#include <psycle/host/detail/project.hpp>
#include <cmath>
#include <cstdint>
#include <universalis/compiler/numeric.hpp>
#include <boost/static_assert.hpp>
namespace psycle
{
	namespace host
	{
		float const F_PI =
			#if defined M_PI
				static_cast<float>(M_PI)
			#else
				3.14159265358979323846f
			#endif
			;

		template<typename X>
		void hexstring_to_integer(std::string const &, X &);

		/// linearly maps a byte (0 to 255) to a float (0 to 1).
		///\todo check if the table lookup is actually faster than calculating.
		///\todo needs some explanation about why the tables have a length of 257.
		class CValueMapper
		{
			public:
				CValueMapper();
				~CValueMapper();
				/// maps a byte (0 to 255) to a float (0 to 1).
				static inline float Map_255_1(int iByte)
				{	
					if(iByte >=0 && iByte <= 256)
						return CValueMapper::fMap_255_1[iByte];
					else	
						return iByte * 0.00390625f;
				}
				/// maps a byte (0 to 255) to a float (0 to 100).
				static inline float Map_255_100(int iByte)
				{
					if(iByte >= 0 && iByte <= 256)
						return CValueMapper::fMap_255_100[iByte];
					else	
						return iByte * 0.390625f;
				}
			private:
				static float fMap_255_1[257];
				static float fMap_255_100[257];
		};

		inline float fast_log2(float const f)
		{ 
			BOOST_STATIC_ASSERT((sizeof f == 4));
			//assert(f > 0); 
			std::uint32_t const i(*reinterpret_cast<std::uint32_t const*>(&f));
			return ((i & 0x7f800000) >> 23) - 0x7f + (i & 0x007fffff) / (float)0x800000; 
		}

		/// converts a floating point number to an integer.
		inline std::int32_t f2i(float f) 
		{ 
			#if defined DIVERSALIS__PROCESSOR__X86 && defined DIVERSALIS__COMPILER__MICROSOFT // or intel?
				///\todo do we really need to write this in custom asm? wouldn't it be better to rely on the compiler?
				///\todo this is probably slowing down things on compilers using sse(2)
				std::int32_t i;
				double const half(0.5);
				_asm
				{ 
					fld f;
					fsub half;
					fistp i;
				} 
				return i;
			#else
				///\todo specify the rounding mode
				return static_cast<std::int32_t>(f);
			#endif
		}
		
		/// clipping.
		template<unsigned int const bits>
		typename universalis::compiler::numeric<bits>::signed_int inline f2iclip(float const & f)
		{
			typedef typename universalis::compiler::numeric<bits>::signed_int result_type;
			typedef std::numeric_limits<result_type> type_traits;
			if(f < type_traits::min) return type_traits::min;
			if(f > type_traits::max) return type_traits::max;
			return static_cast<result_type>(f);
		}

		/// clipping.
		inline int f2iclip16(float flt) 
		{ 
			if (flt <-32767.0f)
			{
				return -32767;
			}
			if (flt > 32767.0f)
			{
				return 32767;
			}
			return f2i(flt);
		}

		/// clipping.
		inline int f2iclip18(float flt) 
		{ 
			if (flt <-131071.0f)
			{
				return -131071;
			}
			if (flt > 131071.0f)
			{
				return 131071;
			}
			return f2i(flt);
		}

		/// clipping.
		inline int f2iclip20(float flt) 
		{ 
			if (flt <-524287.0f)
			{
				return -524287;
			}
			if (flt > 524287.0f)
			{
				return 524287;
			}
			return f2i(flt);
		}

		/// clipping.
		inline int f2iclip24(float flt) 
		{ 
			if (flt <-8388607.0f)
			{
				return -8388607;
			}
			if (flt > 8388607.0f)
			{
				return 8388607;
			}
			return f2i(flt);
		}

		/// clipping.
		inline int f2iclip32(float flt) 
		{ 
			if (flt <-2147483647.0f)
			{
				return -2147483647;
			}
			if (flt > 2147483647.0f)
			{
				return 2147483647;
			}
			return f2i(flt);
		}
	}
}
