/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by  Stefan Nattkemper   *
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
#include "splashscreen.h"
#include "frameborder.h"
#include "image.h"

namespace ngrs {

  SplashScreen::SplashScreen()
    : Window()
  {
    setDecoration(false);
    pane()->setBorder(FrameBorder());
    setPositionToScreenCenter();
  }

  SplashScreen::~SplashScreen()
  {
  }

  void SplashScreen::loadImageFromFile( const std::string & fileName )
  {
    Image* img = new Image();
      img->loadFromFile( fileName );      
    pane()->add( img, nAlClient );
    
    setPosition( 0, 0, img->width(), img->height() );
    setPositionToScreenCenter();
  }

}
