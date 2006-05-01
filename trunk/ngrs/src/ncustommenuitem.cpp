/***************************************************************************
 *   Copyright (C) 2005 by Stefan   *
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
#include "ncustommenuitem.h"
#include "napp.h"
#include "nconfig.h"



NCustomMenuItem::NCustomMenuItem()
 : NCustomItem()
{
  NApp::config()->setSkin(&itemNone,"mitemnone");
  NApp::config()->setSkin(&itemOver,"mitemover");

  setSkin(itemNone);
}


NCustomMenuItem::~NCustomMenuItem()
{
}

void NCustomMenuItem::add( class NMenu * menu )
{
}

void NCustomMenuItem::add( NRuntime * comp )
{
  NCustomItem::add(comp);
}

void NCustomMenuItem::add( NVisualComponent * comp, int align )
{
  NCustomItem::add(comp, align);
}

void NCustomMenuItem::onMouseEnter( )
{
  setSkin(itemOver);
  repaint();
}

void NCustomMenuItem::onMouseExit( )
{
  setSkin(itemNone);
  repaint();
}

void NCustomMenuItem::onMousePress( int x, int y, int button )
{
  NButtonEvent ev(this,x,y,button,"ngrs_menu_item_click");
  sendMessage(&ev);

  click.emit(&ev);
}
