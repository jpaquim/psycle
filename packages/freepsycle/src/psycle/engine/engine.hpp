// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2006 Johan Boule <bohan@jabber.org>
// Copyright (C) 2004-2006 Psycledelics http://psycle.pastnotecut.org

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
#include "node.hpp"
#include "port.hpp"
#include "ports/input.hpp"
#include "ports/inputs/single.hpp"
#include "ports/inputs/multiple.hpp"
#include "ports/output.hpp"
#include "buffer.hpp"
#include "event.hpp"
#include "sample.hpp"
///\internal
/// extensible modular audio frawework.
namespace psycle
{
	///\internal
	/// functionalities shared by both the plugins and the host.
	namespace engine
	{
	}
}
