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
#ifndef PATTERNDATA_H
#define PATTERNDATA_H

#include "singlepattern.h"
#include <vector>

/**
@author  Stefan Nattkemper
*/

namespace psy
{
	namespace core
	{


		class PatternCategory  {
		public:
			PatternCategory();
			PatternCategory(const std::string & name);
            PatternCategory( PatternCategory* parent, const std::string & name );
			~PatternCategory();

			void setName(const std::string & name);
			const std::string & name() const;

			SinglePattern* createNewPattern(const std::string & name);
            SinglePattern* clonePattern( const SinglePattern & src, const std::string & name);
			bool removePattern(SinglePattern* pattern);

			void setColor(long color);
			long color() const;

			SinglePattern* findById(int id);

			void setID(int id);
			int id() const;

			std::string toXml() const;

            const std::vector<SinglePattern*>& patterns() const;
            const std::vector<PatternCategory*>& children() const;

            PatternCategory* createNewCategory( std::string& name );

            PatternCategory* parent() const;

            void removeAll();
            void resetToDefault();


		private:

            PatternCategory* parent_;
			std::string name_;
			long color_;
			int id_;
			static int idCounter;
            
            std::vector<SinglePattern*> patterns_;
            std::vector<PatternCategory*> children_;

		};


		class PatternData : public std::vector<PatternCategory*> {
		public:
			PatternData();

			~PatternData();

			PatternCategory* createNewCategory(const std::string & name);
			void removeSinglePattern(SinglePattern* pattern);
			void removeAll();
            void resetToDefault();

			SinglePattern* findById(int id);

			std::string toXml() const;

		};

	}
}
#endif
