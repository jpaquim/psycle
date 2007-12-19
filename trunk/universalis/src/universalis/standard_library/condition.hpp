// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2007 psycledelics http://psycle.pastnotecut.org ; johan boule <bohan@jabber.org>

///\file \brief condition standard header
#pragma once
#include <universalis/operating_system/threads/condition.hpp>
namespace std {
	template <class Lock> class condition;
	typedef condition<mutex> condition_mtx;
	typedef condition< unique_lock<mutex> > condition_ulm;
}
