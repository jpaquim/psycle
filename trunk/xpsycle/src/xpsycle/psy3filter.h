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
#ifndef PSY3FILTER_H
#define PSY3FILTER_H

#include "psyfilter.h"

/**
@author Stefan Nattkemper
*/

namespace psycle
{
	namespace host
	{

		class RiffFile;

		class Psy3Filter : public PsyFilter
		{
			public:
				Psy3Filter();

				~Psy3Filter();

			protected:

				virtual bool testFormat(const std::string & fileName);
				virtual void load(const std::string & fileName, Song & song);

				virtual int LoadSONGv0(RiffFile* file,Song& song);
				virtual bool LoadINFOv0(RiffFile* file,Song& song,int minorversion);
				virtual bool LoadSNGIv0(RiffFile* file,Song& song,int minorversion);
				virtual bool LoadSEQDv0(RiffFile* file,Song& song,int minorversion);
				virtual bool LoadPATDv0(RiffFile* file,Song& song,int minorversion);
				virtual bool LoadMACDv0(RiffFile* file,Song& song,int minorversion);
				virtual bool LoadINSDv0(RiffFile* file,Song& song,int minorversion);


		protected:

				static std::string const FILE_FOURCC;
				static std::uint32_t const VERSION_INFO;
				static std::uint32_t const VERSION_SNGI;
				static std::uint32_t const VERSION_SEQD;
				static std::uint32_t const VERSION_PATD;
				static std::uint32_t const VERSION_MACD;
				static std::uint32_t const VERSION_INSD;
				static std::uint32_t const VERSION_WAVE;

				static std::uint32_t const FILE_VERSION;


		private:

				std::vector<int> seqList;
				PatternCategory* singleCat;
				SequenceLine* singleLine;


				void preparePatternSequence(Song & song);

		};


	} // end of host namespace
} // end of psycle namespace
#endif
