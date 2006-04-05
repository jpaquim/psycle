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
#include "nbutton.h"
#include "napp.h"
#include "nconfig.h"

using namespace std;

NButton::NButton( ) : NCustomButton(), flat_(1), icon_(0)
{
  init();
}

NButton::NButton( const std::string & text ) : NCustomButton(text), flat_(1), icon_(0)
{
  init();
}

NButton::~ NButton( )
{
}

NButton::NButton( const std::string & text, int minWidth, int minHeight ) : NCustomButton(text), flat_(1), icon_(0)
{
  setMinimumWidth(minWidth);
  //setMinimumHeight(minHeight);
  init();
}

NButton::NButton( NImage * icon )
{
  icon_ = icon;
  icon_->setHAlign(nAlCenter);
  icon_->setVAlign(nAlCenter);
  add(icon);
  init();
}


NButton::NButton( NImage * icon , int minWidth, int minHeight) : NCustomButton(), flat_(1)
{
  icon_ = icon;
  icon_->setHAlign(nAlCenter);
  icon_->setVAlign(nAlCenter);
  setMinimumWidth(minWidth);
  add(icon);
  init();
}

void NButton::resize( )
{
  NCustomButton::resize();
  if (icon_!=0) {
    icon_->setPosition(0,0,spacingWidth(),spacingHeight());//clientHeight(),clientWidth());
  }
}

int NButton::preferredWidth( ) const
{
  if (ownerSize()) return NVisualComponent::preferredWidth();

  if (icon_==0) return NCustomButton::preferredWidth();
  return icon_->preferredWidth() + spacing().left() + spacing().right();
}

int NButton::preferredHeight( ) const
{
  if (ownerSize()) return NVisualComponent::preferredHeight();

  if (icon_==0) return NCustomButton::preferredHeight();
  return icon_->preferredHeight() + spacing().top() + spacing().bottom();
}

void NButton::onMouseExit( )
{
  if (!down()) {
     if (flat_) {
        setSkin(btnFlat_);
     } else setSkin(btnUp_);
  }

  repaint();
}

void NButton::onMouseEnter( )
{
  if (!down()) setSkin(btnOver_);
  repaint();
}

void NButton::setFlat( bool on )
{
  flat_ = on;
  if (flat_) setSkin(btnFlat_); else setSkin(btnUp_);
}

void NButton::init( )
{
  flat_ = true;

  NApp::config()->setSkin(&btnUp_,  "btnup");
  NApp::config()->setSkin(&btnOver_,"btnover");
  NApp::config()->setSkin(&btnDown_,"btndown");
  NApp::config()->setSkin(&btnFlat_,"btnflat");

  setSkin(btnFlat_);
}

void NButton::setDown( bool on )
{
  NCustomButton::setDown(on);

  if (down()) {
    setSkin(btnDown_);
    repaint();
  } else {
    setSkin(btnUp_);
    repaint();
  }
}




