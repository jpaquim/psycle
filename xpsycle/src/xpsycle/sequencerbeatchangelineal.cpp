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
#include "sequencerbeatchangelineal.h"
#include "sequencergui.h"
#include "patternsequence.h"
#include <ngrs/nfontmetrics.h>
#include <ngrs/nedit.h>

namespace psycle {
	namespace host {


SequencerBeatChangeLineal::BeatChangeTriangle::BeatChangeTriangle( )
{
  setMoveable( nMvHorizontal );

  tWidth  = 10;
  tHeight = 10;

  bpmEdt_ = new NEdit("120");
  add(bpmEdt_);

  bpmChangeEvent_ = 0;

  sView = 0;
}

SequencerBeatChangeLineal::BeatChangeTriangle::BeatChangeTriangle( SequencerGUI * gui )
{
    setMoveable( nMvHorizontal );

  tWidth  = 10;
  tHeight = 10;

  bpmEdt_ = new NEdit("120");
  add(bpmEdt_);

  bpmChangeEvent_ = 0;

  sView = gui;

}

SequencerBeatChangeLineal::BeatChangeTriangle::~ BeatChangeTriangle( )
{
}

void SequencerBeatChangeLineal::BeatChangeTriangle::setBpmChangeEvent( BpmChangeEvent * event )
{
  bpmChangeEvent_ = event;
  bpmEdt_->setText( stringify( event->bpm() ) );
}

void SequencerBeatChangeLineal::BeatChangeTriangle::paint( NGraphics * g )
{
  int cw = clientWidth();
  int ch = clientHeight();

  XPoint pts[3];

  pts[0].x = cw / 2;
  pts[0].y = 0;
  pts[1].x = (cw + tWidth) / 2;
  pts[1].y = tHeight;
  pts[2].x = (cw - tWidth) / 2;
  pts[2].y = tHeight;

  g->setForeground( NColor(0,255,0) );
  g->fillPolygon(pts, 3);
  g->setForeground( NColor(150,150,150));
  g->drawPolygon(pts, 3);

}

int SequencerBeatChangeLineal::BeatChangeTriangle::preferredHeight( ) const
{
  NFontMetrics metrics( font() );
  return tHeight + 2 + metrics.textHeight();
}

void SequencerBeatChangeLineal::BeatChangeTriangle::resize( )
{
  int cw = clientWidth();
  NFontMetrics metrics( font() );
  bpmEdt_->setPosition( (cw - bpmEdt_->preferredWidth() ) / 2 , tHeight + 2 , bpmEdt_->preferredWidth() , bpmEdt_->preferredHeight() );
}

void SequencerBeatChangeLineal::BeatChangeTriangle::onMove( const NMoveEvent & moveEvent )
{
  sView->patternSequence()->MoveBpmChangeEntry(bpmChangeEvent_, left() / (double) sView->beatPxLength() );
}

// the Main class of the BeatChangeLineal

SequencerBeatChangeLineal::SequencerBeatChangeLineal()
 : NPanel()
{
  sView = 0;
}

SequencerBeatChangeLineal::SequencerBeatChangeLineal( SequencerGUI * seqGui )
{
  sView = seqGui;
}

SequencerBeatChangeLineal::~SequencerBeatChangeLineal()
{
}

void SequencerBeatChangeLineal::paint( NGraphics * g )
{
  g->setForeground(NColor(0,0,220));

  int cw = clientWidth();
  int ch = clientHeight();

  std::string timeScaleText = "bpm change";
  int rightIdent = 2;
  int scaleTextWidth = g->textWidth(timeScaleText) + rightIdent;
  g->drawText(cw - scaleTextWidth, 10 + 2 + g->textAscent(), timeScaleText);

  g->setForeground(NColor(220,220,220));

  g->drawLine(0, 10, cw, 10);

  for (int i = 0; i < cw / sView->beatPxLength() ; i++) {
     if (! (i % 4)) {
        g->setForeground(NColor(180,180,180));
        g->drawLine(i* sView->beatPxLength(),10,d2i(i*sView->beatPxLength()), 0);
     }
     else {
        g->setForeground(NColor(220,220,220));
        g->drawLine(i* sView->beatPxLength(),10,d2i(i*sView->beatPxLength()), 5);
    }
  }

}

void SequencerBeatChangeLineal::onMouseDoublePress( int x, int y, int button )
{
  if (button == 1) {
    BeatChangeTriangle* triangle = new BeatChangeTriangle(sView);
      BpmChangeEvent* bpmChangeEvent = sView->patternSequence()-> createBpmChangeEntry(x / (double) sView->beatPxLength() ,120);
      triangle->setBpmChangeEvent(bpmChangeEvent);
      triangle->setPosition(x, 10,30,30);
    add(triangle);
    repaint();
  }
}


int SequencerBeatChangeLineal::preferredHeight( ) const
{
  return 50;
}

void SequencerBeatChangeLineal::resize( )
{
  std::vector<NVisualComponent*>::const_iterator it = visualComponents().begin();
  for ( ; it != visualComponents().end(); it++) {
    NVisualComponent* comp = *it;
    comp->setHeight(comp->preferredHeight());
  }
}


}}


















