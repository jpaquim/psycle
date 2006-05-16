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
#include "ngrs/nbutton.h"
#include "ngrs/napp.h"
#include "ngrs/nconfig.h"
#include "ngrs/nwindow.h"
#include "ngrs/nlabel.h"
#include "ngrs/nframeborder.h"
#include "ngrs/nbutton.h"
#include "ngrs/nproperty.h"

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

NButton::NButton( const std::string & text, bool flat ) : NCustomButton(text), icon_(0)
{
  init();
  setFlat(flat);
}

NButton::NButton( NImage * icon )
{
  icon_ = icon;
  icon_->setHAlign(nAlCenter);
  icon_->setVAlign(nAlCenter);
  setPreferredSize(icon->preferredWidth(),icon->preferredHeight());
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

// class factories

extern "C" NObject* createButton() {
    return new NButton();
}

extern "C" void destroyButton(NObject* p) {
    delete p;
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
  if (hint!=0 && hint->mapped()) hint->setVisible(false);
}

void NButton::onMouseEnter( )
{
  if (!down()) setSkin(btnOver_);
  repaint();
  if (hint!=0 && !hint->mapped()) {
     hint->setPosition(window()->left()+ absoluteLeft() + (int) ((3*spacingWidth())/4), window()->top()+absoluteTop() + spacingHeight(),100,100);
     hint->pack();
     hint->setVisible(true);
  }
}

void NButton::setFlat( bool on )
{
  flat_ = on;
  if (flat_) setSkin(btnFlat_); else setSkin(btnUp_);
}

void NButton::init( )
{
  flat_ = true;

  btnUp_ = NApp::config()->skin("btnup");
  btnOver_ = NApp::config()->skin("btnover");
  btnDown_ = NApp::config()->skin("btndown");
  btnFlat_ = NApp::config()->skin("btnflat");

  setSkin(btnFlat_);

  hint = 0;
  hintLbl = 0;

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

void NButton::setHint( const std::string & text )
{
  if (hint==0) {
     hint = new NWindow();
       hintLbl = new NLabel();
         hint->pane()->setBackground(NColor(0xFF,0xFF,0xD0));
         hint->pane()->setBorder(NFrameBorder());
         hint->pane()->setSpacing(NSize(2,2,2,2));
         hint->setDecoration(false);
       hint->pane()->add(hintLbl, nAlClient);
     add(hint);
  }
  hintLbl->setText(text);
}






