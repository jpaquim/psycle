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
#include "button.h"
#include "app.h"
#include "config.h"
#include "window.h"
#include "label.h"
#include "frameborder.h"
#include "button.h"
#include "property.h"
#include "hint.h"

namespace ngrs {

  Button::Button( ) : CustomButton(), flat_(1), icon_(0)
  {
    init();
  }

  Button::Button( const std::string & text ) : CustomButton(text), flat_(1), icon_(0)
  {
    init();
  }

  Button::~ Button( )
  {
  }


  Button::Button( const std::string & text, int minWidth, int minHeight ) : CustomButton(text), flat_(1), icon_(0)
  {
    setMinimumWidth(minWidth);
    //setMinimumHeight(minHeight);
    init();
  }

  Button::Button( const std::string & text, bool flat ) : CustomButton(text), icon_(0)
  {
    init();
    setFlat(flat);
  }

  Button::Button( Image * icon )
  {
    icon_ = icon;
    icon_->setHAlign(nAlCenter);
    icon_->setVAlign(nAlCenter);
    setPreferredSize(icon->preferredWidth(),icon->preferredHeight());
    add(icon);
    init();
  }


  Button::Button( Image * icon , int minWidth, int minHeight) : CustomButton(), flat_(1)
  {
    icon_ = icon;
    icon_->setHAlign(nAlCenter);
    icon_->setVAlign(nAlCenter);
    setMinimumWidth(minWidth);
    add(icon);
    init();
  }

  void Button::setSkin( const Skin & up, const Skin & down, const Skin & over, const Skin & flat ) {
    btnUp_   = up;
    btnDown_ = down;
    btnOver_ = over;
    btnFlat_ = flat;

    setFlat( flat_ );
  }

  void Button::resize( )
  {
    CustomButton::resize();
    if (icon_!=0) {
      icon_->setPosition(0,0,spacingWidth(),spacingHeight());//clientHeight(),clientWidth());
    }
  }

  int Button::preferredWidth( ) const
  {
    if (ownerSize()) return VisualComponent::preferredWidth();

    if (icon_==0) return CustomButton::preferredWidth();
    return icon_->preferredWidth() + spacing().left() + spacing().right();
  }

  int Button::preferredHeight( ) const
  {
    if (ownerSize()) return VisualComponent::preferredHeight();

    if (icon_==0) return CustomButton::preferredHeight();
    return icon_->preferredHeight() + spacing().top() + spacing().bottom();
  }

  void Button::onMouseExit( )
  {
    if (!down()) {
      if (flat_) {
        CustomButton::setSkin(btnFlat_);
      } else CustomButton::setSkin(btnUp_);
    }
    repaint();
    if (hint!=0 && hint->mapped()) hint->setVisible(false);
  }

  void Button::onMouseEnter( )
  {
    if (!down()) CustomButton::setSkin(btnOver_);
    repaint();
    if (hint!=0 && !hint->mapped()) {
      hint->setPosition(window()->left()+ absoluteLeft() + (int) ((3*spacingWidth())/4), window()->top()+absoluteTop() + spacingHeight(),100,100);
      hint->pack();
      hint->setVisible(true);
    }
  }

  void Button::setFlat( bool on )
  {
    flat_ = on;
    if (flat_) 
      CustomButton::setSkin( btnFlat_ ); 
    else 
      CustomButton::setSkin( btnUp_ );
  }

  void Button::init( )
  {
    flat_ = true;

    btnUp_   = App::config()->skin("btnup");
    btnOver_ = App::config()->skin("btnover");
    btnDown_ = App::config()->skin("btndown");
    btnFlat_ = App::config()->skin("btnflat");

    CustomButton::setSkin( btnFlat_ );

    hint = 0;

    // inits the repeat Mode for the button .. at default off
    repeatMode_ = false;
    button_ = 1;
    repeatTimer.setIntervalTime(50);
    repeatTimer.timerEvent.connect(this,&Button::onRepeatTimer);
    startLatencyTimer.setIntervalTime(100);
    startLatencyTimer.timerEvent.connect(this,&Button::onStartTimer);

    setTabStop(true);
  }

  void Button::setDown( bool on )
  {
    CustomButton::setDown(on);

    if (down()) {
      CustomButton::setSkin(btnDown_);
      repaint();
    } else {
      CustomButton::setSkin(btnUp_);
      repaint();
    }
  }

  void Button::setHint( const std::string & text )
  {
    if (hint==0) {
      hint = new NHint();
      hint->setText( text );
      add(hint);
    } else
      hint->setText(text);
  }

  void Button::setRepeatMode( bool on )
  {
    repeatMode_ = on;
  }

  void Button::setRepeatPolicy( int interval, int startLatency )
  {
    repeatTimer.setIntervalTime(interval);
    startLatencyTimer.setIntervalTime(startLatency);
  }

  void Button::onMousePress( int x, int y, int button )
  {
    CustomButton::onMousePress(x, y, button);

    if ( repeatMode_ ) {
      if (!startLatencyTimer.enabled() ) {
        button_ = button;
        startLatencyTimer.enableTimer();
      }
    }
  }

  void Button::onMousePressed( int x, int y, int button )
  {
    CustomButton::onMousePressed(x, y, button);

    if ( repeatMode_ ) {
      repeatTimer.disableTimer();
      startLatencyTimer.disableTimer();
    }
  }

  void Button::onStartTimer( )
  {
    startLatencyTimer.disableTimer();
    repeatTimer.enableTimer();
  }

  void Button::onRepeatTimer( )
  {
    CustomButton::onMousePress(0,0,button_);
  }

}

 // class factories
extern "C" ngrs::Object* createButton() {
  return new ngrs::Button();
}

extern "C" void destroyButton( ngrs::Object* p ) {
 delete p;
}
