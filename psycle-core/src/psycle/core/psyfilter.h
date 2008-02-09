/* -*- mode:c++, indent-tabs-mode:t -*- */
/**************************************************************************
*   Copyright 2007 Psycledelics http://psycle.sourceforge.net             *
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
#ifndef PSYCLE__CORE__PSYFILTER__INCLUDED
#define PSYCLE__CORE__PSYFILTER__INCLUDED
#pragma once

#include <string>
#include <vector>

namespace psy { namespace core {

class CoreSong;
class MachineCallbacks;

/**
@author  Psycledelics  
*/
class RiffFile;
class PsyFilterBase
{
	public:
		virtual ~PsyFilterBase() {}

		//signals
		//sigslot::signal2<const std::string &, const std::string &> report;
		//sigslot::signal3<const std::int32_t& , const std::int32_t& , const std::string& > progress;

	public:
		virtual int version() const = 0;
		virtual std::string filePostfix() const = 0;
		virtual bool testFormat(const std::string & fileName) = 0;
		virtual bool load(const std::string & plugin_path, const std::string & fileName, CoreSong & song, MachineCallbacks *callbacks ) = 0;
		/// virtual function to use for loading UI-Specific data, or other extra-data that is not basic to song.
		virtual bool loadExtra(RiffFile* file,char* header, int version) =0;
		virtual bool save(const std::string & fileName, const CoreSong & song) = 0;
		/// virtual function to use for saving UI-Specific data, or other extra-data that is not basic to song.
		virtual bool saveExtra(RiffFile* file,char* header, int version) =0;

		///\todo: Have to allow a way to add a new filter to psyfilter, which will allow to create derived classes of basic filters (psy2, 3, 4, whatever..)
		/// to add specific "saveextra" and "loadextra" codes, which in turn allow extended Songs (for example, save some windows specific options that 
		/// the linux version doesn't need to know about, etc..)
};

/**
@author  Psycledelics  
*/
class PsyFilters
{
	public:
		PsyFilters();

		bool loadSong(std::string const & plugin_path, const std::string & fileName, CoreSong & song, MachineCallbacks* callbacks);
		bool saveSong(const std::string & fileName, const CoreSong & song, int version);

		//signals
		//sigslot::signal2<const std::string &, const std::string &> report;
		//sigslot::signal3<const std::int32_t& , const std::int32_t& , const std::string& > progress;

	private:
		std::vector<PsyFilterBase*> filters;
};

}}
#endif
