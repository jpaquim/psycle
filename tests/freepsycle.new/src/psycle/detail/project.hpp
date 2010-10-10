// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2010 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

#ifndef PSYCLE__DETAIL__PROJECT__INCLUDED
#define PSYCLE__DETAIL__PROJECT__INCLUDED
#pragma once

#include <universalis.hpp>

#ifndef PSYCLE__DETAIL__SHARED
#define PSYCLE__DETAIL__SHARED(x) PSYCLE__DETAIL__SHARED_(x)
#define PSYCLE__DETAIL__SHARED_(x) x##__SHARED
#endif

#ifndef PSYCLE__DETAIL__SOURCE
#define PSYCLE__DETAIL__SOURCE(x) PSYCLE__DETAIL__SOURCE_(x)
#define PSYCLE__DETAIL__SOURCE_(x) x##__SOURCE
#endif

#endif
