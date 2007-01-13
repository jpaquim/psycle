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
#include "nbutton.h"
#include "napp.h"
#include "nconfig.h"
#include "nwindow.h"
#include "nlabel.h"
#include "nframeborder.h"
#include "nbutton.h"
#include "nproperty.h"
#include "nhint.h"

namespace ngrs {

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

  void NButton::setSkin( const NSkin & up, const NSkin & down, const NSkin & over, const NSkin & flat ) {
    btnUp_   = up;
    btnDown_ = down;
    btnOver_ = over;
    btnFlat_ = flat;

    setFlat( flat_ );
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
        NCustomButton::setSkin(btnFlat_);
      } else NCustomButton::setSkin(btnUp_);
    }
    repaint();
    if (hint!=0 && hint->mapped()) hint->setVisible(false);
  }

  void NButton::onMouseEnter( )
  {
    if (!down()) NCustomButton::setSkin(btnOver_);
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
    if (flat_) 
      NCustomButton::setSkin( btnFlat_ ); 
    else 
      NCustomButton::setSkin( btnUp_ );
  }

  void NButton::init( )
  {
    flat_ = true;

    btnUp_   = NApp::config()->skin("btnup");
    btnOver_ = NApp::config()->skin("btnover");
    btnDown_ = NApp::config()->skin("btndown");
    btnFlat_ = NApp::config()->skin("btnflat");

    NCustomButton::setSkin( btnFlat_ );

    hint = 0;

    // inits the repeat Mode for the button .. at default off
    repeatMode_ = false;
    button_ = 1;
    repeatTimer.setIntervalTime(50);
    repeatTimer.timerEvent.connect(this,&NButton::onRepeatTimer);
    startLatencyTimer.setIntervalTime(100);
    startLatencyTimer.timerEvent.connect(this,&NButton::onStartTimer);

    setTabStop(true);
  }

  void NButton::setDown( bool on )
  {
    NCustomButton::setDown(on);

    if (down()) {
      NCustomButton::setSkin(btnDown_);
      repaint();
    } else {
      NCustomButton::setSkin(btnUp_);
      repaint();
    }
  }

  void NButton::setHint( const std::string & text )
  {
    if (hint==0) {
      hint = new NHint();
      hint->setText( text );
      add(hint);
    } else
      hint->setText(text);
  }

  void NButton::setRepeatMode( bool on )
  {
    repeatMode_ = on;
  }

  void NButton::setRepeatPolicy( int interval, int startLatency )
  {
    repeatTimer.setIntervalTime(interval);
    startLatencyTimer.setIntervalTime(startLatency);
  }

  void NButton::onMousePress( int x, int y, int button )
  {
    NCustomButton::onMousePress(x, y, button);

    if ( repeatMode_ ) {
      if (!startLatencyTimer.enabled() ) {
        button_ = button;
        startLatencyTimer.enableTimer();
      }
    }
  }

  void NButton::onMousePressed( int x, int y, int button )
  {
    NCustomButton::onMousePressed(x, y, button);

    if ( repeatMode_ ) {
      repeatTimer.disableTimer();
      startLatencyTimer.disableTimer();
    }
  }

  void NButton::onStartTimer( )
  {
    startLatencyTimer.disableTimer();
    repeatTimer.enableTimer();
  }

  void NButton::onRepeatTimer( )
  {
    NCustomButton::onMousePress(0,0,button_);
  }

}

 // class factories
extern "C" ngrs::NObject* createButton() {
  return new ngrs::NButton();
}

extern "C" void destroyButton( ngrs::NObject* p ) {
 delete p;
}
