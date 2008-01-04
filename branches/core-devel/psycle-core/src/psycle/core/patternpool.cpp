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
#include <psycle/core/psycleCorePch.hpp>
#include "PatternPool.h"
#include "helpers/xml.h"
#include "Pattern.h"
#include <sstream>

namespace psy { namespace core {

int PatternCategory::idCounter = 0;

PatternCategory::PatternCategory( )
{
	color_ = 0x00FF0000;
	id_ = idCounter;
	idCounter++;
}

PatternCategory::PatternCategory( const std::string & name )
{
	name_ = name;
	color_ = 0x00FF0000;
	id_ = idCounter;
	idCounter++;
}

PatternCategory::~ PatternCategory( )
{
	for (std::vector<Pattern*>::iterator it = begin(); it < end(); it++) {
		delete *it;
	}
}

Pattern* PatternCategory::createNewPattern( const std::string & name )
{
	Pattern* pattern = new Pattern();
	pattern->setCategory(this);
	pattern->setName(name);
	push_back(pattern);
	return pattern;
}
						
Pattern* PatternCategory::clonePattern( const Pattern & src, const std::string & name)
{
	Pattern* pattern = new Pattern( src);
	pattern->setCategory(this);
	pattern->setName(name);
	push_back(pattern);
	return pattern;
}

bool PatternCategory::removePattern( Pattern * pattern )
{
	iterator it = find(begin(), end(), pattern);
	if ( it != end() ) {
		Pattern* pattern = *it;
		erase(it);
		delete(pattern);
		return true;
	}
	return false;
}

void PatternCategory::setName( const std::string & name )
{
	name_ = name;
}

const std::string & PatternCategory::name( ) const
{
	return name_;
}

void PatternCategory::setColor( long color )
{
	color_ = color;
}

long PatternCategory::color( ) const
{
	return color_;
}

Pattern * psy::core::PatternCategory::findById( int id )
{
	for (std::vector<Pattern*>::iterator it = begin(); it < end(); it++) {
		Pattern* pat = *it;
		if (pat->id() == id) return pat;
	}
	return 0;
}

void PatternCategory::setID( int id )
{
	id_ = id;
	idCounter = std::max(id_,idCounter)+1;
}

int PatternCategory::id( ) const
{
	return id_;
}

std::string PatternCategory::toXml( ) const
{
	std::ostringstream xml;
	xml << "<category name='" << replaceIllegalXmlChr( name() ) << "' color='" << color_ << "' >" << std::endl;
	for ( const_iterator it = begin(); it < end(); it++) {
		Pattern* pattern = *it;
		xml << pattern->toXml();
	}
	xml << "</category>" << std::endl;
	return xml.str();
}

// end of Category class


// the pattern data class
PatternPool::PatternPool()
{
}

PatternPool::~PatternPool()
{
	for (std::vector<PatternCategory*>::iterator it = begin(); it < end(); it++) {
		delete *it;
	}
}


PatternCategory * PatternPool::createNewCategory( const std::string & name )
{
	PatternCategory* category = new PatternCategory();
	category->setName(name);
	push_back(category);
	return category;
}

void PatternPool::removeSinglePattern( Pattern * pattern )
{
	iterator it = begin();
	for ( ; it < end(); it++) {
		PatternCategory* cat = *it;
		if (cat->removePattern(pattern)) break;
	}
}

void PatternPool::removeAll( )
{
	for (std::vector<PatternCategory*>::iterator it = begin(); it < end(); it++) {
		delete *it;
	}
	clear();
}

void PatternPool::resetToDefault()
{
	removeAll();
	PatternCategory* cat = createNewCategory( "default" );
	cat->createNewPattern( "pattern0" );

}

Pattern * PatternPool::findById( int id )
{
	iterator it = begin();
	for ( ; it < end(); it++) {
		PatternCategory* cat = *it;
		Pattern* pat = cat->findById(id);
		if (pat) return pat;
	}
	return 0;
}

std::string PatternPool::toXml( ) const
{
	std::ostringstream xml;
	xml << "<PatternPool>" << std::endl;
	for ( const_iterator it = begin(); it < end(); it++) {
		PatternCategory* category = *it;
		xml << category->toXml();
	}
	xml << "</PatternPool>" << std::endl;
	return xml.str();
}

}}
