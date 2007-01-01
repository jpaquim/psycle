/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by Stefan Nattkemper  *
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
#include "ntab.h"
#include "nframeborder.h"
#include "napp.h"
#include "nconfig.h"

NTab::NTab()
 : NCustomButton()
{
  init();
}

NTab::NTab( const std::string & text ) : NCustomButton(text)
{
  init();
}

void NTab::init( )
{
  setToggle(true);

  downSk = NApp::config()->skin("tabup");
  noneSk = NApp::config()->skin("tabnone");

  setSkin(noneSk);

  enablePopup_ = false;
  popupMenu_ = new NPopupMenu();
  add( popupMenu_ );
}

NTab::~NTab()
{
}

void NTab::setDown( bool on )
{
  NCustomButton::setDown(on);
  if (down()) {
    setSkin(downSk);
  } else {
    setSkin(noneSk);
  }
}

int NTab::preferredHeight( ) const
{
  return ((down()) ? 5 : 0 ) + NCustomButton::preferredHeight();
}

void NTab::setEnablePopupMenu( bool on )
{
  enablePopup_ = on;
}

NPopupMenu * NTab::popupMenu( )
{
  return popupMenu_;
}

void NTab::onMousePress( int x, int y, int button )
{
  std::cout << "tab pressed " << std::endl;
  if (enablePopup_ && button == 3) {
      popupMenu_->setPosition(x + absoluteLeft() + window()->left(), y + absoluteTop() + window()->top(),100,100);
      popupMenu_->setVisible(true);
  } else
  if ( !( toggle() && down() ) )
  NCustomButton::onMousePress( x, y, button );
}







