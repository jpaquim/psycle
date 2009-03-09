// This program is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// copyright 2007-2009 members of the psycle project http://psycle.sourceforge.net

#ifndef PSYCLE__CORE__SONG_SERIALIZER__INCLUDED
#define PSYCLE__CORE__SONG_SERIALIZER__INCLUDED
#pragma once

#include <psycle/core/config.hpp>

#include <string>
#include <vector>

namespace psy { namespace core {

class PsyFilterBase;
class CoreSong;
class MachineFactory;

class PSYCLE__CORE__DECL SongSerializer
{
	public:
		SongSerializer();
		~SongSerializer();

		bool loadSong(const std::string & fileName, CoreSong& song);
		bool saveSong(const std::string & fileName, const CoreSong& song, int version);

	private:
		std::vector<PsyFilterBase*>  filters;
};

}}
#endif
