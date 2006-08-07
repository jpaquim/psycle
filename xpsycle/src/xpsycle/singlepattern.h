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
#ifndef SINGLEPATTERN_H
#define SINGLEPATTERN_H

#include "patternline.h"
#include "timesignature.h"
#include <ngrs/sigslot.h>
#include <map>
#include <vector>
#include <string>

/**
@author Stefan Nattkemper
*/

namespace psycle
{
	namespace host
	{

		class PatternCategory;

		class SinglePattern : public std::map<double, PatternLine> {
		public:
			SinglePattern();

			~SinglePattern();

			void setID(int id);
			int id() const;

			void setBeatZoom(int zoom);
			int beatZoom() const;

			void addBar( const TimeSignature & signature );

			float beats() const;

			bool barStart(double pos, TimeSignature & signature) const;
			void clearBars();

			const TimeSignature & playPosTimeSignature(double pos) const;

			void setName(const std::string & name);
			const std::string & name() const;

			void setCategory(PatternCategory* category);
			PatternCategory* category();

			void clearEmptyLines();

			void scaleBlock(int left, int right, double top, double bottom, float factor);
			void transposeBlock(int left, int right, double top, double bottom, int trp);
			void deleteBlock(int left, int right, double top, double bottom);

			void clearPosition(double beatpos, int track, int column);

			std::vector<TimeSignature> &  timeSignatures();
			const std::vector<TimeSignature> &  timeSignatures() const;

			std::string toXml() const;

		private:

			int beatZoom_;
			std::string name_;

			PatternCategory* category_;
			std::vector<TimeSignature> timeSignatures_;

			TimeSignature zeroTime;

			int id_;
			static int idCounter;

		};

	}
}

#endif
