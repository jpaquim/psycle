/***************************************************************************
 *   Copyright (C) 2006 by Stefan Nattkemper   *
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
#include <fstream>
#include <sstream>

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
  return "";
}

void NMemo::setText( const std::string & text )
{
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

void NMemo::setWordBreak( bool on )
{
  textArea->setWordBreak(on);
}

bool NMemo::wordBreak( ) const
{
  return textArea->wordBreak();
}

// clears the text content

void NMemo::clear( )
{
  textArea->clear();
}


// this is the inner class, that displays and edits the text contents

NMemo::TextArea::TextArea( )
{
  init();
}

NMemo::TextArea::TextArea( NMemo * memo )
{
  init();
  pMemo = pMemo;
}

void NMemo::TextArea::init( )
{
  // todo replace with a skin
  setBackground(NColor(255,255,255));
  setTransparent(false);

  clear();
}

NMemo::TextArea::~ TextArea( )
{
}

void NMemo::TextArea::setWordBreak( bool on )
{
  wordBreak_ = on;
}

bool NMemo::TextArea::wordBreak( ) const
{
  return wordBreak_;
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


void NMemo::TextArea::clear( )
{
  lines.clear();
  // There is always one empty line 0
  lines.push_back(Line(this));
  // set the lineindex Iterator to begin
  lineIndexItr = lines.begin();
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
  lines.erase(lineIndexItr);
  lineIndexItr--;
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
  int xp = 0;
  int start = findVerticalStart();
  std::vector<Line>::iterator it = lines.begin() + start;
  for (; it < lines.end(); it++) {
     Line & line = *it;
     g->drawText(xp , line.top() + g->textAscent() , line.text());
     if ( line.top() + line.height() - scrollDy()  > spacingHeight() ) break;
  }
  // the cursor
  Line & actualLine = *lineIndexItr;
  drawCursor(g, actualLine.screenXPos() , actualLine.top() + g->textAscent() );
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
  // dereference to the actual selected Line
  Line & line = *lineIndexItr;

  switch ( keyEvent.scancode() ) {
    case XK_Left:
      if (line.pos() == 0) {
         if (lineIndexItr > lines.begin()) {
            lineIndexItr--;
            Line & line = *lineIndexItr;
            line.setPosEnd();
            repaint();
         }
      } else {
        line.decPos();
        repaint();
      }
    break;
    case XK_Right:
      if (line.pos() == line.text().length()) {
        lineIndexItr++;
        if (lineIndexItr != lines.end()) {
          Line & line = *lineIndexItr;
          line.setPosStart();
          repaint();
        } else
        lineIndexItr--;
      } else {
        line.incPos();
        repaint();
      }
    break;
    case XK_Home:
       line.setPosStart();
       repaint();
    break;
    case XK_End:
       line.setPosEnd();
       repaint();
    break;
    case XK_BackSpace:
      if (line.pos() == 0) {
        if (lineIndexItr > lines.begin()) {
          std::string tmp = line.text();
          deleteLine();
          Line & line = *lineIndexItr;
          line.setPosEnd();
          line.insert(line.pos(),tmp);
          repaint();
        }
      } else {
       line.decPos();
       line.erase(1);
       repaint();
      }
    break;
    case XK_Return: {
       insertLine(line.deleteFromPos());
       repaint();
    }
    break;
    default:
       if (keyEvent.buffer()!="") {
          line.insert( line.pos() , keyEvent.buffer() );
          line.incPos();
          repaint();
       }
  }
}

// this is the data structure for one line of the multi line Memo and inner
// classes of TextArea

NMemo::TextArea::Line::Line( ) : pArea(0), top_(0), pos_(0)
{
}

NMemo::TextArea::Line::Line( TextArea* area ) : pArea(area), top_(0), pos_(0)
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
  return 20;
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

int NMemo::TextArea::Line::screenXPos( ) const
{
  NFontMetrics metrics(pArea->font());
  return metrics.textWidth(text_.substr(0,pos_));
}

int NMemo::TextArea::Line::width( ) const
{
  NFontMetrics metrics(pArea->font());
  return metrics.textWidth(text_);
}

void NMemo::TextArea::Line::insert( unsigned int pos , const std::string & text )
{
  text_.insert(pos_,text);
}

void NMemo::TextArea::Line::erase( unsigned int count )
{
  text_.erase(pos_,count);
}

std::string NMemo::TextArea::Line::strFromPos( ) const
{
  return text_.substr(pos_);
}

std::string NMemo::TextArea::Line::deleteFromPos( )
{
  std::string tmp = text_.substr(pos_);
  text_.erase(pos_, text_.length() - pos_ );
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

