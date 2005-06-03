// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2005 Psycledelics http://psycle.pastnotecut.org : Johan Boule

///\file
/// numeric types
#pragma once
#include <universalis/detail/project.hpp>
#if defined DIVERSALIS__HAVE_INTTYPES_H
	#include <boost/cstdint.hpp>
#endif
namespace universalis
{
	namespace compiler
	{
		///////////////////////////////////////
		/// generic floating point number type
		///////////////////////////////////////

		template<unsigned int const = 0> struct numeric
		{
			typedef
				#if defined DIVERSALIS__COMPILER__FEATURE__TYPEOF
					typeof(0.)
				#else
					// no typeof with this compiler yet, assuming default floating point number type for litterals is double.
					double
				#endif
				floating_point;
		};

		//////////////////////////
		// explicitly sized types
		//////////////////////////

		namespace numerics
		{
			namespace detail
			{
				#if defined DIVERSALIS__COMPILER__FEATURE__NOT_CONCRETE
					/// >=80-bit floating point number
					typedef long double float80;
					///   64-bit floating point number
					typedef      double float64;
					///   32-bit floating point number
					typedef       float float32;
					///   signed 64-bit integral number

					typedef   signed long long int sint64; 
					/// unsigned 64-bit integral number
					typedef unsigned long long int uint64;
					///   signed 32-bit integral number
					typedef   signed           int sint32;
					/// unsigned 32-bit integral number
					typedef unsigned           int uint32;
					///   signed 16-bit integral number
					typedef   signed short     int sint16;
					/// unsigned 16-bit integral number
					typedef unsigned short     int uint16;
					///   signed 8-bit integral number
					typedef   signed          char sint8;
					/// unsigned 8-bit integral number
					typedef unsigned          char uint8;
				#elif defined DIVERSALIS__SIZEOF
					#if DIVERSALIS__SIZEOF_LONG_DOUBLE >= 10
						typedef long double float80;
					#elif DIVERSALIS__SIZEOF_DOUBLE >= 10
						typedef      double float80;
					#elif DIVERSALIS__SIZEOF_FLOAT >= 10
						typedef       float float80;
					#else
						#error "No >=80-bit floating point number."
					#endif
					#if DIVERSALIS__SIZEOF_LONG_DOUBLE == 8
						typedef long double float64;
					#elif DIVERSALIS__SIZEOF_DOUBLE == 8
						typedef      double float64;
					#elif DIVERSALIS__SIZEOF_FLOAT == 8
						typedef       float float64;
					#else
						#error "No 64-bit floating point number."
					#endif
					#if DIVERSALIS__SIZEOF_LONG_DOUBLE == 4
						typedef long double float32;
					#elif DIVERSALIS__SIZEOF_DOUBLE == 4
						typedef      double float32;
					#elif DIVERSALIS__SIZEOF_FLOAT == 4
						typedef       float float32;
					#else
						#error "No 32-bit floating point number."
					#endif

					#if defined DIVERSALIS__HAVE_INTTYPES_H
						typedef boost::uint64_t uint64;
						typedef boost:: int64_t sint64;
						typedef boost::uint32_t uint32;
						typedef boost:: int32_t sint32;
						typedef boost::uint16_t uint16;
						typedef boost:: int16_t sint16;
						typedef boost::uint8_t  uint8;
						typedef boost:: int8_t  sint8;
					#else
						#if DIVERSALIS__SIZEOF_LONG_LONG_INT == 8
							typedef   signed long long int sint64;
							typedef unsigned long long int uint64;
						#elif DIVERSALIS__SIZEOF_LONG_INT == 8
							typedef   signed long      int sint64;
							typedef unsigned long      int uint64;
						#elif DIVERSALIS__SIZEOF_INT == 8
							typedef   signed           int sint64;
							typedef unsigned           int uint64;
						#elif DIVERSALIS__SIZEOF_SHORT_INT == 8
							typedef   signed short     int sint64;
							typedef unsigned short     int uint64;
						#elif DIVERSALIS__SIZEOF_CHAR == 8
							typedef   signed          char sint64;
							typedef unsigned          char uint64;
						#else
							#error "No 64-bit integral number."
						#endif
						#if DIVERSALIS__SIZEOF_LONG_LONG_INT == 4
							typedef   signed long long int sint32;
							typedef unsigned long long int uint32;
						#elif DIVERSALIS__SIZEOF_LONG_INT == 4
							typedef   signed long      int sint32;
							typedef unsigned long      int uint32;
						#elif DIVERSALIS__SIZEOF_INT == 4
							typedef   signed           int sint32;
							typedef unsigned           int uint32;
						#elif DIVERSALIS__SIZEOF_SHORT_INT == 4
							typedef   signed short     int sint32;
							typedef unsigned short     int uint32;
						#elif DIVERSALIS__SIZEOF_CHAR == 4
							typedef   signed          char sint32;
							typedef unsigned          char uint32;
						#else
							#error "No 32-bit integral number."
						#endif
						#if DIVERSALIS__SIZEOF_LONG_LONG_INT == 2
							typedef   signed long long int sint16;
							typedef unsigned long long int uint16;
						#elif DIVERSALIS__SIZEOF_LONG_INT == 2
							typedef   signed long      int sint16;
							typedef unsigned long      int uint16;
						#elif DIVERSALIS__SIZEOF_INT == 2
							typedef   signed           int sint16;
							typedef unsigned           int uint16;
						#elif DIVERSALIS__SIZEOF_SHORT_INT == 2
							typedef   signed short     int sint16;
							typedef unsigned short     int uint16;
						#elif DIVERSALIS__SIZEOF_CHAR == 2
							typedef   signed          char sint16;
							typedef unsigned          char uint16;
						#else
							#error "No 16-bit integral number."
						#endif
						#if DIVERSALIS__SIZEOF_LONG_LONG_INT == 1
							typedef   signed long long int sint8;
							typedef unsigned long long int uint8;
						#elif DIVERSALIS__SIZEOF_LONG_INT == 1
							typedef   signed long      int sint8;
							typedef unsigned long      int uint8;
						#elif DIVERSALIS__SIZEOF_INT == 1
							typedef   signed           int sint8;
							typedef unsigned           int uint8;
						#elif DIVERSALIS__SIZEOF_SHORT_INT == 1
							typedef   signed short     int sint8;
							typedef unsigned short     int uint8;
						#elif DIVERSALIS__SIZEOF_CHAR == 1
							typedef   signed          char sint8;
							typedef unsigned          char uint8;
						#else
							#error "No 8-bit integral number."
						#endif
					#endif
				#elif defined DIVERSALIS__COMPILER__MICROSOFT
					typedef long double float80;
					typedef      double float64;
					typedef       float float32;

					typedef   signed __int64 sint64; 
					typedef unsigned __int64 uint64;
					typedef   signed __int32 sint32;
					typedef unsigned __int32 uint32;
					typedef   signed __int16 sint16;
					typedef unsigned __int16 uint16;
					typedef   signed __int8  sint8;
					typedef unsigned __int8  uint8;
				#else
					#error "No explicitly sized types."
				#endif
			}
		}

		template<> struct numeric<010<<0>
		{
			typedef numerics::detail::sint8         signed_int       ;
			typedef numerics::detail::uint8       unsigned_int       ;
			#if defined DIVERSALIS__HAVE_INTTYPES_H
			typedef boost:: int_fast8_t            signed_int_fastest;
			typedef boost::uint_fast8_t          unsigned_int_fastest;
			#else
			typedef   signed_int                   signed_int_fastest;
			typedef unsigned_int                 unsigned_int_fastest;
			#endif
			
		};
		template<> struct numeric<010<<1>
		{
			typedef numerics::detail::sint16       signed_int       ;
			typedef numerics::detail::uint16     unsigned_int       ;
			#if defined DIVERSALIS__HAVE_INTTYPES_H
			typedef boost:: int_fast16_t          signed_int_fastest;
			typedef boost::uint_fast16_t        unsigned_int_fastest;
			#else
			typedef   signed_int                  signed_int_fastest;
			typedef unsigned_int                unsigned_int_fastest;
			#endif
		};
		template<> struct numeric<010<<2>
		{
			typedef numerics::detail::sint32      signed_int        ;
			typedef numerics::detail::uint32    unsigned_int        ;
			#if defined DIVERSALIS__HAVE_INTTYPES_H
			typedef boost:: int_fast32_t          signed_int_fastest;
			typedef boost::uint_fast32_t        unsigned_int_fastest;
			#else
			typedef   signed_int                  signed_int_fastest;
			typedef unsigned_int                unsigned_int_fastest;
			#endif
			typedef numerics::detail::float32 floating_point        ;
			typedef floating_point            floating_point_fastest;
		};
		template<> struct numeric<010<<3>
		{
			typedef numerics::detail::sint64      signed_int        ;
			typedef numerics::detail::uint64    unsigned_int        ;
			#if defined DIVERSALIS__HAVE_INTTYPES_H
			typedef boost:: int_fast64_t          signed_int_fastest;
			typedef boost::uint_fast64_t        unsigned_int_fastest;
			#else
			typedef   signed_int                  signed_int_fastest;
			typedef unsigned_int                unsigned_int_fastest;
			#endif
			typedef numerics::detail::float64 floating_point        ;
			typedef floating_point            floating_point_fastest;
		};
		template<> struct numeric<010<<3|010<<1>
		{
			typedef numerics::detail::float80 floating_point        ;
			typedef floating_point            floating_point_fastest;
		};
	}
}

// arch-tag: 9f542f1a-932e-488e-9672-0dedd4e30ad9
