/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by Stefan Nattkemper   *
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
#include "colorchooser.h"

namespace ngrs {

  ColorChooser::ColorChooser()
    : Panel()
  {
    chooseSize = 10;
    cols = 4;
    initColorMap();
  }


  ColorChooser::~ColorChooser()
  {
  }

  void ColorChooser::initColorMap( )
  {
    colorMap.push_back(Color(0,0,0));
    colorMap.push_back(Color(255,255,255));
    colorMap.push_back(Color(0,0,255));
    colorMap.push_back(Color(0,255,0));
    colorMap.push_back(Color(255,0,0));

  }

  void ColorChooser::paint( Graphics& g )
  {
    std::vector<Color>::iterator it = colorMap.begin();

    int colCount = 0;
    int xp = 0;
    int yp = 0;

    for ( ; it < colorMap.end(); it++) {
      Color & color = *it;
      g.setForeground(color);

      g.fillRect(xp,yp, chooseSize, chooseSize);

      if (cols == colCount) {
        xp = 0;
        colCount = 0;
        yp+=chooseSize;
      } else {
        xp+=chooseSize;
        colCount++;
      }
    }
  }

  int ColorChooser::preferredWidth( ) const
  {
    return cols*chooseSize;
  }

  int ColorChooser::preferredHeight( ) const
  {
    return d2i((colorMap.size() / (double) cols) * chooseSize);
  }

  void ColorChooser::onMousePress( int x, int y, int button )
  {
    if (button == 1) {
      int col = d2i(x / chooseSize);
      int row = d2i(y / chooseSize);

      unsigned int index = col*row + col;
      if ( index < colorMap.size() ) {
        selectedColor_ = colorMap.at(index);
        colorSelected.emit(selectedColor_);
      }
    }
  }

  const Color & ColorChooser::selectedColor( ) const
  {
    return selectedColor_;
  }

}
