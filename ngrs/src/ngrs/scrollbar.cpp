/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by  Stefan Nattkemper                         *
 *   Made in Germany                                                       *
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
#include "scrollbar.h"
#include "window.h"
#include "gradient.h"
#include "app.h"
#include "config.h"

#ifdef _MSC_VER
#undef min 
#undef max
#endif


namespace ngrs {
  /* XPM */
  const char * arrow_up_xpm[] = {
    "12 6 2 1",
    " 	c None",
    ".	c black",
    "            ",
    "     .      ",
    "    ...     ",
    "   .....    ",
    "  .......   ",
    "            "}
  ;

  /* XPM */
  const char * arrow_down_xpm[] = {
    "12 6 2 1",
    " 	c None",
    ".	c black",
    "            ",
    "  .......   ",
    "   .....    ",
    "    ...     ",
    "     .      ",
    "            "}
  ;

  /* XPM */
  const char * arrow_right_xpm[] = {
    "12 12 2 1",
    " 	c None",
    ".	c black",
    "            ",
    "            ",
    "    .       ",
    "    ..      ",
    "    ...     ",
    "    ....    ",
    "    ...     ",
    "    ..      ",
    "    .       ",
    "            ",
    "            ",
    "            "}
  ;

  /* XPM */
  const char * arrow_left_xpm[] = {
    "12 12 2 1",
    " 	c None",
    ".	c black",
    "            ",
    "            ",
    "      .     ",
    "     ..     ",
    "    ...     ",
    "   ....     ",
    "    ...     ",
    "     ..     ",
    "      .     ",
    "            ",
    "            ",
    "            "}
  ;

  ScrollBar::ScrollBar()
    : Panel()
  {
    init();
  }

  ScrollBar::ScrollBar( int orientation )
  {
    init();
    setOrientation(orientation);
  }

  ScrollBar::~ScrollBar()
  {
  }

  void ScrollBar::init( )
  {
    enableFocus( false );

    inc    = new Image();
    dec    = new Image();

    up.createFromXpmData(arrow_up_xpm);
    down.createFromXpmData(arrow_down_xpm);
    left.createFromXpmData(arrow_left_xpm);
    right.createFromXpmData(arrow_right_xpm);

    inc->setBitmap(down);
    dec->setBitmap(up);

    pos_ = 0;
    min_ = 0;
    max_ = 10;
    largeChange_ = 1;
    smallChange_ =  1;

    control_ = 0;
    scrollPolicy_ = nDy;

    orientation_ = nVertical;

    incBtn = new Button(inc,12,12);//up,10,10);
    incBtn->click.connect(this,&ScrollBar::onIncBtnClick);
    incBtn->setFlat(false);
    incBtn->setRepeatMode(true);
    decBtn = new Button(dec,12,12);
    decBtn->click.connect(this,&ScrollBar::onDecBtnClick);
    decBtn->setFlat(false);
    decBtn->setRepeatMode(true);

    add(incBtn);
    add(decBtn);

    incBtn->setSkin( 
      App::config()->skin( "scb_btn_up" ),
      App::config()->skin( "scb_btn_down" ),
      App::config()->skin( "scb_btn_over" ),
      App::config()->skin( "scb_btn_flat" )	
      );

    decBtn->setSkin( 
      App::config()->skin( "scb_btn_up" ),
      App::config()->skin( "scb_btn_down" ),
      App::config()->skin( "scb_btn_over" ),
      App::config()->skin( "scb_btn_flat" )	
      );

    sliderArea_ = new Panel();
    sliderArea_->skin_ = App::config()->skin("sbar_pane");
    sliderArea_->mousePress.connect(this,&ScrollBar::onScrollAreaClick);
    sliderArea_->setTransparent( false );
    add(sliderArea_);

    slider_ = new Button();
    slider_->setSkin(
      App::config()->skin("scb_btn_up_vsl"),
      App::config()->skin("scb_btn_down_vsl"),
      App::config()->skin("scb_btn_over_vsl"),
      App::config()->skin("scb_btn_up_vsl")
      );
    sliderArea_->add(slider_);

    slider_->setMoveable(Moveable(nMvVertical + nMvParentLimit));
    slider_->move.connect( this, &ScrollBar::onSliderMove );
  }

  void ScrollBar::resize( )
  {
    switch (orientation_) {
    case nVertical :
      decBtn->setPosition(0,0,clientWidth(),15);
      incBtn->setPosition(0,clientHeight()-15,clientWidth(),15);
      sliderArea_->setPosition(0,15,clientWidth(),clientHeight()-30);
      slider_->setWidth(sliderArea_->clientWidth());
      slider_->setMoveable(Moveable(nMvVertical + nMvParentLimit));
      break;
    case nHorizontal :
      decBtn->setPosition(0,0,15,clientHeight());
      incBtn->setPosition(clientWidth()-15,0,15,clientHeight());
      sliderArea_->setPosition(15,0,clientWidth()-30,clientHeight());
      slider_->setHeight(sliderArea_->clientHeight());
      slider_->setMoveable(Moveable(nMvHorizontal + nMvParentLimit));
      break;
    default:;
    }
  }

  void ScrollBar::setOrientation( int orientation )
  {
    orientation_ = orientation;
    if (orientation_==nHorizontal) {
      slider_->setWidth(20);
      slider_->setSkin(
        App::config()->skin("scb_btn_up_hsl"),
        App::config()->skin("scb_btn_down_hsl"),
        App::config()->skin("scb_btn_over_hsl"),
        App::config()->skin("scb_btn_up_hsl")
        );
      scrollPolicy_ = nDx;
      inc->setBitmap(right);
      dec->setBitmap(left);
    }
    else
    {
      slider_->setHeight(20);
      slider_->setSkin(
        App::config()->skin("scb_btn_up_vsl"),
        App::config()->skin("scb_btn_down_vsl"),
        App::config()->skin("scb_btn_over_vsl"),
        App::config()->skin("scb_btn_up_vsl")
        );
      scrollPolicy_ = nDy;
    }
    resize();
  }

  int ScrollBar::orientation( ) const
  {
    return orientation_;
  }

  void ScrollBar::setControl( VisualComponent * control , int scrollPolicy)
  {
    control_ = control;
    scrollPolicy_ = scrollPolicy;
  }

  void ScrollBar::onSliderMove( const MoveEvent & ev )
  {
    double range_ = max_ - min_;

    if ( control_ ) {
      switch ( scrollPolicy_ ) {
      case nDy :
        range_ = control_->clientHeight() - control_->spacingHeight();
        break;
      case nDx :
        range_ = control_->clientWidth() - control_->spacingWidth();
        break;
      }
    }

    switch (orientation_) {
   case nVertical :
     pos_ = ( range_ / (sliderArea_->clientHeight()- slider_->height()) ) * slider_->top();
     break;
   default :
     pos_ = ( range_ / (sliderArea_->clientWidth() - slider_->width())  ) * slider_->left();
    }

    updateControl();
    change.emit( this );
    scroll.emit( this );
  }

  void ScrollBar::scrollComponent( VisualComponent * comp , int dx, int dy )
  {
    if (comp==0) return;

    int compLeft   = comp->absoluteSpacingLeft();
    int compTop    = comp->absoluteSpacingTop();
    int compWidth  = comp->spacingWidth();
    int compHeight = comp->spacingHeight();

    if (dy<0) dy = 0; 

    Window* win = comp->window();
    Graphics& g = win->graphics();

    int diffY = (dy - comp->scrollDy());
    if (diffY!=0) {
      comp->setScrollDy(dy);
      if ( abs(diffY) > compHeight) { comp->repaint(); } else {
        g.setRegion(Rect(compLeft,compTop, compWidth, compHeight));
        g.setClipping(g.region());
        if (diffY > 0) {
          g.copyArea(compLeft  , compTop    + diffY, // src_x, sry_y
            compWidth , compHeight - diffY, // width, height
            compLeft  , compTop             // destX, destY
            );
          g.swap(Rect(compLeft,compTop, compWidth, compHeight - diffY));
          win->repaint(comp,Rect(compLeft,compTop + compHeight - diffY,compWidth,diffY));
        } else {
          g.copyArea(compLeft  , compTop, // src_x, sry_y
            compWidth , compHeight + diffY, // width, height
            compLeft  , compTop    - diffY             // destX, destY
            );
          g.swap(Rect(compLeft,compTop - diffY, compWidth, compHeight + diffY));
          win->repaint(comp,Rect(compLeft,compTop,compWidth,-diffY));
        }
      }
    }

    if (dx<0) dx = 0;

    int diffX = (dx - comp->scrollDx());
    if (diffX !=0) {
      comp->setScrollDx(dx);
      if ( abs(diffX) > compWidth) { comp->repaint(); } else {
        g.setRegion(Rect(compLeft,compTop, compWidth, compHeight));
        g.setClipping(g.region());
        if (diffX > 0) {
          g.copyArea(compLeft  + diffX, compTop, // src_x, sry_y
            compWidth - diffX, compHeight, // width, height
            compLeft  , compTop             // destX, destY
            );
          g.swap(Rect(compLeft,compTop, compWidth - diffX, compHeight));
          win->repaint(comp,Rect(compLeft + compWidth - diffX,compTop,diffX,compHeight));
        } else {
          g.copyArea(compLeft  , compTop, // src_x, sry_y
            compWidth + diffX , compHeight, // width, height
            compLeft - diffX  , compTop            // destX, destY
            );
          g.swap(Rect(compLeft - diffX,compTop, compWidth + diffX, compHeight));
          win->repaint(comp,Rect(compLeft,compTop,-diffX,compHeight));
        }
      }
    }
  }

  void ScrollBar::onDecBtnClick( ButtonEvent * ev )
  {
    updateControlRange();
    setPos( std::max( min_ , pos_ - smallChange_ ) );
    scroll.emit( this );
  }

  void ScrollBar::onIncBtnClick( ButtonEvent * ev )
  {
    updateControlRange();
    setPos ( std::min( max_, pos_ + smallChange_) );
    scroll.emit( this );
  }

  void ScrollBar::onScrollAreaClick( ButtonEvent * ev )
  {
    if (ev->button() == 1) {
      if ( orientation() == nHorizontal ) {
        if (ev->x() > slider_->left() + slider_->width() ) {
          // mouse was pressed right to slider so increment position
          updateControlRange();
          setPos( std::min( max_ , pos_ + largeChange_ ) );
          scroll.emit( this );
        } else {
          // mouse was pressed left to slider so decrement position
          updateControlRange();
          setPos( std::max( min_ , pos_ - largeChange_ ) );
          scroll.emit( this );
        }
      } else
      {
        if (ev->y() > slider_->top() + slider_->height() ) {
          // mouse was pressed under slider so increment position
          updateControlRange();
          setPos( std::min( max_ , pos_ + largeChange_ ) );
        } else {
          // mouse was pressed above slider so decrement position
          updateControlRange();
          setPos( std::max( min_ , pos_ - largeChange_ ) );
          scroll.emit( this );
        }
      }
    }
  }

  void ScrollBar::setLargeChange( double step )
  {
    largeChange_ = step;
  }

  double ScrollBar::largeChange( ) const
  {
    return largeChange_;
  }

  void ScrollBar::setSmallChange( double step )
  {
    smallChange_ = step;
  }

  double ScrollBar::smallChange( ) const
  {
    return smallChange_;
  }

  void ScrollBar::setPos( double pos )
  {
    pos_ = pos;
    updateSlider(); 
    sliderArea_->repaint();
    updateControl();

    change.emit(this );  
  }

  double ScrollBar::pos() const {
    return pos_;
  }

  void ScrollBar::setRange( double min, double max )
  {
    min_ = min;
    max_ = max;
  }

  double ScrollBar::range( ) const
  {
    return max_ - min_;
  }

  void ScrollBar::updateSlider( )
  {
    double range_ = max_ - min_;

    if ( control_ ) {
      switch ( scrollPolicy_ ) {
      case nDy :
        range_ = control_->clientHeight() - control_->spacingHeight();
        break;
      case nDx :
        range_ = control_->clientWidth() - control_->spacingWidth();
        break;
      }
    }

    if ( range_ == 0 ) return;

    if ( orientation_ == nVertical )
      slider_->setTop(  (int) (pos_  / ((range_ / (sliderArea_->clientHeight()- slider_->height()))) ));
    else 
      slider_->setLeft( (int) (pos_  / ((range_ / (sliderArea_->clientWidth() - slider_->width()))) ));

  }

  void ScrollBar::updateControl( )
  {
    if ( control_ ) {
      switch ( scrollPolicy_ ) {
      case nDy :
        scrollComponent(control_,control_->scrollDx(), static_cast<int>(pos_) );    
        break;
      case nDx :
        scrollComponent(control_, static_cast<int>(pos_), control_->scrollDy());
        break;
      }
    }  
  }

  void ScrollBar::updateControlRange( )
  {
    // maybe this can be better done through a resize signal from the scrollcomponent
    // but needs first check if a deadlock can occur
    if ( control_ ) {
      switch ( scrollPolicy_ ) {
      case nDy :
        min_ = 0;
        max_ = control_->clientHeight() - control_->spacingHeight();
        break;
      case nDx :
        min_ = 0;
        max_ = control_->clientWidth() - control_->spacingWidth();
        break;
      }
    }
  }

}


// class factories

extern "C" ngrs::Object* createScrollBar() {
  return new ngrs::ScrollBar();
}

extern "C" void destroyScrollBar( ngrs::Object* p ) {
  delete p;
}
