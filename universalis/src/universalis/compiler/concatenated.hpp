/* -*- mode:c++, indent-tabs-mode:t -*- */
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.sourceforge.net

///\file
///\brief concatenation
#pragma once
#include <universalis/detail/project.hpp>
//#region UNIVERSALIS
	//#region COMPILER
		/// Concatenates two tokens, after expanding the arguments.
		/// The indirection in the call to ## lets the macro expansion on the arguments be done first.
		#define UNIVERSALIS__COMPILER__CONCATENATED(left_token, right_token) UNIVERSALIS__COMPILER__CONCATENATED__DETAIL__NO_EXPANSION(left_token, right_token)

		//#region DETAIL
			///\internal
			/// Don't call this macro directly ; call UNIVERSALIS__COMPILER__CONCATENATED, which calls this macro after macro expansion is done on the argument.
			///\relates UNIVERSALIS__COMPILER__CONCATENATED
			#define UNIVERSALIS__COMPILER__CONCATENATED__DETAIL__NO_EXPANSION(left_token, right_token) UNIVERSALIS__COMPILER__CONCATENATED__NO_EXPANSION(left_token, right_token)
		//#endregion

		/// Concatenates two tokens, without expanding the arguments.
		///\relates UNIVERSALIS__COMPILER__CONCATENATED
		#define UNIVERSALIS__COMPILER__CONCATENATED__NO_EXPANSION(left_token, right_token) left_token##right_token
	//#endregion
//#endregion
