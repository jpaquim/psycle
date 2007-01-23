/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by  Stefan Nattkemper   *
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
#include "edit.h"
#include "app.h"
#include "system.h"
#include "config.h"

#ifdef _MSC_VER
#undef min 
#undef max
#endif

namespace ngrs {

  Edit::Edit()
    : Panel(), autoSize_(0),readOnly_(0),valign_(nAlCenter),halign_(nAlLeft),dx(0),pos_(0), selStartIdx_(0), selEndIdx_(0)
  {
    init();
  }

  Edit::Edit( const std::string & text ) : Panel(), autoSize_(0), readOnly_(0), valign_(nAlCenter),halign_(nAlLeft),dx(0),pos_(0), selStartIdx_(0), selEndIdx_(0), text_(text)
  {
    init();
  }

  Edit::~Edit()
  {
  }

  void Edit::init( )
  {
    skin_ = App::config()->skin("edit");

    valign_ = nAlCenter;
    halign_ = nAlLeft;

    setCursor ( nCrIBeam );
  }


  void Edit::setText( const std::string & text )
  {
    text_ = text;
    pos_ = 0;
  }

  const std::string & Edit::text( ) const
  {
    return text_;
  }

  void Edit::paint( Graphics& g )
  {
    dx = computeDx(g,text_);

    Point screenPos = getScreenPos(g, text_);

    if ( selEndIdx_ != selStartIdx_ ) {
      std::string startText = text_.substr(0, selStartIdx_ );
      std::string midText   = text_.substr(selStartIdx_, selEndIdx_-selStartIdx_);
      std::string endText   = text_.substr(selEndIdx_);

      int xoff = screenPos.x();
      g.drawText(  xoff , screenPos.y() , startText );
      xoff+= g.textWidth( startText );
      Color oldColor = foreground();
      g.setForeground( Color(0,0,255));
      g.fillRect( xoff, 0, g.textWidth( midText ), clientHeight() );
      setForeground( oldColor );
      g.drawText(  xoff , screenPos.y() , midText );
      xoff+= g.textWidth( midText );
      g.drawText(  xoff , screenPos.y() , endText );

    } else {
      g.drawText( screenPos.x() , screenPos.y() ,text_);
    }

    if (focus() && !readOnly_) drawCursor(g,text_);
  }

  void Edit::drawCursor(Graphics& g, const std::string & text )
  {
    Color oldColor = foreground();
    g.setForeground(font().textColor());

    Point screenPos = getScreenPos(g, text );
    int w = g.textWidth(text.substr(0,pos()));
    g.drawLine(screenPos.x()+w,screenPos.y() + g.textDescent(),screenPos.x()+w,screenPos.y() - g.textAscent());
    g.setForeground(oldColor);
  }

  int Edit::computeDx( Graphics& g, const std::string & text )
  {
    if (autoSize_) return 0;

    dx = 0;

    Point screenPos = getScreenPos(g, text );

    int w = 0;
    int wa = g.textWidth(text);

    if ( pos() == text.length() )
      w  = wa + g.textWidth("A");
    else
      w = g.textWidth(text.substr(0,pos()+1));

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



  int Edit::preferredHeight( ) const
  {
    if (ownerSize()) return Panel::preferredHeight();

    FontMetrics metrics( font() );
    return metrics.textHeight() + spacing().top()+spacing().bottom()+borderTop()+borderBottom();
  }


  int Edit::preferredWidth( ) const
  {
    if (ownerSize()) return Panel::preferredWidth();

    FontMetrics metrics( font() );
    return metrics.textWidth(text_) + spacing().left()+spacing().right()+borderLeft()+borderRight();
  }


  Point Edit::getScreenPos(Graphics& g, const std::string & text )
  {
    int xp=0;
    int yp=0;

    int h = g.textAscent()+g.textDescent();

    switch (valign_) {
    case nAlCenter : yp = (clientHeight() - g.textHeight()) / 2  + g.textAscent(); break;
    case nAlTop    : yp = h;                  break;
    case nAlBottom : yp = (int) spacingHeight()- g.textDescent();         break;
    default        : yp = h;                  break;
    }

    int w = g.textWidth(text.c_str());
    switch (halign_) {
    case nAlCenter : xp = d2i((spacingWidth() - w ) / 2.0) - dx;  break;
    case nAlLeft   : xp = -dx;                  break;
    case nAlRight  : xp = spacingWidth() - w -dx;          break;
    default        : xp = 0;                  break;
    }

    return Point(xp,yp);
  }

  void Edit::setVAlign( int align )
  {
    valign_ = align;
  }

  void Edit::setHAlign( int align )
  {
    halign_ = align;
  }

  int Edit::vAlign( ) const
  {
    return valign_;
  }

  int Edit::hAlign( ) const
  {
    return halign_;
  }

  void Edit::setPos( unsigned int pos )
  {
    if (pos >= 0 && pos < text_.length() ) pos_ = pos;
  }

  unsigned int Edit::pos( ) const
  {
    return pos_;
  }


  void Edit::onKeyPress( const KeyEvent & keyEvent )
  {
    if (!readOnly_) {
      int keyCode = keyEvent.scancode();
      switch (keyCode) {
    case NK_Shift:
      if ( selStartIdx_ == selEndIdx_ ) startSel();   
      break;         
    case NK_Left:
      if ( pos_>0) {
        pos_--;
        if ( keyEvent.shift() & nsShift ) {
          computeSel();   
        }
        repaint();
      } 
      break;
    case NK_Right:
      if (pos_< text_.length()) {
        pos_++;
        if ( keyEvent.shift() & nsShift ) {
          computeSel();   
        }
        repaint();
      }
      break;
    case NK_BackSpace:
      if ( pos_ > 0 ) {
        if ( selStartIdx_ < selEndIdx_ ) {
          text_.erase( selStartIdx_, selEndIdx_ - selStartIdx_ );
          pos_ =  selStartIdx_;
          selStartIdx_ = 0;
          selEndIdx_   = 0;
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
        if ( keyEvent.shift() & nsShift ) {
          selEndIdx_   = pos_;
          selStartIdx_ = 0;
        }
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
      if ( pos_ < text_.length() ) {
        std::string::size_type count = 1;
        if ( selStartIdx_ < selEndIdx_ ) {
          count = selEndIdx_ - selStartIdx_;
          pos_  = selStartIdx_;
          selStartIdx_ = 0;
          selEndIdx_   = 0;
        }
        text_.erase( pos_, count );
        repaint();
      }
      break;
    default: {            
      if ( keyEvent.buffer() != "" ) {
        if ( selStartIdx_ != selEndIdx_ ) {
          std::string::size_type count = selEndIdx_ - selStartIdx_;
          pos_ = selStartIdx_;
          selStartIdx_ = 0;
          selEndIdx_   = 0;
          text_.erase( pos_, count ); 
        }     
        text_.insert(pos_,keyEvent.buffer());
        pos_++;
        repaint();
      }
             }  
      }
      //emitActions();
      keyPress.emit(keyEvent);
    }
  }

  void Edit::onExit()
  {
    repaint();
  }

  void Edit::onEnter( )
  {
    repaint();
  }

  void Edit::setAutoSize( bool on )
  {
    autoSize_ = on;
  }

  void Edit::setReadOnly( bool on )
  {
    readOnly_ = on;
  }

  void Edit::onMousePress( int x, int y, int button ) {
    Panel::onMousePress( x, y, button );  

    FntString myText;
    myText.setText( text_ );
    FontMetrics metrics( font( ) );
    pos_ = metrics.findWidthMax( x + dx, myText );

    if ( !( App::system().shiftState() & nsShift ) ) {
      startSel();       
    }     

    computeSel();
    repaint();
  }

  void Edit::onMouseOver( int x, int y ) {
    Panel::onMouseOver( x, y );
    int shiftState = App::system().shiftState();

    if ( shiftState & nsLeft ) {
      FntString myText;
      myText.setText( text_ );
      FontMetrics metrics( font( ) );
      pos_ = metrics.findWidthMax( x + dx, myText );   
      computeSel();
      repaint();
    }       
  }

  void Edit::onMousePressed( int x, int y, int button ) {
    Panel::onMousePressed( x, y, button ); 
    endSel();    
  }

  std::string Edit::selText() const {
    std::string midText   = text_.substr( selStartIdx_, selEndIdx_ - selStartIdx_ );
    return midText;
  }

  void Edit::setInputPolicy( const std::string & regexp )
  {
  }

  void Edit::startSel( ) {
    selStartPos_ = pos_;
  }

  void Edit::computeSel( ) {
    selStartIdx_ = std::min( pos_, selStartPos_ );
    selEndIdx_   = std::max( pos_, selStartPos_ );
  }

  void Edit::endSel() {   
  }     

}


// the class factories
extern "C" ngrs::Object* createEdit() {
  return new ngrs::Edit();
}

extern "C" void destroyEdit( ngrs::Object* p ) {
  delete p;
}
