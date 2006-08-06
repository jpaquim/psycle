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

#include "psy3filter.h"

/**
@author Stefan Nattkemper
*/

namespace psycle {
	namespace host {

		class Psy4Filter : public Psy3Filter
		{
			public:
				Psy4Filter();

				~Psy4Filter();

				virtual int version() const;

			protected:

				virtual bool testFormat(const std::string & fileName);
				virtual void save(const std::string & fileName, const Song & song) const;

				virtual bool LoadSNGIv0(RiffFile* file,Song& song,int minorversion);

				virtual bool SaveSONGv0(RiffFile* file,const Song& song);
				virtual bool SaveINFOv0(RiffFile* file,const Song& song);
				virtual bool SaveSNGIv0(RiffFile* file,const Song& song);
				virtual bool SaveSEQDv0(RiffFile* file,const Song& song);
				virtual bool SavePATDv0(RiffFile* file,const Song& song);
				virtual bool SaveMACDv0(RiffFile* file,const Song& song,int index);
				virtual bool SaveINSDv0(RiffFile* file,const Song& song,int index);
				virtual bool SaveWAVEv0(RiffFile* file,const Song& song,int index);

				static std::uint32_t const VERSION_SNGI;

		};

	}
}

#endif
