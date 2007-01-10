/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by Stefan Nattkemper                         *
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
#include "nscrollbar.h"
#include "nwindow.h"
#include "ngradient.h"
#include "napp.h"
#include "nconfig.h"

#ifdef _MSC_VER
#undef min 
#undef max
#endif


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
"            "};

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
"            "};

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
"            "};

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
"            "};



NScrollBar::NScrollBar()
 : NPanel()
{
  init();
}

NScrollBar::NScrollBar( int orientation )
{
  init();
  setOrientation(orientation);
}

NScrollBar::~NScrollBar()
{
}

void NScrollBar::init( )
{
  enableFocus( false );

  inc    = new NImage();
  dec    = new NImage();

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

  incBtn = new NButton(inc,12,12);//up,10,10);
  incBtn->click.connect(this,&NScrollBar::onIncBtnClick);
  incBtn->setFlat(false);
  incBtn->setRepeatMode(true);
  decBtn = new NButton(dec,12,12);
  decBtn->click.connect(this,&NScrollBar::onDecBtnClick);
  decBtn->setFlat(false);
  decBtn->setRepeatMode(true);

  add(incBtn);
  add(decBtn);

  incBtn->setSkin( 
	  NApp::config()->skin( "scb_btn_up" ),
      NApp::config()->skin( "scb_btn_down" ),
	  NApp::config()->skin( "scb_btn_over" ),
	  NApp::config()->skin( "scb_btn_flat" )	
  );

  decBtn->setSkin( 
	  NApp::config()->skin( "scb_btn_up" ),
      NApp::config()->skin( "scb_btn_down" ),
	  NApp::config()->skin( "scb_btn_over" ),
	  NApp::config()->skin( "scb_btn_flat" )	
  );

  sliderArea_ = new NPanel();
    sliderArea_->skin_ = NApp::config()->skin("sbar_pane");
    sliderArea_->mousePress.connect(this,&NScrollBar::onScrollAreaClick);
	sliderArea_->setTransparent( false );
  add(sliderArea_);

  slider_ = new NButton();
  slider_->setSkin(
	NApp::config()->skin("scb_btn_up_vsl"),
	NApp::config()->skin("scb_btn_down_vsl"),
	NApp::config()->skin("scb_btn_over_vsl"),
	NApp::config()->skin("scb_btn_up_vsl")
  );
  sliderArea_->add(slider_);

  slider_->setMoveable(NMoveable(nMvVertical + nMvParentLimit));
  slider_->move.connect( this, &NScrollBar::onSliderMove );
}

// class factories

extern "C" NObject* createScrollBar() {
    return new NScrollBar();
}

extern "C" void destroyScrollBar(NObject* p) {
    delete p;
}


void NScrollBar::resize( )
{
  switch (orientation_) {
    case nVertical :
       decBtn->setPosition(0,0,clientWidth(),15);
       incBtn->setPosition(0,clientHeight()-15,clientWidth(),15);
       sliderArea_->setPosition(0,15,clientWidth(),clientHeight()-30);
       slider_->setWidth(sliderArea_->clientWidth());
       slider_->setMoveable(NMoveable(nMvVertical + nMvParentLimit));
    break;
     case nHorizontal :
       decBtn->setPosition(0,0,15,clientHeight());
       incBtn->setPosition(clientWidth()-15,0,15,clientHeight());
       sliderArea_->setPosition(15,0,clientWidth()-30,clientHeight());
       slider_->setHeight(sliderArea_->clientHeight());
       slider_->setMoveable(NMoveable(nMvHorizontal + nMvParentLimit));
    break;
    default:;
  }
}

void NScrollBar::setOrientation( int orientation )
{
  orientation_ = orientation;
  if (orientation_==nHorizontal) {
	 slider_->setWidth(20);
     slider_->setSkin(
		NApp::config()->skin("scb_btn_up_hsl"),
		NApp::config()->skin("scb_btn_down_hsl"),
		NApp::config()->skin("scb_btn_over_hsl"),
		NApp::config()->skin("scb_btn_up_hsl")
     );
     scrollPolicy_ = nDx;
     inc->setBitmap(right);
     dec->setBitmap(left);
  }
  else
   {
     slider_->setHeight(20);
	 slider_->setSkin(
		NApp::config()->skin("scb_btn_up_vsl"),
		NApp::config()->skin("scb_btn_down_vsl"),
		NApp::config()->skin("scb_btn_over_vsl"),
		NApp::config()->skin("scb_btn_up_vsl")
     );
     scrollPolicy_ = nDy;
   }
  resize();
}

int NScrollBar::orientation( ) const
{
  return orientation_;
}

void NScrollBar::setControl( NVisualComponent * control , int scrollPolicy)
{
  control_ = control;
  scrollPolicy_ = scrollPolicy;
}

void NScrollBar::onSliderMove( const NMoveEvent & ev )
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

void NScrollBar::scrollComponent( NVisualComponent * comp , int dx, int dy )
{
   if (comp==0) return;

   int compLeft   = comp->absoluteSpacingLeft();
   int compTop    = comp->absoluteSpacingTop();
   int compWidth  = comp->spacingWidth();
   int compHeight = comp->spacingHeight();

   if (dy<0) dy = 0; 

   NWindow* win = comp->window();
   NGraphics* g = win->graphics();

  int diffY = (dy - comp->scrollDy());
  if (diffY!=0) {
    comp->setScrollDy(dy);
    if ( abs(diffY) > compHeight) { comp->repaint(); } else {
    g->setRegion(NRect(compLeft,compTop, compWidth, compHeight));
    g->setClipping(g->region());
    if (diffY > 0) {
       g->copyArea(compLeft  , compTop    + diffY, // src_x, sry_y
                 compWidth , compHeight - diffY, // width, height
                 compLeft  , compTop             // destX, destY
       );
       g->swap(NRect(compLeft,compTop, compWidth, compHeight - diffY));
       win->repaint(comp,NRect(compLeft,compTop + compHeight - diffY,compWidth,diffY));
     } else {
       g->copyArea(compLeft  , compTop, // src_x, sry_y
                   compWidth , compHeight + diffY, // width, height
                   compLeft  , compTop    - diffY             // destX, destY
       );
       g->swap(NRect(compLeft,compTop - diffY, compWidth, compHeight + diffY));
       win->repaint(comp,NRect(compLeft,compTop,compWidth,-diffY));
       }
     }
   }

   if (dx<0) dx = 0;

   int diffX = (dx - comp->scrollDx());
   if (diffX !=0) {
    comp->setScrollDx(dx);
    if ( abs(diffX) > compWidth) { comp->repaint(); } else {
    g->setRegion(NRect(compLeft,compTop, compWidth, compHeight));
    g->setClipping(g->region());
    if (diffX > 0) {
       g->copyArea(compLeft  + diffX, compTop, // src_x, sry_y
                 compWidth - diffX, compHeight, // width, height
                 compLeft  , compTop             // destX, destY
       );
       g->swap(NRect(compLeft,compTop, compWidth - diffX, compHeight));
       win->repaint(comp,NRect(compLeft + compWidth - diffX,compTop,diffX,compHeight));
     } else {
       g->copyArea(compLeft  , compTop, // src_x, sry_y
                   compWidth + diffX , compHeight, // width, height
                   compLeft - diffX  , compTop            // destX, destY
       );
       g->swap(NRect(compLeft - diffX,compTop, compWidth + diffX, compHeight));
       win->repaint(comp,NRect(compLeft,compTop,-diffX,compHeight));
       }
     }
   }
}

void NScrollBar::onDecBtnClick( NButtonEvent * ev )
{
  updateControlRange();
  setPos( std::max( min_ , pos_ - smallChange_ ) );
  scroll.emit( this );
}

void NScrollBar::onIncBtnClick( NButtonEvent * ev )
{
  updateControlRange();
  setPos ( std::min( max_, pos_ + smallChange_) );
  scroll.emit( this );
}

void NScrollBar::onScrollAreaClick( NButtonEvent * ev )
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

void NScrollBar::setLargeChange( double step )
{
  largeChange_ = step;
}

double NScrollBar::largeChange( ) const
{
  return largeChange_;
}

void NScrollBar::setSmallChange( double step )
{
  smallChange_ = step;
}

double NScrollBar::smallChange( ) const
{
  return smallChange_;
}

void NScrollBar::setPos( double pos )
{
  pos_ = pos;
  updateSlider(); 
  sliderArea_->repaint();
  updateControl();

  change.emit(this );  
}

double NScrollBar::pos() const {
  return pos_;
}

void NScrollBar::setRange( double min, double max )
{
  min_ = min;
  max_ = max;
}

double NScrollBar::range( ) const
{
  return max_ - min_;
}

void NScrollBar::updateSlider( )
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

void NScrollBar::updateControl( )
{
  if ( control_ ) {
    switch ( scrollPolicy_ ) {
      case nDy :
        scrollComponent(control_,control_->scrollDx(), static_cast<int>(pos_) );    
        control_->repaint();
      break;
      case nDx :
        scrollComponent(control_, static_cast<int>(pos_), control_->scrollDy());
        control_->repaint();
      break;
    }
  }  
}

void NScrollBar::updateControlRange( )
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


