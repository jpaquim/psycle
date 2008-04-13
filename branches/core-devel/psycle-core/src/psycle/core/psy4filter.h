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
#include <fstream>
#include <map>

namespace psy { namespace core {

class Pattern;
class Machine;

/**
@author  Stefan Nattkemper
*/
template class<T>
class Psy4Filter<T> : public Psy3Filter<T>
{
	///\name Singleton Pattern
	///\{ 
		protected:
				Psy4Filter();
		private:
			Psy4Filter( Psy4Filter const & );
			Psy4Filter& operator=(Psy4Filter const&);
		public:
				static Psy4Filter* Instance() {
					// don`t use multithreaded
					static Psy4Filter s;
					return &s; 
				}
	///\}

	public:
		/*override*/ int version() const { return 4; }
		/*override*/ std::string filePostfix() const { return "psy"; }
		/*override*/ bool testFormat(const std::string & fileName);
		/*override*/ bool load(const std::string & fileName, T & song, MachineFactory& factory);
		/*override*/ bool save( const std::string & fileName, const T & song );

	protected:
		/*override*/ int LoadSONGv0(RiffFile* file,T& song);
		bool saveSONGv0(RiffFile* file,const T& song);
		bool saveMACDv0(RiffFile* file,const T& song,int index);
		bool saveINSDv0(RiffFile* file,const T& song,int index);
		bool saveWAVEv0(RiffFile* file,const T& song,int index);

	private:
		std::fstream _stream;

		PatternCategory* lastCategory;
		Pattern* lastPattern;
		SequenceLine* lastSeqLine;
		Machine* lastMachine;
		float lastPatternPos;

		std::map<int, Pattern*> patMap;

		T* song_;
};

}}
#endif
