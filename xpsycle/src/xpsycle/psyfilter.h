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
#ifndef PSYFILTER_H
#define PSYFILTER_H

#include "song.h"
#include <vector>

/**
@author Stefan Nattkemper
*/

namespace psycle
{
	namespace host
	{


		class PsyFilter{
		public:
			PsyFilter();

			virtual ~PsyFilter();

			bool loadSong(const std::string & fileName, Song & song);
			bool saveSong(const std::string & fileName, Song & song, int version);

			//signals
			sigslot::signal2<const std::string &, const std::string &> report;
			sigslot::signal3<const std::int32_t& , const std::int32_t& , const std::string& > progress;


		protected:

			virtual int version() const;
			virtual bool testFormat(const std::string & fileName);
			virtual bool load(const std::string & fileName, Song & song);
			virtual bool save(const std::string & fileName, const Song & song);

		private:

			static int c;
			static std::vector<PsyFilter*> filters;

			bool fileIsReadable( const std::string & file );

		};

	} // end of host namespace
}// end of psycle namespace

#endif
