/***************************************************************************
*   Copyright (C) 2007 by  Stefan Nattkemper   *
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
#include "startpage.h"
#include "logopanel.h"
#include "defaultbitmaps.h"
#include <ngrs/alignlayout.h>
#include <ngrs/frameborder.h>

namespace psy {
  namespace host {

    StartPage::StartPage( DefaultBitmaps& icons )
      : Module()
    {
      ngrs::FrameBorder frBorder;
      frBorder.setOval();
      setBorder( frBorder );

      setLayout( ngrs::AlignLayout() );
      add( new LogoPanel(icons), ngrs::nAlBottom );
    }

    StartPage::~StartPage()
    {
    }

  }
}