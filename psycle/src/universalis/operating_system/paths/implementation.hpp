// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2006 Johan Boule <bohan@jabber.org>
// Copyright (C) 2004-2006 Psycledelics http://psycle.pastnotecut.org

///\implementation universalis::operating_system::paths
#include <universalis/operating_system/exception.hpp>
#include <universalis/operating_system/exceptions/code_description.hpp>
#include <universalis/operating_system/detail/microsoft/max_path.hpp>
#include <cstdlib> // std::getenv for user's home dir
#include <sstream>
#include <iostream>
