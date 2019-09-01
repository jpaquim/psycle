// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2002-2007 johan boule <bohan@jabber.org>
// copyright 2001-2007 psycledelics http://psycle.pastnotecut.org

///\file
///\brief meta header
/// extensions to the language which would become standard
#pragma once
#include "stringized.hpp"
#include "concatenated.hpp"
#include "token.hpp"
#if !defined DIVERSALIS__COMPILER__RESOURCE
	#include "detail/standard/restrict.hpp"
	#include "detail/standard/wchar_t.hpp"
	#include "detail/pragmatic/pragmas.hpp"
	#include "detail/pragmatic/attribute.hpp"
	#include "detail/pragmatic/const_function.hpp"
	#include "detail/pragmatic/align.hpp"
	#include "detail/pragmatic/pack.hpp"
	#include "detail/pragmatic/deprecated.hpp"
	#include "detail/pragmatic/virtual.hpp"
	#include "detail/pragmatic/thread_local_storage.hpp"
	#include "detail/pragmatic/weak.hpp"
	#include "detail/pragmatic/calling_convention.hpp"
	#include "detail/pragmatic/asm.hpp"
	#include "detail/pragmatic/finally.hpp"
	#include "detail/pragmatic/dynamic_link.hpp"
	#include "detail/pragmatic/hardware_exception.hpp"
	#include "detail/gnu/typeof.hpp"
	#include "detail/gnu/diagnostics.hpp"
	#include "detail/microsoft/assume.hpp"
	#include "detail/microsoft/super.hpp"
	#include "detail/microsoft/warnings.hpp"
	#include "detail/microsoft/optimizations.hpp"
	#include "detail/microsoft/namespace.hpp"
	#include <boost/static_assert.hpp>
#endif
