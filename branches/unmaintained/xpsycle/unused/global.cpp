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
#include "global.h"
#include "song.h"
#include "player.h"
#include "configuration.h"
#include "defaultbitmaps.h"

namespace psycle {
	namespace host {


Configuration* Global::pConfig() {
	static Configuration* c = new Configuration();
	return c;
}

Configuration & Global::configuration() {
	return *pConfig();
}

Global::Global() { }

Global::~Global() { }

void Global::log(std::string message) {
				std::cout << message << std::endl;
}

}
}