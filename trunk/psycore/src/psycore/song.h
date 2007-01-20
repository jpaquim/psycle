/***************************************************************************
  *   Copyright (C) 2007 by  Stefan Nattkemper  *
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
#ifndef SONG_H
#define SONG_H

#include "patterndata.h"
#include "patternsequence.h"
#include <string>

namespace psy
{
	namespace core
	{

		/// songs hold everything comprising a "tracker module",
		/// this include patterns, pattern sequence, machines 
		/// and their initial parameters and coordinates, wavetables


      class SongInfo {
      public: 

        SongInfo();
        ~SongInfo();

        void setName( const std::string& name );
        const std::string& name() const;

      private:

        std::string name_;

      };

		class Song
		{
			public:
				Song();

				virtual ~Song();

                const SongInfo& info() const;

				PatternSequence* patternSequence();
                const PatternSequence & patternSequence() const;

			private:

				PatternSequence patternSequence_;
                SongInfo info_;

			
		};
	}
}

#endif