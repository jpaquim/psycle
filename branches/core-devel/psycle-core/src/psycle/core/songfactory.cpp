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
#include "songfactory.h"
#include "psy2filter.h"
#include "psy3filter.h"
#include "psy4filter.h"

namespace psy
{
	namespace core
	{
		SongFactory<T>::SongFactory(MachineFactory& factory1)
		:factory(factory1)
		{
			filters.push_back( new Psy2Filter<T>() );
			filters.push_back( new Psy3Filter<T>() );
			filters.push_back( new Psy4Filter<T>() );
		}
		SongFactory<T>::~SongFactory() {
			for (std::vector<PsyFilterBase*>::iterator it = filters.begin(); it < filters.end(); ++it) {
				delete *it;
			}
		}
		
		T* SongFactory<T>::createEmptySong() {
			T* song = new T();
			song.addMachine(factory.CreateMachine(MachineKey::master(),MASTER_INDEX));
			return song;
		}
		T* SongFactory<T>::loadSong(const std::string & fileName)
		{
			T* song = new T();
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

		bool SongFactory<T>::saveSong( const std::string & fileName, const T& song, int version )
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
			std::cerr << "SongFactory<T>::saveSong(): Couldn't find appropriate filter for file format version " << version << std::endl;
			return false;
		}
	}
}
