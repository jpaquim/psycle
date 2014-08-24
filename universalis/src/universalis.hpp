// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2013 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\file \brief meta header that includes universalis' essential features

#pragma once

#include "universalis/compiler/stringize.hpp"
#include "universalis/compiler/concat.hpp"
#include "universalis/compiler/token.hpp"
#include "universalis/os/eol.hpp"

#ifndef DIVERSALIS__COMPILER__RESOURCE

	#include "universalis/compiler/constexpr.hpp"
	#include "universalis/compiler/noexcept.hpp"
	#include "universalis/compiler/restrict.hpp"
	#include "universalis/compiler/pragma.hpp"
	#include "universalis/compiler/attribute.hpp"
	#include "universalis/compiler/const_function.hpp"
	#include "universalis/compiler/pure_function.hpp"
	#include "universalis/compiler/align.hpp"
	#include "universalis/compiler/pack.hpp"
	#include "universalis/compiler/message.hpp"
	#include "universalis/compiler/deprecated.hpp"
	#include "universalis/compiler/virtual.hpp"
	#include "universalis/compiler/thread_local.hpp"
	#include "universalis/compiler/weak.hpp"
	#include "universalis/compiler/calling_convention.hpp"
	#include "universalis/compiler/asm.hpp"
	#include "universalis/compiler/dyn_link.hpp"
	#include "universalis/compiler/auto_link.hpp"
	#include "universalis/compiler/typenameof.hpp"
	#include "universalis/compiler/location.hpp"
	#include "universalis/compiler/exception.hpp"

	#include "universalis/stdlib/cstdint.hpp"
	#include "universalis/stdlib/exception.hpp"

	#include "universalis/os/loggers.hpp"
	#include "universalis/os/exception.hpp"

	#include "universalis/cpu/exception.hpp"

	// TODO remove these
	#include <boost/preprocessor/stringize.hpp>
	#include <boost/static_assert.hpp>
	#include <cassert>
	#include <ciso646>

#endif
