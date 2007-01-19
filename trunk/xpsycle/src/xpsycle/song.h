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

namespace psycle
{
	namespace host
	{

		class PluginFinder;
		class PluginFinderKey;

		/// songs hold everything comprising a "tracker module",
		/// this include patterns, pattern sequence, machines 
		/// and their initial parameters and coordinates, wavetables

		class Song
		{
			public:
				Song();

				virtual ~Song();

				void clear(); // clears all song data

				PatternSequence* patternSequence();
				const PatternSequence& patternSequence() const;

				// loads a song
				bool load(const std::string& fileName);
				bool save(const std::string& fileName);

				//authorship
				void setName(const std::string & name);
				const std::string & name() const;
				void setAuthor(const std::string & author);
				const std::string & author() const;
				void setComment(const std::string & comment);
				const std::string & comment() const;

				// The number of tracks in each pattern of this song.
				unsigned int tracks() const;
				void setTracks( unsigned int trackCount) ;

				// start bpm for song, can be overwritten through global bpm event
				void setBpm(float bpm);
				float bpm() const;

                std::string fileName;

			private:

				PatternSequence patternSequence_;

				unsigned int tracks_;
				float bpm_;

				//authorship
				std::string name_;
				std::string author_;
				std::string comment_;

			
		};
	}
}

#endif