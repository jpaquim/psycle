// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2010 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\file
///\brief meta header: all the psycle::engine interface in one header file inclusion.
#ifndef PSYCLE__ENGINE__ENGINE__INCLUDED
#define PSYCLE__ENGINE__ENGINE__INCLUDED
#pragma once
#include <psycle/detail/project.hpp>
#include "forward_declarations.hpp"
#include "exception.hpp"
#include "reference_counter.hpp"
#include "named.hpp"
#include "graph.hpp"
#include "buffer.hpp"
#include "event.hpp"
#include "sample.hpp"
///\internal
/// extensible modular audio frawework.
namespace psycle {
	///\internal
	/// functionalities shared by both the plugins and the host.
	namespace engine {}
}
#endif
