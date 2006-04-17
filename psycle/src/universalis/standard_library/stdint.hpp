// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2006 Johan Boule <bohan@jabber.org>
// Copyright (C) 2004-2006 Psycledelics http://psycle.pastnotecut.org

///\file
#pragma once
#include <universalis/detail/project.hpp>
#if 0 && !defined DIVERSALIS__COMPILER__MICROSOFT
	#include <cstdint>
	// some unix systems had the equivalent inttypes.h for a long time
	//#include <inttypes.h>
#else /// microsoft has been ignoring the C1999 standard for six years now
	#include <boost/cstdint.hpp>
	namespace std
	{
		using boost::int8_t;             
		using boost::int_least8_t;       
		using boost::int_fast8_t;        
		using boost::uint8_t;            
		using boost::uint_least8_t;      
		using boost::uint_fast8_t;       
		                 
		using boost::int16_t;            
		using boost::int_least16_t;      
		using boost::int_fast16_t;       
		using boost::uint16_t;           
		using boost::uint_least16_t;     
		using boost::uint_fast16_t;      
		                 
		using boost::int32_t;            
		using boost::int_least32_t;      
		using boost::int_fast32_t;       
		using boost::uint32_t;           
		using boost::uint_least32_t;     
		using boost::uint_fast32_t;      
		                 
		using boost::int64_t;            
		using boost::int_least64_t;      
		using boost::int_fast64_t;       
		using boost::uint64_t;           
		using boost::uint_least64_t;     
		using boost::uint_fast64_t;      

		using boost::intmax_t;      
		using boost::uintmax_t;     
	}
#endif
