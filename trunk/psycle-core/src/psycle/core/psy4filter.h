// This program is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// copyright 2007-2009 members of the psycle project http://psycle.sourceforge.net

#ifndef PSYCLE__CORE__PSY4FILTER__INCLUDED
#define PSYCLE__CORE__PSY4FILTER__INCLUDED
#pragma once

#include "psy3filter.h"

namespace psy { namespace core {

/**
@author  Stefan Nattkemper
*/
class PSYCLE__CORE__DECL Psy4Filter : public Psy3Filter
{
	///\name Singleton Pattern
	///\{ 
		protected:
			Psy4Filter();
		private:
			Psy4Filter( Psy4Filter const & );
			Psy4Filter& operator=(Psy4Filter const&);
		public:
			static Psy4Filter* getInstance();
	///\}

	public:
		/*override*/ int version() const { return 4; }
		/*override*/ std::string filePostfix() const { return "psy"; }
		/*override*/ bool testFormat(const std::string & fileName);
		/*override*/ bool load(const std::string & fileName, CoreSong & song);
		/*override*/ bool save( const std::string & fileName, const CoreSong & song );

	protected:
		/*override*/ int LoadSONGv0(RiffFile* file,CoreSong& song);
		bool saveSONGv0(RiffFile* file,const CoreSong& song);
		bool loadMACDv1(RiffFile* file,CoreSong& song, int minorversion);
		bool saveMACDv1(RiffFile* file,const CoreSong& song,int index);
		bool saveINSDv0(RiffFile* file,const CoreSong& song,int index);
		bool saveWAVEv0(RiffFile* file,const CoreSong& song,int index);

};

}}
#endif
