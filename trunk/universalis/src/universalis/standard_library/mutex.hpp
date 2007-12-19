// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2007 psycledelics http://psycle.pastnotecut.org ; johan boule <bohan@jabber.org>

///\file \brief mutex standard header
#pragma once
#include <universalis/operating_system/threads/mutex.hpp>
namespace std {
	typedef universalis::operating_system::threads::mutex mutex;
	typedef universalis::operating_system::threads::recursive_mutex recursive_mutex;
}
