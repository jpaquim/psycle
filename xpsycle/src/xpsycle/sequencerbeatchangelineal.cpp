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
#include <stdexcept>

namespace psycle {
	namespace host {


SequencerBeatChangeLineal::BeatChangeTriangle::BeatChangeTriangle( )
{
  setMoveable( ngrs::nMvLeftLimit | ngrs::nMvHorizontal );

  tWidth  = 10;
  tHeight = 10;

  bpmEdt_ = new ngrs::NEdit("120");
  add(bpmEdt_);

  bpmChangeEvent_ = 0;

  sView = 0;
}

SequencerBeatChangeLineal::BeatChangeTriangle::BeatChangeTriangle( SequencerGUI * gui )
{
    setMoveable( ngrs::nMvHorizontal );

  tWidth  = 10;
  tHeight = 10;

  bpmEdt_ = new ngrs::NEdit("120");
  add(bpmEdt_);

  bpmEdt_->keyPress.connect(this,&SequencerBeatChangeLineal::BeatChangeTriangle::onKeyPress);

  bpmChangeEvent_ = 0;

  sView = gui;

}

SequencerBeatChangeLineal::BeatChangeTriangle::~ BeatChangeTriangle( )
{
}

void SequencerBeatChangeLineal::BeatChangeTriangle::setBpmChangeEvent( GlobalEvent * event )
{
  if(event->type() != GlobalEvent::BPM_CHANGE ) throw std::invalid_argument("Wrong global event type in setBpmChangeEvent()");
  bpmChangeEvent_ = event;
  bpmEdt_->setText( stringify( event->parameter() ) );
}

GlobalEvent * SequencerBeatChangeLineal::BeatChangeTriangle::bpmChangeEvent( )
{
  return bpmChangeEvent_;
}

void SequencerBeatChangeLineal::BeatChangeTriangle::paint( ngrs::NGraphics * g )
{
  int cw = clientWidth();
  int ch = clientHeight();

  ngrs::NPoint pts[3];

  pts[0].setX( cw / 2 );
  pts[0].setY( 0 );
  pts[1].setX( (cw + tWidth) / 2 );
  pts[1].setY( tHeight );
  pts[2].setX( (cw - tWidth) / 2 );
  pts[2].setY( tHeight );

  g->setForeground( ngrs::NColor(0,255,0) );
  g->fillPolygon(pts, 3);
  g->setForeground( ngrs::NColor(150,150,150));
  g->drawPolygon(pts, 3);

}

int SequencerBeatChangeLineal::BeatChangeTriangle::preferredHeight( ) const
{
  ngrs::NFontMetrics metrics( font() );
  return tHeight + 2 + metrics.textHeight();
}

void SequencerBeatChangeLineal::BeatChangeTriangle::resize( )
{
  int cw = clientWidth();
  ngrs::NFontMetrics metrics( font() );
  bpmEdt_->setPosition( (cw - bpmEdt_->preferredWidth() ) / 2 , tHeight + 2 , bpmEdt_->preferredWidth() , bpmEdt_->preferredHeight() );
}

void SequencerBeatChangeLineal::BeatChangeTriangle::onMove( const ngrs::NMoveEvent & moveEvent )
{
  sView->patternSequence()->moveGlobalEvent(bpmChangeEvent_, (left() +15) / (double) sView->beatPxLength() );
}

void SequencerBeatChangeLineal::BeatChangeTriangle::onKeyPress( const ngrs::NKeyEvent & event )
{
  bpmChangeEvent_->setParameter( ngrs::str<float>(bpmEdt_->text()) );
}

// the Main class of the BeatChangeLineal

SequencerBeatChangeLineal::SequencerBeatChangeLineal()
 : ngrs::NPanel()
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

void SequencerBeatChangeLineal::paint( ngrs::NGraphics * g )
{
  ngrs::NRect area = g->repaintArea().rectClipBox();

  g->setForeground( ngrs::NColor(0,0,220) );

  int cw = clientWidth();
  int ch = clientHeight();

  /*std::string timeScaleText = "bpm change";
  int rightIdent = 2;
  int scaleTextWidth = g->textWidth(timeScaleText) + rightIdent;
  g->drawText(cw - scaleTextWidth + scrollDx(), 10 + 2 + g->textAscent(), timeScaleText);*/

  g->setForeground( ngrs::NColor( 220, 220, 220) );

  g->drawLine(scrollDx(), ch - 10 , cw + scrollDx(), ch - 10);

  int start = (area.left() - absoluteLeft() + scrollDx()) / sView->beatPxLength();
  int end   = (area.left() + area.width() - absoluteLeft() + scrollDx() ) / sView->beatPxLength();


  for (int i = start; i < end ; i++) {
     if (! (i % 16)) {
        g->setForeground( ngrs::NColor( 180, 180, 180) );
        g->drawLine(i* sView->beatPxLength(),10,d2i(i*sView->beatPxLength()), 0);
     }
     else {
        g->setForeground( ngrs::NColor( 220, 220, 220) );
        g->drawLine(i* sView->beatPxLength(),10,d2i(i*sView->beatPxLength()), 5);
    }
  }

}

void SequencerBeatChangeLineal::onMouseDoublePress( int x, int y, int button )
{
  if (button == 1) {
    BeatChangeTriangle* triangle = new BeatChangeTriangle(sView);
      GlobalEvent* bpmChangeEvent = sView->patternSequence()-> createBpmChangeEntry(x / (double) sView->beatPxLength() ,120.0f);
      triangle->setBpmChangeEvent(bpmChangeEvent);
      triangle->setPosition(x-15, 10,30,30);
    add(triangle);
    beatChanges.push_back(triangle);
    resize();
    repaint();
  }
}


int SequencerBeatChangeLineal::preferredHeight( ) const
{
  return 50;
}

void SequencerBeatChangeLineal::resize( )
{
  std::list<BeatChangeTriangle*>::iterator it = beatChanges.begin();
  for ( ; it !=beatChanges.end(); it++) {
    BeatChangeTriangle* item = *it;
    double tickPosition = sView->patternSequence()->globalTickPosition(item->bpmChangeEvent() );
    item->setPosition(d2i(sView->beatPxLength() * tickPosition) - 15,5,30,item->preferredHeight());
  }
}


} // end of host namespace
} // end of psycle namespace
