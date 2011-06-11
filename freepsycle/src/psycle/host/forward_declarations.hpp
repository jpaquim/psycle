// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2010 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\file
/// forward declarations for the namespace psycle::host.
/// This file is used to avoid mutually recursive dependencies in declarations.
/// It does not have any real declarations, just forward names.
#ifndef PSYCLE__HOST__FORWARD_DECLARATIONS__INCLUDED
#define PSYCLE__HOST__FORWARD_DECLARATIONS__INCLUDED
#pragma once
#include <psycle/engine/forward_declarations.hpp>
namespace psycle {

/// functionalities used by the host side only, not by the plugins.
namespace host {
	class graph;
	class node;
	class port;
	namespace ports {
		class output;
		class input;
		namespace inputs {
			class single;
			class multiple;
		}
	}
}

}
#endif
