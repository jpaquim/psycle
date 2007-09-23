// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

///\file
///\brief meta header: all the psycle::engine interface in one header file inclusion.
#pragma once
#include <psycle/detail/project.hpp>
#include <universalis/operating_system/loggers.hpp>
#include "hello.hpp"
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

