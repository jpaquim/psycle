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
#include "tab.h"
#include "frameborder.h"
#include "app.h"
#include "config.h"

namespace ngrs {

  NTab::NTab()
    : CustomButton()
  {
    init();
  }

  NTab::NTab( const std::string & text ) : CustomButton(text)
  {
    init();
  }

  void NTab::init( )
  {
    setToggle(true);

    downSk = App::config()->skin("tabup");
    noneSk = App::config()->skin("tabnone");

    CustomButton::setSkin(noneSk);

    enablePopup_ = false;
    popupMenu_ = new NPopupMenu();
    add( popupMenu_ );

    heightIdent_ = 5;
  }

  NTab::~NTab()
  {
  }

  void NTab::setDown( bool on )
  {
    CustomButton::setDown(on);
    if (down()) {
      CustomButton::setSkin(downSk);
    } else {
      CustomButton::setSkin(noneSk);
    }
  }

  int NTab::preferredHeight( ) const
  {
    return ((down()) ? heightIdent_ : 0 ) + CustomButton::preferredHeight();
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
        CustomButton::onMousePress( x, y, button );
  }

  void NTab::setSkin( const Skin & noneSkin, const Skin & downSkin, int ident ) {
    noneSk = noneSkin;
    downSk = downSkin;
    heightIdent_ = ident;
    if (down()) {
      CustomButton::setSkin( downSk );
    } else {
      CustomButton::setSkin( noneSk );
    }
  }
 
}
