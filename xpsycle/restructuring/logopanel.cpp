/***************************************************************************
*   Copyright (C) 2007 by  Stefan Nattkemper  *
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
#include "logopanel.h"
#include <ngrs/image.h>
#include <ngrs/alignlayout.h>

namespace psy {
  namespace host {


    LogoPanel::LogoPanel( DefaultBitmaps& icons )
    {
      setLayout( ngrs::AlignLayout() );
      setSpacing( ngrs::Size( 10,5,10,5 ) );

      ngrs::Image* img = new ngrs::Image( icons.logoRight()  );
      img->setVAlign( ngrs::nAlCenter );
      add( img , ngrs::nAlRight );

      /*img = new ngrs::Image( icons.logoLeft() );
      img->setVAlign( ngrs::nAlCenter );
      add( img , ngrs::nAlLeft );*/

      img = new ngrs::Image( icons.logoMid() );
      img->setVAlign( ngrs::nAlCenter );
      img->setHAlign( ngrs::nAlWallPaper );
      img->setEvents( true );
      add( img , ngrs::nAlClient );
    }

    LogoPanel::~LogoPanel()
    {
    }

  }
}