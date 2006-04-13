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
#include "nedit.h"
#include "napp.h"
#include "nconfig.h"


using namespace std;


NEdit::NEdit()
 : NPanel(), valign_(nAlLeft),halign_(nAlCenter),dx(0),pos_(0), selStartIdx_(0), selEndIdx_(0)
{
  init();
}


NEdit::~NEdit()
{
}

void NEdit::init( )
{
  metrics.setFont(font());
  NApp::config()->setSkin(&skin_,"edit");

  valign_ = nAlCenter;
  halign_ = nAlLeft;
}


void NEdit::setText( const std::string & text )
{
  text_ = text;
  pos_ = 0;
}

const std::string & NEdit::text( ) const
{
  return text_;
}

void NEdit::paint( NGraphics * g )
{
  dx = computeDx(g,text_);

  NPoint screenPos = getScreenPos(g, text_);

  g->drawText( screenPos.x() , screenPos.y() ,text_);

  if (focus()) drawCursor(g,text_);
}

void NEdit::drawCursor(NGraphics* g, const std::string & text )
{
  NColor oldColor = foreground();
  g->setForeground(font().textColor());

  NPoint screenPos = getScreenPos(g, text );
  int w = g->textWidth(text.substr(0,pos()));
  g->drawLine(screenPos.x()+w,screenPos.y(),screenPos.x()+w,screenPos.y() - g->textAscent());
  g->setForeground(oldColor);
}

int NEdit::computeDx( NGraphics* g, const std::string & text )
{
  int dx = 0;

  NPoint screenPos = getScreenPos(g, text );

  int w  = g->textWidth(text.substr(0,pos()));
  int wa = g->textWidth(text);

  switch (halign_) {
      case nAlLeft:
          if (screenPos.x()+w > spacingWidth()) dx = w - spacingWidth();
          if (screenPos.x()+w < 0 ) dx = w;
      break;
      case nAlRight:
          if (screenPos.x()+w < 0) dx = spacingWidth()-wa+w; else
          if (screenPos.x()+w > spacingWidth()) dx = w-wa;
      break;
      case nAlCenter:
          if (screenPos.x()+w > spacingWidth()) dx =   ((int) d2i((spacingWidth() - wa ) / 2.0) + w - spacingWidth());
          if (screenPos.x()+w < 0) dx = ((int) d2i((spacingWidth() - wa ) / 2.0) + w );
      break;
  }

  return dx;
}



int NEdit::preferredHeight( ) const
{
  NFontMetrics metrics;
  metrics.setFont(font());
  return metrics.textHeight();// + spacing().top()+spacing().bottom()+borderTop()+borderBottom();
}


int NEdit::preferredWidth( ) const
{ 
  NFontMetrics metrics;
  metrics.setFont(font());
  return width();
}


NPoint NEdit::getScreenPos(NGraphics* g, const std::string & text )
{
  int xp=0;
  int yp=0;

  int h = g->textAscent()+g->textDescent();

  switch (valign_) {
    case nAlCenter : yp = d2i( (spacingHeight()+g->textAscent()) / 2); break;
    case nAlTop    : yp = h;                  break;
    case nAlBottom : yp = (int) spacingHeight()- g->textDescent();         break;
    default        : yp = h;                  break;
  }

  int w = g->textWidth(text.c_str());
  switch (halign_) {
    case nAlCenter : xp = d2i((spacingWidth() - w ) / 2.0) - dx;  break;
    case nAlLeft   : xp = -dx;                  break;
    case nAlRight  : xp = spacingWidth() - w -dx;          break;
    default        : xp = 0;                  break;
  }

  return NPoint(xp,yp);
}

void NEdit::setVAlign( int align )
{
  valign_ = align;
}

void NEdit::setHAlign( int align )
{
  halign_ = align;
}

int NEdit::vAlign( ) const
{
  return valign_;
}

int NEdit::hAlign( ) const
{
  return halign_;
}

unsigned int NEdit::pos( ) const
{
  return pos_;
}


void NEdit::onKeyPress( const NKeyEvent & keyEvent )
{
 int keyCode = keyEvent.scancode();
 switch (keyCode) {
    case XK_Left:
                  if ( pos_>0) {
                    pos_--;
                    if (NApp::system().keyState() & ShiftMask) {
                       if (selStartIdx_==selEndIdx_) {
                          selStartIdx_ = pos_;
                          selEndIdx_ = pos_+1;
                       } else 
                       if (selStartIdx_ < selEndIdx_) {
                          selStartIdx_ = pos_;
                       }
                    } else selStartIdx_ = selEndIdx_ = pos_;
                  }
                  repaint();
                break;
    case XK_Right:
                  if (pos_< text_.length()) {
                    pos_++;
                      if (NApp::system().keyState() & ShiftMask) {
                        if (selStartIdx_==selEndIdx_) {
                          selEndIdx_ = pos_;
                          selStartIdx_ = pos_-1;
                       } else
                       if (selStartIdx_ < selEndIdx_) {
                          selEndIdx_ = pos_;
                       }
                    } else
                        selStartIdx_ = selEndIdx_ = pos_;
                    repaint();
                  }
                 break;
    case XK_BackSpace:
                 if (pos_>0) {
                    if (selStartIdx_!=selEndIdx_) {
                       int s = min(selStartIdx_,selEndIdx_);
                       int e = max(selStartIdx_,selEndIdx_);
                       text_.erase(s,e-s);
                       pos_=s;
                       selStartIdx_=selEndIdx_=0;
                       repaint();
                    } else {
/*                      bool flag = true;
                      if (pattern_!=NULL) {
                         string test = text_;
                         test.erase(pos_-1,1);
                         flag = pattern_->accept(test.c_str(),test.length()); 
                      }*/
                      //if (flag) {
                        pos_--;
                        text_.erase(pos_,1);
                        //doAutoTextWidth();
                        //textChanged.emit(this);
                        repaint();
                      //}
                    }
                 }
                 break;
    case XK_Home:
                 if (pos_>0) {
                    if (NApp::system().keyState() & ShiftMask) {
                      selEndIdx_   = pos_;
		      selStartIdx_ = 0;
                    }
                     pos_=0;
                    dx=0;
                    repaint();
                 }
                break;
    case XK_End:
                if (pos_<text_.length()) {
                   pos_=text_.length();
                   repaint();
                }
                break;
    case XK_Delete:
                if (pos_<text_.length()) {
                   //bool flag = true;
                   //if (pattern_!=NULL) pattern_->accept(text_.c_str(),text_.length());
                   //if (flag) {
                    int count = 1;
                    if (selStartIdx_!=selEndIdx_) {
                       count = abs(selEndIdx_ - selStartIdx_);
                       pos_ = min(selStartIdx_,selEndIdx_);
                       selStartIdx_ =selEndIdx_ = 0;
                    //}
                    text_.erase(pos_,count);
                    //doAutoTextWidth();
                    //textChanged.emit(this);
                    repaint();
                   }
                }
                break;
    default:
      if (keyEvent.buffer()!="") {
        text_.insert(pos_,keyEvent.buffer());
        pos_++;
        //doAutoTextWidth();
        repaint();
      }
 }
 //emitActions();
}

void NEdit::onFocus( )
{
  repaint();
}
