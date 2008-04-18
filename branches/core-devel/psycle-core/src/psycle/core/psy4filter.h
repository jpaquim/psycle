// -*- mode:c++; indent-tabs-mode:t -*-
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
#ifndef PSYCLE__CORE__PSY4FILTER__INCLUDED
#define PSYCLE__CORE__PSY4FILTER__INCLUDED
#pragma once

#include "psy3filter.h"

namespace psy { namespace core {

/**
@author  Stefan Nattkemper
*/
class Psy4Filter : public Psy3Filter
{
	///\name Singleton SinglePattern
	///\{ 
		protected:
			Psy4Filter();
		private:
			Psy4Filter( Psy4Filter const & );
			Psy4Filter& operator=(Psy4Filter const&);
		public:
			static Psy4Filter* getInstance() {
				// don`t use multithreaded
				static Psy4Filter s;
				return &s; 
			}
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
		bool saveMACDv0(RiffFile* file,const CoreSong& song,int index);
		bool saveINSDv0(RiffFile* file,const CoreSong& song,int index);
		bool saveWAVEv0(RiffFile* file,const CoreSong& song,int index);

};

}}
#endif
