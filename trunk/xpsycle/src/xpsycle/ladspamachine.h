/***************************************************************************
 *   Copyright (C) 2006 by Stefan Nattkemper   *
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
#ifndef LADSPAMACHINE_H
#define LADSPAMACHINE_H

#include "machine.h"
#include <ladspa.h>
/**
@author Stefan Nattkemper
*/

namespace psycle {
	namespace host {

		class LADSPAMachine: public Machine {
		public:

				LADSPAMachine( Machine::id_type id, Song* song );

				virtual ~LADSPAMachine() throw();

				bool loadPlugin( const std::string & fileName );

		private:

			void* pluginHandle_;
			std::string ladspa_path;

			const LADSPA_Descriptor * psDescriptor;

		};

	}
}

#endif
