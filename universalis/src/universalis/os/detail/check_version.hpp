// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

///\interface universalis::os::detail::check_version

#ifndef UNIVERSALIS__OS__DETAIL__CHECK_VERSION__INCLUDED
#define UNIVERSALIS__OS__DETAIL__CHECK_VERSION__INCLUDED
#pragma once

#include <universalis/exception.hpp>

#define UNIVERSALIS__COMPILER__DYNAMIC_LINK UNIVERSALIS__SOURCE
#include <universalis/compiler/dynamic_link/begin.hpp>

namespace universalis { namespace os { namespace detail {

/// excludes poor operating systems
void UNIVERSALIS__COMPILER__DYNAMIC_LINK check_version() throw(universalis::exception);

}}}

#include <universalis/compiler/dynamic_link/end.hpp>

#endif
