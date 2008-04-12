// -*- mode:c++; indent-tabs-mode:t -*-
/***************************************************************************
*   Copyright (C) 2007 Psycledelics     *
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
//#include <psycle/core/psycleCorePch.hpp>
#include <iostream>
#include <fstream>
#include "file.h"
#include "machineFactory.h"
#include "psyfilter.h"
#include "psy2filter.h"
#include "psy3filter.h"
#include "psy4filter.h"
#include "song.h"

namespace psy
{
	namespace core
	{
		PsyFilters::PsyFilters(MachineFactory& factory1)
		:factory(factory1)
		{
			filters.push_back( Psy2Filter::Instance() );
			filters.push_back( Psy3Filter::Instance() );
			filters.push_back( Psy4Filter::Instance() );
		}

		bool PsyFilters::loadSong(const std::string & fileName, CoreSong & song)
		{
			if ( File::fileIsReadable( fileName ) ) {
				for (std::vector<PsyFilterBase*>::iterator it = filters.begin(); it < filters.end(); ++it) {
					PsyFilterBase* filter = *it;
					if ( filter->testFormat(fileName) ) {
						return filter->load(fileName,song,factory);
						break;
					}
				}
			}
			return false;
		}

		bool PsyFilters::saveSong( const std::string & fileName, const CoreSong & song, int version )
		{
			for (std::vector<PsyFilterBase*>::iterator it = filters.begin(); it < filters.end(); it++) {
				PsyFilterBase* filter = *it;
				if ( filter->version() == version ) {
					// check postfix
					std::string newFileName = fileName;
					int dotPos = fileName.rfind(".");
					if ( dotPos == std::string::npos ) 
						// append postfix
						newFileName = fileName + "." + filter->filePostfix();
					return filter->save(newFileName,song);
					break;
				}
			}
			std::cerr << "PsyFilter::saveSong(): Couldn't find appropriate filter for file format version " << version << std::endl;
			return false;
		}
	}
}
