/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by Stefan Nattkemper   *
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
#include "nmemo.h"
#include "nalignlayout.h"
#include "nscrollbar.h"
#include "nfontmetrics.h"
#include "nwindow.h"
#include <fstream>
#include <sstream>

#ifdef _MSC_VER
#undef min 
#undef max
#endif


NMemo::NMemo()
 : NTextBase()
{
  setLayout(NAlignLayout());

  hBar = new NScrollBar(nHorizontal);
    hBar->setHeight(15);
  add(hBar, nAlBottom);

  vBar = new NScrollBar(nVertical);
    vBar->setWidth(15);
  add(vBar, nAlRight);

  textArea = new TextArea(this);
    textArea->setClientSizePolicy(nVertical | nHorizontal);
  add(textArea, nAlClient);

  hBar->setControl(textArea, nDx);
  vBar->setControl(textArea, nDy);
}


NMemo::~NMemo()
{
}

// set and get the text content

std::string NMemo::text( ) const
{
  return textArea->text();
}

void NMemo::setText( const std::string & text )
{
  textArea->setText(text);
}

// the load save function

void NMemo::loadFromFile( const std::string & fileName )
{
  textArea->loadFromFile(fileName);
}

void NMemo::saveToFile( const std::string & fileName )
{
}

// The ability for the text to continue on the next line when the caret encounters the right border of the memo is controlled whose default value is set to true. If you do not want text to wrap to the subsequent line, set the  WordWrap property to false.

void NMemo::setWordWrap( bool on )
{
  textArea->setWordWrap(on);
  hBar->setVisible(!on);
  resize();
}

bool NMemo::wordWrap( ) const
{
  return textArea->wordWrap();
}

// clears the text content

void NMemo::clear( )
{
  textArea->clear();
}

// setting readOnly means to have a view

void NMemo::setReadOnly( bool on )
{
  textArea->setReadOnly(on);
}

bool NMemo::readOnly( ) const
{
  return textArea->readOnly();
}


// this is the inner class, that displays and edits the text contents

NMemo::TextArea::TextArea( )
{
  pMemo = 0;
  init();
}

NMemo::TextArea::TextArea( NMemo * memo )
{
  pMemo = memo;
  init();
}

void NMemo::TextArea::init( )
{
  // todo replace with a skin
  setBackground(NColor(255,255,255));
  setTransparent(false);

  wordWrap_ = false;
  readOnly_ = false;

  clear();
}

NMemo::TextArea::~ TextArea( )
{
}

void NMemo::TextArea::setWordWrap( bool on )
{
  wordWrap_ = on;
}

bool NMemo::TextArea::wordWrap( ) const
{
  return wordWrap_;
}

void NMemo::TextArea::setReadOnly( bool on )
{
  readOnly_ = on;
}

bool NMemo::TextArea::readOnly( ) const
{
  return readOnly_;
}

void NMemo::TextArea::setText( const std::string & text )
{
  lines.clear();

  std::string substr;
  unsigned int start = 0;
  unsigned int i = 0;
  do {
    i = text.find("\n", i);
    if (i != std::string::npos) {
       i+=1;
       substr = text.substr(start,i-start-1);
       start = i;
    } else substr = text.substr(start);
     appendLine(substr);
  } while (i != std::string::npos);

  if (lines.size() == 0) clear();

  lineIndexItr = lines.begin();
}

std::string NMemo::TextArea::text( ) const
{
  std::string text = "";
  std::vector< Line >::const_iterator it = lines.begin();
  for ( ; it < lines.end(); it++ ) {
     if (it !=lines.begin()) text+='\n';
     const Line & line = *it;
     text+=line.text();
  }
  return text;
}

void NMemo::TextArea::loadFromFile(const std::string & fileName) {
  std::fstream file(fileName.c_str());
  if (!file.is_open ()) throw "couldn't open file";

  lines.clear();

  std::string line;

  while(!file.eof())
  {
    getline(file, line, '\n');
    appendLine(line);
  }

  if (lines.size() == 0) clear();

  lineIndexItr = lines.begin();

}

NRect NMemo::TextArea::selection( ) const
{
  return selection_;
}

void NMemo::TextArea::clear( )
{
  lines.clear();
  // There is always one empty line 0
  lines.push_back(Line(this));
  // set the lineindex Iterator to begin
  lineIndexItr = lines.begin();
  // this is the x pos saver for up and down key, 
  //that always keeps the xpos , although if one line was smaller
  xupdownpos = 0;
}

void NMemo::TextArea::insertLine( const std::string & text )
{
  Line & oldLine = *lineIndexItr;
  Line newLine(this);
  newLine.insert(0,text);
  newLine.setTop(oldLine.top() + oldLine.height());
  lineIndexItr = lines.insert(lineIndexItr+1, newLine);
  moveLines(lineIndexItr+1,lines.end(),newLine.height());
}

void NMemo::TextArea::appendLine(const std::string & text) {
  Line newLine(this);
  newLine.insert(0,text);
  if (lines.size() > 0) {
    Line & oldLine = *lineIndexItr;
    newLine.setTop(oldLine.top() + oldLine.height());
  }
  lines.push_back(newLine);
  lineIndexItr = lines.end() - 1;
}

void NMemo::TextArea::deleteLine( )
{
  Line & oldLine = *lineIndexItr;
  moveLines(lineIndexItr,lines.end(), -oldLine.height() );
  lines.erase( lineIndexItr-- );  
}

void NMemo::TextArea::moveLines( std::vector< Line >::iterator from, std::vector< Line >::iterator to, int dy )
{
  std::vector< Line >::iterator moveItr = from;
  for ( ; moveItr < to ; moveItr++ ) {
     Line & line = *moveItr;
     line.move(dy);
  }
}

// the follwing methods are painting the content

void NMemo::TextArea::paint( NGraphics * g )
{
  int start = findVerticalStart();
  std::vector<Line>::iterator it = lines.begin() + start;
  for (; it < lines.end(); it++) {
     Line & line = *it;
     line.drawText(g);
     if ( line.top() + line.height() - scrollDy()  > spacingHeight() ) break;
  }
  // the cursor
  if (!readOnly()) {
    Line & actualLine = *lineIndexItr;
    NPoint3D p = actualLine.screenPos();
    drawCursor(g, p.x() , p.y() );
  }
}

void NMemo::TextArea::drawCursor( NGraphics* g, int x, int y )
{
  NColor oldColor = foreground();
  g->setForeground(font().textColor());
  g->drawLine( x , y - g->textAscent() , x , y );
  g->setForeground(oldColor);
}

int NMemo::TextArea::findVerticalStart() const
{
  int Low = 0; int High = lines.size()-1;  int Mid=High; int w=0;

  while( Low <= High ) {
    Mid = ( Low + High ) / 2;
    const Line & line = lines.at(Mid);

    if(  line.top() - scrollDy() < 0 ) {
                   if (line.top()+line.height() > scrollDy() ) {
                     return Mid;
                   }
                   Low = Mid + 1;
                   }  else 
                   {
                     if (line.top()+line.height() < scrollDy() ) {
                     return Mid;
                   }
                   High = Mid - 1;
                 }
  }
  if (Mid<0) return 0;
  return Mid;
}

void NMemo::TextArea::onKeyPress( const NKeyEvent & keyEvent )
{
  if ( !readOnly() ) {
  // dereference to the actual selected Line
  Line & line = *lineIndexItr;

  switch ( keyEvent.scancode() ) {
    case NK_Up:
       if (!line.setPosLineUp(xupdownpos) && lineIndexItr != lines.begin() ) {
          Line & oldLine = *lineIndexItr;
          lineIndexItr--;
          Line & line = *lineIndexItr;
          line.setPosToScreenPos(xupdownpos, oldLine.top() - 1 );
          window()->repaint(this,NRegion(oldLine.repaintLineArea()) | NRegion(line.repaintLineArea()),true);
       } else {
         line.repaint();
       }
    break;
    case NK_Down:
        if (!line.setPosLineDown(xupdownpos) && lineIndexItr != lines.end() -1) {
          Line & oldLine = *lineIndexItr;
          lineIndexItr++;
          Line & line = *lineIndexItr;
          line.setPosToScreenPos(xupdownpos, line.top());
          window()->repaint(this,NRegion(oldLine.repaintLineArea()) | NRegion(line.repaintLineArea()),true);
       } else line.repaint();
    break;
    case NK_Left:
      if (line.pos() == 0) {
         if (lineIndexItr > lines.begin()) {
            lineIndexItr--;
            Line & line = *lineIndexItr;
            line.setPosEnd();
            xupdownpos = line.screenPos().x();
            repaint();
         }
      } else {
        line.decPos();
        xupdownpos = line.screenPos().x();
        line.repaint();
      }
    break;
    case NK_Right:
      if (line.pos() == line.text().length()) {
        lineIndexItr++;
        if (lineIndexItr != lines.end()) {
          Line & line = *lineIndexItr;
          line.setPosStart();
          xupdownpos = line.screenPos().x();
          repaint();
        } else
        lineIndexItr--;
      } else {
        line.incPos();
        xupdownpos = line.screenPos().x();
        line.repaint();
      }
    break;
    case NK_Home:
       line.setPosStart();
       xupdownpos = line.screenPos().x();
       repaint();
    break;
    case NK_End:
       line.setPosEnd();
       xupdownpos = line.screenPos().x();
       repaint();
    break;
    case NK_BackSpace:
      if (line.pos() == 0) {
        if (lineIndexItr > lines.begin()) {
          std::string tmp = line.text();
          deleteLine();
          Line & line = *lineIndexItr;
          line.setPosEnd();
          xupdownpos = line.screenPos().x();
          line.insert(line.pos(),tmp);
          repaint();
        }
      } else {
       int oldHeight = line.height();
       line.decPos();
       line.erase(1);
       if (oldHeight!=line.height()) {
         int diff = line.height() - oldHeight;
         moveLines(lineIndexItr + 1, lines.end(), diff);
         repaint();
       } else line.repaint();

       xupdownpos = line.screenPos().x();
      }
    break;
    case NK_Return: {
       insertLine(line.deleteFromPos());
       xupdownpos = 0;
       repaint();
    }
    break;
    default:
       if (keyEvent.buffer()!="") {
          int oldHeight = line.height();
          line.insert( line.pos() , keyEvent.buffer() );
          line.incPos();
          if (oldHeight!=line.height()) {
             int diff = line.height() - oldHeight;
             moveLines(lineIndexItr + 1, lines.end(), diff);
             repaint();
          } else line.repaint();

          xupdownpos = line.screenPos().x();
       }
  }
  }
}

void NMemo::TextArea::onMousePress( int x, int y, int button )
{
  if ( button == 1 && !readOnly() ) {
    int start = findVerticalStart();
    std::vector<Line>::iterator it = lines.begin() + start;
    for ( ; it < lines.end(); it++) {
      Line & line = *it;
      if (y>= line.top()  && y < line.top() + line.height()) {
         // line found
         line.setPosToScreenPos(x,y);
         lineIndexItr = it;
         xupdownpos = line.screenPos().x();
         repaint();
         break;
      }
      if ( line.top() + line.height() - scrollDy()  > spacingHeight() ) break;
    }
  }
}

int NMemo::TextArea::preferredWidth( ) const
{
  int maxWidth = 0;
  int start = findVerticalStart();
  std::vector<Line>::const_iterator it = lines.begin() + start;
  for ( ; it < lines.end(); it++) {
    const Line & line = *it;
    maxWidth = std::max(maxWidth, line.width());
    if ( line.top() + line.height() - scrollDy()  > spacingHeight() ) break;
  }
  return maxWidth;
}

int NMemo::TextArea::preferredHeight( ) const
{
  Line line = lines.back();
  return line.top() + line.height();
}


void NMemo::TextArea::resize( )
{
  if ( wordWrap() ) {
     Line* oldLine = 0;
     std::vector<Line>::iterator it = lines.begin();
     for (; it < lines.end(); it++) {
        Line & line = *it;
        line.computeBreakPoints();
        if (oldLine) line.setTop(oldLine->top() + oldLine->height());
        oldLine = &line;
     }
  }
}

// this is the data structure for one line of the multi line Memo and inner
// classes of TextArea

NMemo::TextArea::Line::Line( ) : pArea(0), top_(0), pos_(0), height_(20)
{
}

NMemo::TextArea::Line::Line( TextArea* area ) : pArea(area), top_(0), pos_(0), height_(20)
{
}

NMemo::TextArea::Line::~ Line( )
{
}

void NMemo::TextArea::Line::setText( const std::string & text )
{
  text_ = text;
}

const std::string & NMemo::TextArea::Line::text( ) const
{
  return text_;
}


// this sets and gets the beginning position in the textArea

void NMemo::TextArea::Line::setTop( int top )
{
  top_ = top;
}

void NMemo::TextArea::Line::move( int dy )
{
  top_+= dy;
}

int NMemo::TextArea::Line::top( ) const
{
  return top_;
}

// the height of the line

int NMemo::TextArea::Line::height( ) const
{
  if (!pArea->wordWrap()) {
    NFontMetrics metrics(pArea->font());
    return metrics.textHeight();
  } else {
    return height_;
  }
}

// this sets and gets the text position in the line

void NMemo::TextArea::Line::incPos( )
{
  if (pos_ < text_.length() ) pos_++;
}

void NMemo::TextArea::Line::decPos( )
{
  if (pos_ > 0) pos_--;
}

void NMemo::TextArea::Line::setPosEnd( )
{
   pos_ = text_.length();
}

void NMemo::TextArea::Line::setPos( unsigned int pos )
{
  if (pos < text_.length()) pos_ = pos;
}

void NMemo::TextArea::Line::setPosStart( )
{
  pos_ = 0;
}

unsigned int NMemo::TextArea::Line::pos( ) const
{
  return pos_;
}

NPoint3D NMemo::TextArea::Line::screenPos( ) const
{
  NPoint3D position;
  NFontMetrics metrics(pArea->font());
  if ( !pArea->wordWrap() ) {
    position.setX( metrics.textWidth(text_.substr(0,pos_)) );
    position.setY( top() + metrics.textAscent() );
    position.setZ( 0 );
  } else {
    int yp = top() + metrics.textAscent();
    int pos = 0;
    int z = 0;
    for (std::vector<int>::const_iterator it = breakPoints.begin(); it < breakPoints.end(); it++) {
      int lineEnd = *it;
      if (pos_ < lineEnd) {
         position.setX( metrics.textWidth( text_.substr(pos, pos_-pos)) );
         position.setY(yp);
         position.setZ(z);
         return position;
      }
      yp+= metrics.textHeight();
      z++;
      pos = lineEnd;
    }
    position.setX( metrics.textWidth( text_.substr(pos,pos_-pos)) );
    position.setY(yp);
    position.setZ(z);
    return position;
  }
  return position;
}

void NMemo::TextArea::Line::setPosToScreenPos( int x, int y )
{
  if ( pArea->wordWrap() ) {
    NFontMetrics metrics(pArea->font());
    int yp = top();
    int pos = 0;

    for (std::vector<int>::iterator it = breakPoints.begin(); it < breakPoints.end(); it++) {
      int lineEnd = *it;
      if (y >= yp && y < yp + metrics.textHeight()) {
        // right line
        pos_ = pos + findWidthMax(x,text_.substr(pos,lineEnd-pos),false);
        if (pos_ > text_.length()) pos_ = text_.length();
        return;
      }
      yp+=metrics.textHeight();
      pos = lineEnd;
    }
    pos_ = pos + findWidthMax(x,text_.substr(pos),false);
    if (pos_ > text_.length()) pos_ = text_.length();
  } else {
    pos_ = findWidthMax(x,text_,false);
    if (pos_ > text_.length()) pos_ = text_.length();
  }
}

bool NMemo::TextArea::Line::setPosLineUp(int x)
{
  if ( pArea->wordWrap() ) {
      NPoint3D p = screenPos();
      if (p.z() == 0) return false;
      NFontMetrics metrics(pArea->font());
      p.setY(p.y() - metrics.textHeight());
      setPosToScreenPos(x , p.y() );
      return true;
  }
  return false;
}

bool NMemo::TextArea::Line::setPosLineDown(int x)
{
  if ( pArea->wordWrap() ) {
      NPoint3D p = screenPos();
      NFontMetrics metrics(pArea->font());
      if (p.y() + metrics.textHeight() > top() + height() ) return false;
      p.setY(p.y() + metrics.textHeight());
      setPosToScreenPos(x , p.y() );
      return true;
  }
  return false;
}

int NMemo::TextArea::Line::width( ) const
{
  NFontMetrics metrics(pArea->font());
  return metrics.textWidth(text_);
}

void NMemo::TextArea::Line::computeBreakPoints( )
{
  breakPoints.clear();  // clear old breakpoints

  std::string part = text_;

  int last = 0;
  unsigned int pos  = 0;
  while ( (pos = findWidthMax(pArea->spacingWidth(),part,true)) < part.length() && (pos!=0)) {
     part = part.substr(pos);
     last  += pos;
     breakPoints.push_back(last);
  }

  int yp =  0;
  pos = 0;
  NFontMetrics metrics(pArea->font());
  for (std::vector<int>::const_iterator it = breakPoints.begin(); it < breakPoints.end(); it++) {
    int lineEnd = *it;
    yp+=metrics.textHeight();
  }
  yp+=metrics.textHeight();
  height_ = yp;
}

int NMemo::TextArea::Line::findWidthMax(long width, const std::string & data, bool wbreak)
{
  NFontMetrics metrics(pArea->font());

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
  if (!wbreak || data.substr(0,Mid).find(" ")==std::string::npos || Mid == 0 || Mid>=data.length()) return Mid; else
  {
    unsigned int p = data.rfind(" ",Mid);
    if (p!=std::string::npos ) return p+1;
  }
  return Mid;
}

void NMemo::TextArea::Line::drawText( NGraphics * g )
{
  if (!pArea->wordWrap()) {
     g->drawText( 0 , top() + g->textAscent() , text_);
  } else
  {
    int yp = top() + g->textAscent();
    int pos = 0;

    for (std::vector<int>::iterator it = breakPoints.begin(); it < breakPoints.end(); it++) {
      int lineEnd = *it;
      g->drawText( 0, yp, text_.substr(pos,lineEnd-pos));
      yp+=g->textHeight();
      pos = lineEnd;
    }
    g->drawText( 0, yp, text_.substr(pos) );
  }
}

void NMemo::TextArea::Line::insert( unsigned int pos , const std::string & text )
{
  text_.insert(pos_,text);
  if (pArea->wordWrap()) {
    computeBreakPoints();
  }
}

void NMemo::TextArea::Line::erase( unsigned int count )
{
  text_.erase(pos_,count);
  if (pArea->wordWrap()) {
    computeBreakPoints();
  }
}

std::string NMemo::TextArea::Line::strFromPos( ) const
{
  return text_.substr(pos_);
}

std::string NMemo::TextArea::Line::deleteFromPos( )
{
  std::string tmp = text_.substr(pos_);
  text_.erase(pos_, text_.length() - pos_ );
  if (pArea->wordWrap()) {
    computeBreakPoints();
  }
  return tmp;
}

std::string NMemo::TextArea::Line::strToPos( ) const
{
  return text_.substr(0,pos_);
}

std::string NMemo::TextArea::Line::deleteToPos( )
{
  std::string tmp = text_.substr(0,pos_);
  text_.erase(0,pos_);
  return tmp;
}

void NMemo::TextArea::Line::repaint( )
{
  pArea->window()->repaint(pArea,NRegion(repaintLineArea()),true);
}

NRect NMemo::TextArea::Line::repaintLineArea( ) const
{
  int top_    = pArea->absoluteTop() + top() - pArea->scrollDy();
  int bottom_ = top_  + height();
  int left_   = pArea->absoluteLeft() - pArea->scrollDx();
  int right_  = left_ + pArea->spacingWidth();

  return NRect(left_,top_,right_ - left_,bottom_ - top_);
}

void NMemo::resize( )
{
  NTextBase::resize();
  NFontMetrics metrics( font() );
  vBar->setSmallChange( metrics.textHeight() );
  vBar->setLargeChange( metrics.textHeight() );        
  hBar->setSmallChange( metrics.textHeight() );
  hBar->setLargeChange( metrics.textHeight() );
}
