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
#include "ngroupbox.h"
#include "nfontmetrics.h"

/// GBorder Class

// creates the border for the groupbox
NGroupBox::GBorder::GBorder( )
{
}

NGroupBox::GBorder::~GBorder( )
{
}

// clones the border and uses the copy ctor
NBorder* NGroupBox::GBorder::clone( ) const
{
  return new GBorder(*this);
}

// sets the border text
void NGroupBox::GBorder::setHeaderText( const std::string & text )
{
  headerText_ = text;
}

// paints the border
void NGroupBox::GBorder::paint( NGraphics * g, const NShape & geometry )
{
	g->setForeground(NColor(130,130,130));
  NRect r = geometry.rectArea();

  // we need to draw the oval rect manually with arcs, due the text hole

	int left_h   = spacing().left()   / 2;
	int right_h  = spacing().right()  / 2;
	int top_h    = spacing().top()    / 2;
	int bottom_h = spacing().bottom() / 2;

  int nx = r.left() + left_h;
  int ny = r.top() + top_h ;
  int nw = r.width() - left_h - right_h -1;
  int nh = r.height() - bottom_h - top_h -1;
  int naw = 5; // arcWidth
  int nah = 5; // arcHeight

  int leftTextIdent = r.left() + spacing().left() + naw;

  // this code is copied from NGraphics drawRoundRect and modified for the text hole

  if (nw < 0) { 
    nw = 0 - nw;
    nx = nx - nw;
  }
  if (nh < 0) {
    nh = 0 - nh;
    ny = ny - nh;
  }
  if (naw < 0)  naw = 0 - naw;
  if (nah < 0)  nah = 0 - nah;

  int naw2 = naw / 2;
  int nah2 = nah / 2;

  if (nw > naw) {
                if (nh > nah) {
                        g->drawArc(nx, ny, naw, nah, 5760, 5760);
                        // make hole for our text
                        g->drawLine( nx + naw2, ny, leftTextIdent, ny );
                        g->drawLine( leftTextIdent + g->textWidth( headerText_), ny, nx + nw - naw2, ny );
                        // end of modification
                        g->drawArc(nx + nw - naw, ny, naw, nah, 0, 5760);
                        g->drawLine(nx + nw, ny + nah2, nx + nw, ny + nh - nah2);
                        g->drawArc(nx + nw - naw, ny + nh - nah, naw, nah, 17280, 5760);
                        g->drawLine(nx + naw2, ny + nh, nx + nw - naw2, ny + nh);
                        g->drawArc(nx, ny + nh - nah, naw, nah, 11520, 5760);
                        g->drawLine(nx, ny + nah2, nx, ny + nh - nah2);
                } else {
                        g->drawArc(nx, ny, naw, nh, 5760, 11520);
                        g->drawLine(nx + naw2, ny, nx + nw - naw2, ny);
                        g->drawArc(nx + nw - naw, ny, naw, nh, 17280, 11520);
                        g->drawLine(nx + naw2, ny + nh, nx + nw - naw2, ny + nh);
                }
        } else {
                if (nh > nah) {
                        g->drawArc(nx, ny, nw, nah, 0, 11520);
                        g->drawLine(nx + nw, ny + nah2, nx + nw, ny + nh - nah2);
                        g->drawArc(nx, ny + nh - nah, nw, nah, 11520, 11520);
                        g->drawLine(nx, ny + nah2, nx, ny + nh - nah2);
                } else {
                        g->drawArc(nx, ny, nw, nh, 0, 23040);
                }
        }

  // finally we draw the text
  g->drawText( leftTextIdent ,r.top() + g->textAscent(), headerText_ );
}

/// end of GroupBox border



// the groupbox class itself

NGroupBox::NGroupBox()
 : NPanel()
{
	gBorder.setHeaderText("GroupBox");
  NFontMetrics metrics(font());
	gBorder.setSpacing( NSize( 5, metrics.textHeight(), 5, 5) );
  setBorder( gBorder );
}


NGroupBox::NGroupBox( const std::string & text) {
  gBorder.setHeaderText(text);
  NFontMetrics metrics(font());
	gBorder.setSpacing( NSize( 5, metrics.textHeight(), 5, 5) );
	setBorder( gBorder );
}

NGroupBox::~NGroupBox()
{
}

void NGroupBox::setHeaderText( const std::string & text )
{
	gBorder.setHeaderText(text);
	setBorder( gBorder );
}

// class factories for c bindings

extern "C" NObject* createGroupBox() {
    return new NGroupBox();
}

extern "C" void destroyGroupBox(NObject* p) {
    delete p;
}
