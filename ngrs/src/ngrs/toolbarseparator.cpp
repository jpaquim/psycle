/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by  Stefan Nattkemper  *
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
#include "toolbarseparator.h"
#include "flowlayout.h"
#include "app.h"
#include "config.h"

namespace ngrs {

  ToolBarSeparator::ToolBarSeparator()
    : Panel()
  {
    setSkin( App::config()->skin("tool_sep") );
    setAlign( nAlLeft );
  }


  ToolBarSeparator::~ToolBarSeparator()
  {
  }

  void ToolBarSeparator::paint( Graphics& g )
  {
    if ( align() == nAlTop ) {
      g.drawLine( 0, 1, clientWidth(), 1 );   
    } else 
    {
      g.drawLine( 1, 0, 1, clientHeight() );
    } 
  }

  int ToolBarSeparator::preferredWidth( ) const
  {
    return ( align() == nAlTop ) ?  15 : 3;
  }

  int ToolBarSeparator::preferredHeight( ) const
  {
    return ( align() == nAlTop ) ? 3 : 15;
  }

}
