///\file
/// numeric types
#pragma once
#include INCLUDE(PROJECT/project.hpp)
#include INCLUDE(PROJECT/compilers.hpp)

///////////////////////////////////////
/// generic floating point number type
///////////////////////////////////////

#if defined COMPILER__FEATURE__TYPEOF
	typedef typeof(0.) real;
#else
	// no typeof with this compiler yet, assuming default floating point number type for litterals is double.
	typedef double real;
#endif

//////////////////////////
// explicitly sized types
//////////////////////////

#if defined COMPILER__FEATURE__NOT_CONCRETE
	namespace compiler
	{
		/// >=80-bit floating point number
		typedef long double float80;
		/// 64-bit floating point number
		typedef double float64;
		/// 32-bit floating point number
		typedef float float32;
		/// signed 64-bit integral number
		typedef signed long int sint64; 
		/// unsigned 64-bit integral number
		typedef unsigned long int uint64;
		/// signed 32-bit integral number
		typedef signed int sint32;
		/// unsigned 32-bit integral number
		typedef unsigned int uint32;
		/// signed 16-bit integral number
		typedef signed short int sint16;
		/// unsigned 16-bit integral number
		typedef unsigned short int uint16;
		/// 8-bit integral number
		typedef signed char sint8;
		/// unsigned 8-bit integral number
		typedef unsigned char uint8;
	}
#elif defined CONFIGURATION__MU
	namespace compiler
	{
		#if CONFIGURATION__MU__SIZEOF_LONG_DOUBLE >= 10
			typedef long double float80;
		#elif CONFIGURATION__MU__SIZEOF_DOUBLE >= 10
			typedef double float80;
		#elif CONFIGURATION__MU__SIZEOF_FLOAT >= 10
			typedef float float80;
		#else
			#error "No >=80-bit floating point number."
		#endif
		#if CONFIGURATION__MU__SIZEOF_LONG_DOUBLE == 8
			typedef long double float64;
		#elif CONFIGURATION__MU__SIZEOF_DOUBLE == 8
			typedef double float64;
		#elif CONFIGURATION__MU__SIZEOF_FLOAT == 8
			typedef float float64;
		#else
			#error "No 64-bit floating point number."
		#endif
		#if CONFIGURATION__MU__SIZEOF_LONG_DOUBLE == 4
			typedef long double float32;
		#elif CONFIGURATION__MU__SIZEOF_DOUBLE == 4
			typedef double float32;
		#elif CONFIGURATION__MU__SIZEOF_FLOAT == 4
			typedef float float32;
		#else
			#error "No 32-bit floating point number."
		#endif
	}
	#if defined CONFIGURATION__MU__HAVE_INTTYPES_H
		#include <boost/cstdint.hpp>
		namespace compiler
		{
			typedef boost::uint64_t uint64;
			typedef boost::int64_t sint64;
			typedef boost::uint32_t uint32;
			typedef boost::int32_t sint32;
			typedef boost::uint16_t uint16;
			typedef boost::int16_t sint16;
			typedef boost::uint8_t uint8;
			typedef boost::int8_t sint8;
		}
	#else
		namespace compiler
		{
			#if CONFIGURATION__MU__SIZEOF_LONG_LONG_INT == 8
				typedef signed long long int sint64;
				typedef unsigned long long int uint64;
			#elif CONFIGURATION__MU__SIZEOF_LONG_INT == 8
				typedef signed long int sint64;
				typedef unsigned long int uint64;
			#elif CONFIGURATION__MU__SIZEOF_INT == 8
				typedef signed int sint64;
				typedef unsigned int uint64;
			#elif CONFIGURATION__MU__SIZEOF_SHORT_INT == 8
				typedef signed short int sint64;
				typedef unsigned short int uint64;
			#elif CONFIGURATION__MU__SIZEOF_CHAR == 8
				typedef signed char sint64;
				typedef unsigned char uint64;
			#else
				#error "No 64-bit integral number."
			#endif
			#if CONFIGURATION__MU__SIZEOF_LONG_LONG_INT == 4
				typedef signed long long int sint32;
				typedef unsigned long long int uint32;
			#elif CONFIGURATION__MU__SIZEOF_LONG_INT == 4
				typedef signed long int sint32;
				typedef unsigned long int uint32;
			#elif CONFIGURATION__MU__SIZEOF_INT == 4
				typedef signed int sint32;
				typedef unsigned int uint32;
			#elif CONFIGURATION__MU__SIZEOF_SHORT_INT == 4
				typedef signed short int sint32;
				typedef unsigned short int uint32;
			#elif CONFIGURATION__MU__SIZEOF_CHAR == 4
				typedef signed char sint32;
				typedef unsigned char uint32;
			#else
				#error "No 32-bit integral number."
			#endif
			#if CONFIGURATION__MU__SIZEOF_LONG_LONG_INT == 2
				typedef signed long long int sint16;
				typedef unsigned long long int uint16;
			#elif CONFIGURATION__MU__SIZEOF_LONG_INT == 2
				typedef signed long int sint16;
				typedef unsigned long int uint16;
			#elif CONFIGURATION__MU__SIZEOF_INT == 2
				typedef signed int sint16;
				typedef unsigned int uint16;
			#elif CONFIGURATION__MU__SIZEOF_SHORT_INT == 2
				typedef signed short int sint16;
				typedef unsigned short int uint16;
			#elif CONFIGURATION__MU__SIZEOF_CHAR == 2
				typedef signed char sint16;
				typedef unsigned char uint16;
			#else
				#error "No 16-bit integral number."
			#endif
			#if CONFIGURATION__MU__SIZEOF_LONG_LONG_INT == 1
				typedef signed long long int sint8;
				typedef unsigned long long int uint8;
			#elif CONFIGURATION__MU__SIZEOF_LONG_INT == 1
				typedef signed long int sint8;
				typedef unsigned long int uint8;
			#elif CONFIGURATION__MU__SIZEOF_INT == 1
				typedef signed int sint8;
				typedef unsigned int uint8;
			#elif CONFIGURATION__MU__SIZEOF_SHORT_INT == 1
				typedef signed short int sint8;
				typedef unsigned short int uint8;
			#elif CONFIGURATION__MU__SIZEOF_CHAR == 1
				typedef signed char sint8;
				typedef unsigned char uint8;
			#else
				#error "No 8-bit integral number."
			#endif
		}
	#endif
#elif defined COMPILER__MICROSOFT
	namespace compiler
	{
		typedef long double float80;
		typedef double float64;
		typedef float float32;
		typedef signed __int64 sint64; 
		typedef unsigned __int64 uint64;
		typedef signed __int32 sint32;
		typedef unsigned __int32 uint32;
		typedef signed __int16 sint16;
		typedef unsigned __int16 uint16;
		typedef signed __int8 sint8;
		typedef unsigned __int8 uint8;
	}
#else
	#error "No explicitly sized types."
#endif
