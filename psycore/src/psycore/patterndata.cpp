/***************************************************************************
 *   Copyright (C) 2006 by  Stefan Nattkemper   *
 *   natti@linux   *
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
#include "patterndata.h"
#include <algorithm>
#include <sstream>

#ifdef _MSC_VER
#undef min 
#undef max
#endif

namespace psy
{
	namespace core
	{

		int PatternCategory::idCounter = 0;

		PatternCategory::PatternCategory()
		{
			color_ = 0x00FF0000;
			id_ = idCounter;
			idCounter++;
            parent_ = 0;
		}

        PatternCategory::PatternCategory( PatternCategory* parent, const std::string& name ) {
	      name_ = name;
		  color_ = 0x00FF0000;
		  id_ = idCounter;
		  idCounter++;
          parent_ = parent;
        }

		PatternCategory::PatternCategory( const std::string& name )
		{
			name_ = name;
			color_ = 0x00FF0000;
			id_ = idCounter;
			idCounter++;
            parent_ = 0;
		}

		PatternCategory::~ PatternCategory( )
		{
			for (std::vector<SinglePattern*>::iterator it = patterns_.begin(); it < patterns_.end(); it++ ) {
				delete *it;
			}
            for (std::vector<PatternCategory*>::iterator it = children_.begin(); it < children_.end(); it++ ) {
				delete *it;
			}
		}

		SinglePattern* PatternCategory::createNewPattern( const std::string& name )
		{
			SinglePattern* pattern = new SinglePattern();
			pattern->setCategory(this);
			pattern->setName(name);
			patterns_.push_back(pattern);
			return pattern;
		}
                
        SinglePattern* PatternCategory::clonePattern( const SinglePattern & src, const std::string & name)
        {
                     SinglePattern* pattern = new SinglePattern( src);
                     pattern->setCategory(this);
                     pattern->setName(name);
                     patterns_.push_back(pattern);
                     return pattern;
		}

		bool PatternCategory::removePattern( SinglePattern * pattern )
		{
          std::vector<SinglePattern*>::iterator it = find( patterns_.begin(), patterns_.end(), pattern );
		  if ( it != patterns_.end() ) {
			SinglePattern* pattern = *it;
			patterns_.erase(it);
			delete(pattern);
			return true;
          } else {
            std::vector<PatternCategory*>::iterator cat_it = children_.begin();
            for ( ; cat_it < children_.end(); cat_it++) {
              if ( (*cat_it)->removePattern( pattern ) ) return true;
            }
          }
		  return false;
		}

		void PatternCategory::setName( const std::string& name )
		{
			name_ = name;
		}

		const std::string& PatternCategory::name( ) const
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

		SinglePattern* PatternCategory::findById( int id )
		{
			for (std::vector<SinglePattern*>::iterator it = patterns_.begin(); it < patterns_.end(); it++) {
				SinglePattern* pat = *it;
				if (pat->id() == id) return pat;
			}
            for (std::vector<PatternCategory*>::iterator cat_it = children_.begin(); cat_it < children_.end(); cat_it++ ) {
              SinglePattern* pat = (*cat_it)->findById( id );
              if ( pat ) return pat;
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
		{/*
			std::ostringstream xml;
//			xml << "<category name='" << PsyFilter::replaceIllegalXmlChr( name() ) << "' color='" << color_ << "' >" << std::endl;
            for ( std::vector<SinglePattern*>::const_iterator it = patterns_.begin(); it < patterns_.end(); it++ ) {
				SinglePattern* pattern = *it;
				xml << pattern->toXml();
			}
			xml << "</category>" << std::endl;
			return xml.str();*/
          return "broken!";
		}

        const std::vector<SinglePattern*>& PatternCategory::patterns() const {
          return patterns_;
        }

        const std::vector<PatternCategory*>& PatternCategory::children() const {
          return children_;
        }

        PatternCategory* PatternCategory::parent() const {
          return parent_;
        }

        PatternCategory* PatternCategory::createNewCategory( std::string& name ) {
          PatternCategory* cat = new PatternCategory(this, name);
          children_.push_back( cat );
          return cat;
        }

        void PatternCategory::removeAll() {
	      	for (std::vector<SinglePattern*>::iterator it = patterns_.begin(); it < patterns_.end(); it++) {
				SinglePattern* pat = *it;
				delete* pat;
			}
            for (std::vector<PatternCategory*>::iterator cat_it = children_.begin(); cat_it < children_.end(); cat_it++ ) {
              cat_it->removeAll();
            }
            children_.clear();
        }

        void PatternCategory::resetToDefault()
        {
          removeAll();
          PatternCategory* cat = createNewCategory( "default" );
          cat->createNewPattern( "pattern0" );
        }
		// end of Category class


		// the pattern data class
		PatternData::PatternData()
		{
		}

		PatternData::~PatternData()
		{
			for (std::vector<PatternCategory*>::iterator it = begin(); it < end(); it++) {
				delete *it;
			}
		}


		PatternCategory * PatternData::createNewCategory( const std::string& name )
		{
			PatternCategory* category = new PatternCategory();
			category->setName(name);
			push_back(category);
			return category;
		}

		void PatternData::removeSinglePattern( SinglePattern * pattern )
		{
			iterator it = begin();
			for ( ; it < end(); it++) {
				PatternCategory* cat = *it;
				if (cat->removePattern(pattern)) break;
			}
		}

		void PatternData::removeAll( )
		{
			for (std::vector<PatternCategory*>::iterator it = begin(); it < end(); it++) {
				delete *it;
			}
			clear();
		}

                void PatternData::resetToDefault()
                {
                        removeAll();
                        PatternCategory* cat = createNewCategory( "default" );
                        cat->createNewPattern( "pattern0" );

                }

		SinglePattern * PatternData::findById( int id )
		{
			iterator it = begin();
			for ( ; it < end(); it++) {
				PatternCategory* cat = *it;
				SinglePattern* pat = cat->findById(id);
				if (pat) return pat;
			}
			return 0;
		}

		std::string PatternData::toXml( ) const
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

	}
}
