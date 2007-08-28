///\todo This is not up-to-date and should be replaced with the common psycle/helpers/helpers.hpp

/***************************************************************************
	*   Copyright (C) 2007 Psycledelics   *
	*   psycle.sf.net   *
	*                                                                         *
	*   This program is free software; you can redistribute it and/or modify  *
	*   it under the terms of the GNU General Public License as published by  *
	*   the Free Software Foundation; either version 2 of the License, or     *
	*   (at your option) any later version.                                   *
	*                                                                         *
	*   This program is distributed in the hope that it will be useful,       *
	*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
	*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
	*   GNU General Public License for more details.                          *
	*                                                                         *
	*   You should have received a copy of the GNU General Public License     *
	*   along with this program; if not, write to the                         *
	*   Free Software Foundation, Inc.,                                       *
	*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
	***************************************************************************/
#pragma once
#ifndef PSYCLE_CORE_HELPERS_INCLUDED
#define PSYCLE_CORE_HELPERS_INCLUDED
#include <psycle/core/cstdint.h>

//namespace psycle { namespace core {

		/// the pi constant as a 32-bit floating point number
		float const F_PI =
			#if defined M_PI
				static_cast<float>(M_PI)
			#else
				3.14159265358979323846f
			#endif
			;

		/// parses an hexadecimal string to convert it to an integral number
		template<typename X>
		void hexstring_to_integer(const std::string &, X &);

		/// linearly maps a byte (0 to 255) to a float (0 to 1).
		///\todo check if the table lookup is actually faster than calculating.
		///\todo needs some explanation about why the tables have a length of 257.
		class CValueMapper
		{
		public:
			/// contructor.
			CValueMapper();
			/// destructor.
			virtual ~CValueMapper() throw();
			/// maps a byte (0 to 255) to a float (0 to 1).
			static inline float Map_255_1(int iByte)
			{
				if(iByte>=0&&iByte<=256)
					return CValueMapper::fMap_255_1[iByte];
				else
					return iByte * 0.00390625f;
			}
			/// maps a byte (0 to 255) to a float (O to 100).
			static inline float Map_255_100(int iByte)
			{
				if(iByte>=0&&iByte<=256)
					return CValueMapper::fMap_255_100[iByte];
				else
					return iByte*0.390625f;
			}
		private:
			static float fMap_255_1[257];
			static float fMap_255_100[257];
		};

		///\todo doc
		inline float fast_log2(float f) 
		{ 
			//BOOST_STATIC_ASSERT((sizeof f == sizeof(int)));
			//BOOST_STATIC_ASSERT((sizeof f == 4));
			//assert( f > 0. ); 
			union tmp_union {
				float f;
				std::uint32_t i;
			} tmp;
			tmp.f = f;
			return ((tmp.i & 0x7f800000) >> 23) - 0x7f + (tmp.i & 0x007fffff) / (float)0x800000;
		} 

		#if 0 // v1.9
			inline std::int32_t f2i(float f) 
			{ 
				#if defined DIVERSALIS__PROCESSOR__X86 && defined DIVERSALIS__COMPILER__MICROSOFT // also intel's compiler?
					///\todo not always the fastest when using sse(2)
					///\todo we can also use C1999's lrint if available
					///\todo do we really need to write this in custom asm? wouldn't it be better to rely on the compiler?
					#if 1
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
						const double magic = 6755399441055744.0; // 2^51 + 2^52
						union tmp_union
						{
							double d;
							int i;
						} tmp;
						tmp.d = (d - 0.5) + magic;
						return tmp.i;
					#endif
				#else
					///\todo specify the rounding mode
					return static_cast<std::int32_t>(f);
				#endif
			}
		#else
			/// converts a floating point number to an integer.
			inline int f2i(float f) 
			{
				return static_cast<int>(f); ///\todo this is wrong since it's not the same rounding mode as f2i ; use C1999's lrint
			}
		#endif

		#if 0 // uses universalis
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
		#endif

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
//}}
#endif
