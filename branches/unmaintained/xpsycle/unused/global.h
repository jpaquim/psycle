/***************************************************************************
	*   Copyright (C) 2006 by  Stefan   *
	*   natti@linux   *
	*                                                                         *
	*   This program is free software; you can redistribute it and/or modify  *
	*   it under the terms of the GNU General Public License as published by  *
	*   the Free Software Foundation; either version 2 of the License, or     *
	*   (at your option) any later version.                                   *
	*                                                                         *
	*   This program is distributed in the hope that it will be useful,       *
	*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
	*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
	*   GNU General Public License for more details.                          *
	*                                                                         *
	*   You should have received a copy of the GNU General Public License     *
	*   along with this program; if not, write to the                         *
	*   Free Software Foundation, Inc.,                                       *
	*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
	***************************************************************************/
#ifndef PSYCLEGLOBAL_H
#define PSYCLEGLOBAL_H

#include <iostream>

#pragma once

		/**
		@author  Stefan
		*/

namespace psycle {
	namespace host {

		class Song;
		class Configuration;

		class Global{
		public:
				Global();

				~Global();

				static Configuration* pConfig();
				static Configuration & configuration();
				static void log(std::string message);
		};

	template<typename single_object> inline single_object * zapObject(single_object *& pointer, single_object * const new_value = 0)
	{
		if(pointer) delete pointer;
		return pointer = new_value;
	}

	template<typename object_array> inline object_array * zapArray(object_array *& pointer, object_array * const new_value = 0)
	{
		if(pointer) delete [] pointer;
		return pointer = new_value;
	}

	}
}

#endif