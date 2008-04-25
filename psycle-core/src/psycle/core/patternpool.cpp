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
//
#include "patternpool.h"
#include "helpers/xml.h"
#include "singlepattern.h"
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
	for (std::vector<SinglePattern*>::iterator it = begin(); it < end(); it++) {
		delete *it;
	}
}

SinglePattern* PatternCategory::createNewPattern( const std::string & name )
{
	SinglePattern* pattern = new SinglePattern();
	pattern->setCategory(this);
	pattern->setName(name);
	push_back(pattern);
	return pattern;
}
						
SinglePattern* PatternCategory::clonePattern( const SinglePattern & src, const std::string & name)
{
	SinglePattern* pattern = new SinglePattern( src);
	pattern->setCategory(this);
	pattern->setName(name);
	push_back(pattern);
	return pattern;
}

bool PatternCategory::removePattern( SinglePattern * pattern )
{
	iterator it = find(begin(), end(), pattern);
	if ( it != end() ) {
		SinglePattern* pattern = *it;
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

SinglePattern * psy::core::PatternCategory::findById( int id )
{
	for (std::vector<SinglePattern*>::iterator it = begin(); it < end(); it++) {
		SinglePattern* pat = *it;
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
		SinglePattern* pattern = *it;
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

void PatternPool::removeSinglePattern( SinglePattern * pattern )
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

SinglePattern * PatternPool::findById( int id )
{
	iterator it = begin();
	for ( ; it < end(); it++) {
		PatternCategory* cat = *it;
		SinglePattern* pat = cat->findById(id);
		if (pat) return pat;
	}
	return 0;
}

std::string PatternPool::toXml( ) const
{
	std::ostringstream xml;
	xml << "<patterndata>" << std::endl;
	for ( const_iterator it = begin(); it < end(); it++) {
		PatternCategory* category = *it;
		xml << category->toXml();
	}
	xml << "</patterndata>" << std::endl;
	return xml.str();
}

}}
