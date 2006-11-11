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
 : NPanel(), autoSize_(0),readOnly_(0),valign_(nAlCenter),halign_(nAlLeft),dx(0),pos_(0), selStartIdx_(0), selEndIdx_(0)
{
  init();
}

NEdit::NEdit( const std::string & text ) : NPanel(), autoSize_(0), readOnly_(0), valign_(nAlCenter),halign_(nAlLeft),dx(0),pos_(0), selStartIdx_(0), selEndIdx_(0), text_(text)
{
  init();
}

NEdit::~NEdit()
{
}

// the class factories

extern "C" NObject* createEdit() {
    return new NEdit();
}

extern "C" void destroyEdit(NObject* p) {
    delete p;
}


void NEdit::init( )
{
  metrics.setFont(font());
  skin_ = NApp::config()->skin("edit");

  valign_ = nAlCenter;
  halign_ = nAlLeft;

  setCursor ( nCrIBeam );
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

	if ( selEndIdx_ != selStartIdx_ ) {
		std::string startText = text_.substr(0, selStartIdx_ );
		std::string midText   = text_.substr(selStartIdx_, selEndIdx_-selStartIdx_);
		std::string endText   = text_.substr(selEndIdx_);

		int xoff = screenPos.x();
		g->drawText(  xoff , screenPos.y() , startText );
		xoff+= g->textWidth( startText );
    NColor oldColor = foreground();
		g->setForeground( NColor(0,0,255));
		g->fillRect( xoff, 0, g->textWidth( midText ), clientHeight() );
		setForeground( oldColor );
		g->drawText(  xoff , screenPos.y() , midText );
		xoff+= g->textWidth( midText );
		g->drawText(  xoff , screenPos.y() , endText );

	} else {
  	g->drawText( screenPos.x() , screenPos.y() ,text_);
	}

  if (focus() && !readOnly_) drawCursor(g,text_);
}

void NEdit::drawCursor(NGraphics* g, const std::string & text )
{
  NColor oldColor = foreground();
  g->setForeground(font().textColor());

  NPoint screenPos = getScreenPos(g, text );
  int w = g->textWidth(text.substr(0,pos()));
  g->drawLine(screenPos.x()+w,screenPos.y() + g->textDescent(),screenPos.x()+w,screenPos.y() - g->textAscent());
  g->setForeground(oldColor);
}

int NEdit::computeDx( NGraphics* g, const std::string & text )
{
  if (autoSize_) return 0;

  dx = 0;

  NPoint screenPos = getScreenPos(g, text );

	int w = 0;
	int wa = g->textWidth(text);

	if ( pos() == text.length() )
		w  = wa + g->textWidth("A");
	else
    w = g->textWidth(text.substr(0,pos()+1));

  switch (halign_) {
      case nAlLeft:
          if (screenPos.x()+w >= spacingWidth()) {
						dx = w - spacingWidth(); 
					}
						else
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
  if (ownerSize()) return NPanel::preferredHeight();

  NFontMetrics metrics( font() );
  metrics.setFont(font());
  return metrics.textHeight() + spacing().top()+spacing().bottom()+borderTop()+borderBottom();
}


int NEdit::preferredWidth( ) const
{
  if (ownerSize()) return NPanel::preferredWidth();

  NFontMetrics metrics;
  metrics.setFont(font());
  return metrics.textWidth(text_) + spacing().left()+spacing().right()+borderLeft()+borderRight();
}


NPoint NEdit::getScreenPos(NGraphics* g, const std::string & text )
{
  int xp=0;
  int yp=0;

  int h = g->textAscent()+g->textDescent();

  switch (valign_) {
    case nAlCenter : yp = (clientHeight() - g->textHeight()) / 2  + g->textAscent(); break;
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

void NEdit::setPos( unsigned int pos )
{
  if (pos >= 0 && pos < text_.length() ) pos_ = pos;
}

unsigned int NEdit::pos( ) const
{
  return pos_;
}


void NEdit::onKeyPress( const NKeyEvent & keyEvent )
{
 if (!readOnly_) {
 int keyCode = keyEvent.scancode();
 switch (keyCode) {
    case NK_Left:
                  if ( pos_>0) {
                    pos_--;
                    #ifdef __unix__
                    if (NApp::system().keyState() & ShiftMask) {
                       if (selStartIdx_==selEndIdx_) {
                          selStartIdx_ = pos_;
                          selEndIdx_ = pos_+1;
                       } else 
                       if (selStartIdx_ < selEndIdx_) {
                          selStartIdx_ = pos_;
                       }
                    } else selStartIdx_ = selEndIdx_ = pos_;
                    #endif
                  }
                  repaint();
                break;
    case NK_Right:
                  if (pos_< text_.length()) {
                    pos_++;
                    #ifdef __unix__
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
                    #endif    
                    repaint();
                  }
                 break;
    case NK_BackSpace:
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
                        //textChanged.emit(this);
                        repaint();
                      //}
                    }
                 }
                 break;
    case NK_Home:
                 if (pos_>0) {
                    #ifdef __unix__
                    if (NApp::system().keyState() & ShiftMask) {
                      selEndIdx_   = pos_;
                      selStartIdx_ = 0;
                    }
                    #endif
                     pos_=0;
                    dx=0;
                    repaint();
                 }
                break;
    case NK_End:
                if (pos_<text_.length()) {
                   pos_=text_.length();
                   repaint();
                }
                break;
    case NK_Return:

    break;
    case NK_Delete:
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
                    //textChanged.emit(this);
                    repaint();
                   }
                }
                break;
    default:
      if (keyEvent.buffer()!="") {
        text_.insert(pos_,keyEvent.buffer());
        pos_++;
        repaint();
      }
 }
 //emitActions();
  keyPress.emit(keyEvent);
 }
}

void NEdit::onExit()
{
  repaint();
}

void NEdit::onEnter( )
{
  repaint();
}

void NEdit::setAutoSize( bool on )
{
  autoSize_ = on;
}

void NEdit::setReadOnly( bool on )
{
  readOnly_ = on;
}

void NEdit::onMousePress( int x, int y, int button ) {
  NPanel::onMousePress( x, y, button );  

	NFntString myText;
  myText.setText( text_ );
  int newPos = findWidthMax( x + dx, myText );
  pos_ = newPos;
  repaint();
}

int NEdit::findWidthMax(long width, const NFntString & data ) const
{
  NFontMetrics metrics(font());
  int Low = 0; int High = data.length();  int Mid=High;
  while( Low <= High ) {
    Mid = ( Low + High ) / 2;
    NFntString s     = data.substr(0,Mid);
    NFntString snext;
    if (Mid>0) snext  = data.substr(0,Mid+1); else snext = s;
    int w     = metrics.textWidth(s);
    if(  w < width  ) {
                        int wnext = metrics.textWidth(snext);
                        if (wnext  >= width ) break;
                        Low = Mid + 1; 
                      } else
                      {
                        High = Mid - 1;
                      }
  }  
  return Mid;

}

std::string NEdit::selText() const {
  std::string midText   = text_.substr(selStartIdx_, selEndIdx_-selStartIdx_);
  return midText;
}

void NEdit::setInputPolicy( const std::string & regexp )
{

}



