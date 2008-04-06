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
#ifndef PSYCLE__CORE__PATTERN_DATA__INCLUDED
#define PSYCLE__CORE__PATTERN_DATA__INCLUDED
#pragma once

#include <vector>
#include <string>

namespace psy { namespace core {

/**
@author  Psycledelics  
*/
class Pattern;

/**
@author  Psycledelics  
*/
class PatternCategory : public std::vector<Pattern*> {
	public:
		PatternCategory();
		PatternCategory(const std::string & name);
		~PatternCategory();

		void setName(const std::string & name);
		const std::string & name() const;

		Pattern* createNewPattern(const std::string & name);
		Pattern* clonePattern( const Pattern & src, const std::string & name);
		bool removePattern(Pattern* pattern);

		void setColor(long color);
		long color() const;

		Pattern* findById(int id);

		void setID(int id);
		int id() const;

		std::string toXml() const;

	private:

		std::string name_;
		long color_;

		int id_;
		static int idCounter;

	};


/**
@author  Psycledelics  
*/
class PatternPool : public std::vector<PatternCategory*> {
	public:
		PatternPool();
		~PatternPool();

		PatternCategory* createNewCategory(const std::string & name);
		void removeSinglePattern(Pattern* pattern);
		void removeAll();
		void resetToDefault();

		Pattern* findById(int id);

		std::string toXml() const;

};

}}
#endif
