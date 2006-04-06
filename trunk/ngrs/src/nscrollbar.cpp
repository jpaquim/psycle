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
#include "nscrollbar.h"
#include "nwindow.h"
#include "ngradient.h"
#include "napp.h"
#include "nconfig.h"


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


NScrollBar::~NScrollBar()
{
}

void NScrollBar::init( )
{
  inc    = new NImage();
  dec    = new NImage();

  up.createFromXpmData(arrow_up_xpm);
  down.createFromXpmData(arrow_down_xpm);
  left.createFromXpmData(arrow_left_xpm);
  right.createFromXpmData(arrow_right_xpm);

  inc->setBitmap(down);
  dec->setBitmap(up);

  range_ = 0;
  control_ = 0;
  scrollPolicy_ = nDy;

  orientation_ = nVertical;

  setBorder(0);
  incBtn = new NButton(inc,12,12);//up,10,10);
  incBtn->setFlat(false);
  decBtn = new NButton(dec,12,12);
  decBtn->setFlat(false);

  add(incBtn);
  add(decBtn);

  setTransparent(true);

  sliderArea_ = new NPanel();
    NApp::config()->setSkin(&sliderArea_->skin_,"sbar_pane");
  add(sliderArea_);

  NApp::config()->setSkin(&vSlSkin,"sbar_vsl");
  NApp::config()->setSkin(&hSlSkin,"sbar_hsl");

  slider_ = new Slider(this);
     slider_->setSkin(vSlSkin);
  sliderArea_->add(slider_);

  slider_->setMoveable(NMoveable(nMvVertical + nMvParentLimit + nMvRepaint));
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
     sliderArea_->setGradientOrientation(nVertical);
     slider_->setWidth(20);
     slider_->setSkin(hSlSkin);
     scrollPolicy_ = nDx;
     inc->setBitmap(right);
     dec->setBitmap(left);
  }
  else
   {
     slider_->setHeight(20);
     slider_->setSkin(vSlSkin);
     sliderArea_->setGradientOrientation(nHorizontal);
     scrollPolicy_ = nDy;
   }
  resize();
}

int NScrollBar::orientation( )
{
  return orientation_;
}

void NScrollBar::setControl( NVisualComponent * control , int scrollPolicy)
{
  control_ = control;
  scrollPolicy_ = scrollPolicy;
}

void NScrollBar::onSliderMove( )
{
  switch ( scrollPolicy_) {
      case nDy : {
         int dy = slider_->top();
         if (control_!=0) {
           dy = (int)( (( control_->clientHeight() - control_->spacingHeight()) / ((double) sliderArea_->spacingHeight() - slider_->height())) * slider_->top());
           scrollComponent(control_,control_->scrollDx(),dy);
         }
         dy = (int) d2i(((range_ /((double) (sliderArea_->clientHeight()-slider_->height()))) * dy));
         posChange.emit(this,dy);
      }
      break;
      case nDx : {
         int dx = slider_->left();
         if (control_!=0) {
           dx = (int)( (( control_->clientWidth() - control_->spacingWidth()) / ((double) sliderArea_->clientWidth() - slider_->width())) * slider_->left());
           scrollComponent(control_,dx,control_->scrollDy());
         }
         posChange.emit(this,(int) (range_ /((double) (sliderArea_->clientWidth()-slider_->width())) * dx));
      }
      break;
      default:;
    }

  /*if (control_!=0) {
    switch ( scrollPolicy_) {
      case nDy : {
        //control_->setScrollDy(0);
        control_->setScrollDx(dy);//slider_->top());
        control_->repaint();
       }
      break;
      case nDx :
        control_->setScrollDx(slider_->left());
        control_->repaint();
      break;
      default:;
    }
  }*/
}

NScrollBar::Slider::Slider( NScrollBar * sl )
{
  sl_ = sl;
}

NScrollBar::Slider::~ Slider( )
{
}

void NScrollBar::Slider::onMove( const NMoveEvent & moveEvent )
{
  sl_->onSliderMove();
}

void NScrollBar::scrollComponent( NVisualComponent * comp , int dx, int dy )
{
   if (comp==0) return;

   int compLeft   = comp->absoluteSpacingLeft();
   int compTop    = comp->absoluteSpacingTop();
   int compWidth  = comp->spacingWidth();
   int compHeight = comp->spacingHeight();

   if (dy<0) dy = 0;

   NRect repaintArea();

   NWindow* win = comp->window();
   NGraphics* g = win->graphics();

  int diffY = (dy - comp->scrollDy());
  if (diffY!=0) {
    comp->setScrollDy(dy);
    if ( abs(diffY) > compHeight) { comp->repaint(); } else {
    g->setRectRegion(NRect(compLeft,compTop, compWidth, compHeight));
    g->setRegion(g->region(),true);
    if (diffY > 0) {
       g->copyArea(compLeft  , compTop    + diffY, // src_x, sry_y
                 compWidth , compHeight - diffY, // width, height
                 compLeft  , compTop             // destX, destY
       );
       g->swap(NRect(compLeft,compTop, compWidth, compHeight - diffY));
       win->repaint(compLeft,compTop + compHeight - diffY,compWidth,diffY);
     } else {
       g->copyArea(compLeft  , compTop, // src_x, sry_y
                   compWidth , compHeight + diffY, // width, height
                   compLeft  , compTop    - diffY             // destX, destY
       );
       g->swap(NRect(compLeft,compTop - diffY, compWidth, compHeight + diffY));
       win->repaint(compLeft,compTop,compWidth,-diffY);
       }
     }
   }

   int diffX = (dx - comp->scrollDx());
   if (diffX !=0) {
    comp->setScrollDx(dx);
    if ( abs(diffX) > compWidth) { comp->repaint(); } else {
    g->setRectRegion(NRect(compLeft,compTop, compWidth, compHeight));
    g->setRegion(g->region(),true);
    if (diffX > 0) {
       g->copyArea(compLeft  + diffX, compTop, // src_x, sry_y
                 compWidth - diffX, compHeight, // width, height
                 compLeft  , compTop             // destX, destY
       );
       g->swap(NRect(compLeft,compTop, compWidth - diffX, compHeight));
       win->repaint(compLeft + compWidth - diffX,compTop,diffX,compHeight);
     } else {
       g->copyArea(compLeft  , compTop, // src_x, sry_y
                   compWidth + diffX , compHeight, // width, height
                   compLeft - diffX  , compTop            // destX, destY
       );
       g->swap(NRect(compLeft - diffX,compTop, compWidth + diffX, compHeight));
       win->repaint(compLeft,compTop,-diffX,compHeight);
       }
     }
   }
}

void NScrollBar::setRange( int range )
{
  range_ = range;
}

void NScrollBar::Slider::onMousePress( int x, int y, int button )
{
  /*unsigned int mask = ExposureMask | KeyPressMask | KeyReleaseMask  |
                    ButtonPressMask|ButtonReleaseMask | PointerMotionHintMask
                    |PropertyChangeMask | NoEventMask | StructureNotifyMask | SubstructureRedirectMask | GravityNotify;
  XSetWindowAttributes attr;
  attr.event_mask = mask;
  XChangeWindowAttributes(NApp::system().dpy(), window()->win(), CWEventMask, &attr);*/
}

void NScrollBar::Slider::onMousePressed( int x, int y, int button )
{
/*  XSetWindowAttributes attr;
  attr.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | ButtonMotionMask |
                    ButtonPressMask|ButtonReleaseMask | PointerMotionMask
                    |PropertyChangeMask | NoEventMask | StructureNotifyMask | SubstructureRedirectMask | GravityNotify;
  XChangeWindowAttributes(NApp::system().dpy(), window()->win(), CWEventMask, &attr);
  XSync(NApp::system().dpy(),true);*/
}

void NScrollBar::setPos( int value )
{
  switch ( scrollPolicy_) {
      case nDy : {
         int sliderTop =(int)( (value * (sliderArea_->clientHeight()-slider_->height())) / (double) range_ );
         slider_->setTop(sliderTop);
         sliderArea_->repaint();
         onSliderMove();
      }
      break;
      case nDx : {
         int sliderLeft =(int)( (value * (sliderArea_->clientWidth()-slider_->width())) / (double) range_ );
         slider_->setLeft(sliderLeft);
         sliderArea_->repaint();
         onSliderMove();
      }
      break;
  }
}





