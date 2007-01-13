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
#include "nlabel.h"
#include "nproperty.h"
#include "nrectshape.h"

namespace ngrs {

  NLabel::NLabel()
    : NVisualComponent()
  {
    init();
  }

  NLabel::NLabel( const std::string & text )
  {
    init();
    text_ = text;
  }

  void NLabel::init( )
  {
    rectShape = new NRectShape();
    setGeometry( rectShape );
    geometry()->setPosition(0,0,10,10);
    metrics.setFont(font());
    mnemonic_ = '\0';
    halign_=nAlLeft;
    valign_=nAlTop;
    orientation_ = nHorizontal;
    setEvents(false);
    wbreak_ = false;

    // runtime
    //if (properties()) properties()->bind("text", *this, &NLabel::text, &NLabel::setText);
  }

  NLabel::~NLabel()
  {
    delete rectShape;
  }

  void NLabel::paint( NGraphics * g )
  {
    std::string::size_type i     = 0;
    std::string::size_type start = 0;

    int yp_ = g->textAscent() ;

    do {
      i = text_.find( "\n", i);
      std::string substr;
      if (i != -1) {
        substr = text_.substr(start,i-start);
        start = i+1;
        i+=1;
      } else substr = text_.substr(start);

      if (!wbreak_) {

        int xp_ = 0;

        switch (halign_) {
        case nAlCenter : xp_ = (clientWidth() - g->textWidth(substr)) / 2;
          break;
        default:
          ;
        }

        switch (valign_) {				
        case nAlCenter : yp_ = (clientHeight() - g->textHeight()) / 2  + g->textAscent();
          break;
        case nAlBottom : yp_ = clientHeight() - g->textDescent();
          break;
        default:
          ;
        }

        g->drawText( xp_, yp_, substr);

        if (mnemonic_!='\0') {
          std::string::size_type pos = substr.find( static_cast<char>( mnemonic_- 32) );
          if ( pos==std::string::npos ) pos =  substr.find( mnemonic_ );
          if ( pos!=std::string::npos ) {
            int w  = g->textWidth( substr.substr( 0, pos) );
            int w1 = g->textWidth( substr.substr( 0, pos+1) );
            g->drawLine( w, yp_+2, w1, yp_+2 );
          }
        }
        yp_ = yp_ + g->textHeight();
      } else {  // multiline wordbreak;
        int xp = 0;
        int yp = g->textAscent();

        std::string::size_type pos = 0;

        for (std::vector<std::string::size_type>::iterator it = breakPoints.begin(); it < breakPoints.end(); it++) {
          std::string::size_type lineEnd = *it;
          g->drawText( xp, yp, text_.substr( pos, lineEnd - pos) );
          yp+=g->textHeight();
          pos = lineEnd;
        }
        g->drawText(xp,yp,text_.substr(pos));
      }

    } while ( i != std::string::npos );
  }

  void NLabel::setText( const std::string & text )
  {
    text_ = text;
    if (orientation_ == nVertical) {
      // todo not implemented yet
    }
  }

  const std::string & NLabel::text( ) const
  {
    return text_;
  }


  int NLabel::preferredHeight( ) const
  {
    if (!wbreak_) {
      NFontMetrics metrics(font());
      std::string::size_type i = 0;
      int yp_ = metrics.textHeight() ;
      do {
        i = text_.find("\n", i);
        if ( i != -1 ) {
          i += 1;
          yp_ = yp_ + metrics.textHeight();
        }
      } while ( i != std::string::npos );
      return yp_ + spacing().top() + spacing().bottom() + borderTop() + borderBottom();
    } else {
      int yp =  0;
      std::string::size_type pos = 0;

      NFontMetrics metrics(font());

      for (std::vector<std::string::size_type>::const_iterator it = breakPoints.begin(); it < breakPoints.end(); it++) {
        std::string::size_type lineEnd = *it;
        yp+=metrics.textHeight();
      }
      yp += metrics.textHeight();
      return yp + spacing().top() + spacing().bottom() + borderTop() + borderBottom();
    }
  }

  int NLabel::preferredWidth( ) const
  {
    NFontMetrics metrics(font());
    std::string::size_type i = 0;
    std::string::size_type start = 0;

    std::string substr;
    int xmax = 0;
    do {
      i = text_.find("\n", i);
      if ( i != std::string::npos ) {
        i +=1 ;
        substr = text_.substr( start, i-start-1 );
        start = i;
      } else substr = text_.substr( start );
      if ( metrics.textWidth( substr ) > xmax) xmax = metrics.textWidth( substr );
    } while ( i != std::string::npos );

    return xmax + spacing().left() + spacing().right() + borderLeft() + borderRight();
  }

  void NLabel::setMnemonic( char c )
  {
    mnemonic_ = c;
  }

  char NLabel::mnemonic() const {
    return mnemonic_;
  }

  void NLabel::setVAlign( int align )
  {
    valign_ = align;
  }

  void NLabel::setHAlign( int align )
  {
    halign_ = align;
  }

  int NLabel::vAlign( ) const
  {
    return valign_;
  }

  int NLabel::hAlign( ) const
  {
    return halign_;
  }

  void NLabel::setTextOrientation( int orientation )
  {
    orientation_ = orientation;
  }

  void NLabel::computeBreakPoints( )
  {
    breakPoints.clear();  // clear old breakpoints

    std::string part = text_;

    std::string::size_type last = 0;
    std::string::size_type pos  = 0;
    while ( ( pos = findWidthMax(spacingWidth(),part,true)) < part.length() && (pos!=0) ) {
      part = part.substr(pos);
      last  += pos;
      breakPoints.push_back(last);
    }
  }

  int NLabel::findWidthMax(long width, const std::string & data, bool wbreak)
  {
    NFontMetrics metrics(font());

    int Low = 0; int High = data.length();  int Mid=High;
    while( Low <= High ) {
      Mid = ( Low + High ) / 2;
      std::string s     = data.substr(0,Mid);
      std::string snext;
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
    if (!wbreak || data.substr(0,Mid).find(" ") == std::string::npos || Mid == 0 || Mid>=data.length()) return Mid; else
    {
      std::string::size_type p = data.rfind( " ", Mid );
      if ( p != std::string::npos ) return p + 1;
    }
    return Mid;
  }

  void NLabel::resize( )
  {
    if (wbreak_) computeBreakPoints();
  }

  void NLabel::setWordWrap( bool on )
  {
    wbreak_ = on;
  }

  bool NLabel::wordWrap( ) const
  {
    return wbreak_;
  }

}

// the class factories
extern "C" ngrs::NObject* createLabel() {
  return new ngrs::NLabel();
}

extern "C" void destroyLabel( ngrs::NObject* p ) {
  delete p;
}
