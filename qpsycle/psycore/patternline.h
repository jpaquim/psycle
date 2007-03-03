/***************************************************************************
	*   Copyright (C) 2006 by  Psycledelics     *
	*   psycle.sf.net   *
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
#ifndef PATTERNLINE_H
#define PATTERNLINE_H

#include "patternevent.h"
#include <map>

/**
@author  Psycledelics  
*/

namespace psycle
{
	namespace host
	{

		class PatternLine {
		public:
			PatternLine();
			~PatternLine();

			void setSequenceTrack(int track);
			int sequenceTrack() const;

			std::string toXml( float pos ) const;

			std::map<int, PatternEvent> & notes();
			const std::map<int, PatternEvent> & notes() const;

			std::map<int, PatternEvent> & tweaks();
			const std::map<int, PatternEvent> & tweaks() const;

			virtual bool empty() const;

		private:

			int sequencerTrack_;

						std::map<int, PatternEvent>  noteMap;
			std::map<int, PatternEvent> tweakMap;

		};

	}
}

#endif
