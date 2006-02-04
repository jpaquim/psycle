// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2005 Psycledelics http://psycle.pastnotecut.org : Johan Boule

///\file
///\brief concatenation
#pragma once
#include <universalis/detail/project.hpp>
//#namespace UNIVERSALIS
	//#namespace COMPILER
		/// Concatenates two tokens.
		/// The indirection in the call to ## lets the macro expansion on the arguments be done first.
		#define UNIVERSALIS__COMPILER__CONCATENATED(left_token, right_token) UNIVERSALIS__COMPILER__CONCATENATED__DETAIL__EXPANSION(left_token, right_token)

		//#namespace DETAIL		
			///\internal
			/// Don't call this macro directly ; call UNIVERSALIS__COMPILER__CONCATENATED, which calls this macro after macro expansion is done on the argument.
			///\relates UNIVERSALIS__COMPILER__CONCATENATED
			#define UNIVERSALIS__COMPILER__CONCATENATED__DETAIL__EXPANSION(left_token, right_token) UNIVERSALIS__COMPILER__CONCATENATED__DETAIL__NO_EXPANSION(left_token, right_token)
			
			///\internal
			/// Don't call this macro directly ; call UNIVERSALIS__COMPILER__CONCATENATED, which calls this macro after macro expansion is done on the argument.
			///\relates UNIVERSALIS__COMPILER__CONCATENATED
			#define UNIVERSALIS__COMPILER__CONCATENATED__DETAIL__NO_EXPANSION(left_token, right_token) left_token##right_token
		//#endnamespace
	//#endnamespace
//#endnamespace
