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
#ifndef PSY4FILTER_H
#define PSY4FILTER_H

#include "psyfilter.h"
#include <ngrs/sigslot.h>
#include <ngrs/nxmlparser.h>
#include <iostream>
#include <fstream>
#include <sstream>

/**
@author Stefan Nattkemper
*/

namespace psycle {
	namespace host {

		class Psy4Filter : public PsyFilter, public sigslot::has_slots<>
		{
			public:
				Psy4Filter();

				~Psy4Filter();

				virtual int version() const;

				virtual bool testFormat(const std::string & fileName);
				virtual bool load(const std::string & fileName, Song & song);
				virtual bool save( const std::string & fileName, const Song & song );

			private:

				NXmlParser parser;

				std::fstream _stream;
				void onDetectFilterTag(const std::string & tagName);
				void onTagParse(const std::string & tagName);
				bool isPsy4;

				PatternCategory* lastCategory;
				SinglePattern* lastPattern;
				SequenceLine* lastSeqLine;
				Machine* lastMachine;
				float lastPatternPos;

				Song* song_;

		};

	}
}

#endif
