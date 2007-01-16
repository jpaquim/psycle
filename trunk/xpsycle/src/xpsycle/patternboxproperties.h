/***************************************************************************
 *   Copyright (C) 2006 by Stefan Nattkemper   *
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
#ifndef PATTERNBOXPROPERTIES_H
#define PATTERNBOXPROPERTIES_H

#include <ngrs/flipbox.h>


/**
@author Stefan Nattkemper
*/

namespace ngrs {
  class ColorComboBox;
  class Edit;
}

namespace psycle { 
  namespace host {

    class CategoryItem;

    class PatternBoxProperties : public ngrs::NFlipBox
    {
    public:
      PatternBoxProperties();

      ~PatternBoxProperties();

      void setName( const std::string & name );
      const std::string & name() const;

      sigslot::signal1<const std::string &> nameChanged;
      sigslot::signal1<const ngrs::Color &> colorChanged;

      void setCategoryItem( CategoryItem* cat );

    private:

      CategoryItem* cat_;

      ngrs::ColorComboBox* clBox;
      ngrs::Edit* categoryEdt;

      void onKeyPress( const ngrs::KeyEvent& );
      void onColorChange( const ngrs::Color& color );

    };

  }
}

#endif
