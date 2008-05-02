// -*- mode:c++; indent-tabs-mode:t -*-
/***************************************************************************
*   Copyright (C) 2007 Psycledelics Community   *
*   psycle.sourceforge.net   *
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
#include "wavedisplay.hpp"

#include <psycle/core/instrument.h>
#include "../../model/instrumentsmodel.hpp"

#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QStyleOptionGraphicsItem>
#include <QResizeEvent>

namespace qpsycle {

WaveDisplay::WaveDisplay( QWidget *parent, InstrumentsModel *instModel )
	: QGraphicsView( parent )
{
	scene_ = new QGraphicsScene(this);
	scene_->setBackgroundBrush(Qt::black);
	wave_ = new WaveItem(this, instModel, scene_);
	setScene(scene_);
	setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
}

void WaveDisplay::resizeEvent(QResizeEvent *ev)
{
	///\todo widening the view probably shouldn't change the horizontal scale
	int oldheight = ev->oldSize().height();
	int oldwidth = ev->oldSize().width();
	if ( oldheight > 0 && oldwidth > 0 ) {
		scale(
			ev->size().width() / (float) oldwidth,
			ev->size().height() / (float) oldheight
		);
	}

	wave_->resize();

	QGraphicsView::resizeEvent(ev);
}

void WaveDisplay::ZoomIn()
{
	scale( 1.5, 1 );
	wave_->IncrementZoomLevel();
}

void WaveDisplay::ZoomOut()
{
	if (wave_->GetZoomLevel() > 0)
	{
	scale( 2.0/3.0, 1 );
	wave_->DecrementZoomLevel();
	}
}

WaveItem::WaveItem( WaveDisplay *disp, InstrumentsModel *instModel, QGraphicsScene *scene )
	: QGraphicsRectItem( 0, scene )
	, instrumentsModel_( instModel )
	, disp_( disp )
{
	connect (
		instModel, SIGNAL(selectedInstrumentChanged(int)),
		this, SLOT( resetInstrument() )
	);

	nodata_ = new QGraphicsTextItem( tr("No wave data."), this, this->scene() );
	nodata_->setDefaultTextColor( Qt::white );
	nodata_->setFont( QFont("verdana", 10) );

	resetInstrument();
}

void WaveItem::resetInstrument()
{
	int curInst = instrumentsModel_->selectedInstrumentIndex();
	if ( instrumentsModel_->slotIsEmpty(curInst) ) {
		inst_=0;
		nodata_->show();
		setRect( QRectF(0, 0, 100, 20) );
		disp_->resetMatrix();
		disp_->centerOn(nodata_);
	} else {
		inst_ = instrumentsModel_->getInstrument( curInst );
		nodata_->hide();
		setRect( QRectF(0, 0, inst_->waveLength, 1024) );
		disp_->fitInView( rect() );
	}

	scene()->setSceneRect( rect() );
	zoomLevel = 0;
}

void WaveItem::resize()
{
	if ( !inst_ ) {
		setRect( QRectF(0, 0, 100, 20) );
		disp_->resetMatrix();
		disp_->centerOn(nodata_);
		scene()->setSceneRect( rect() );
	}
}

void WaveItem::paint (
	QPainter * painter,
	const QStyleOptionGraphicsItem * option,
	QWidget * widget )
{
	if (inst_) {
		int halfWaveHeight = (int)( rect().height() / (inst_->waveStereo? 4: 2) );
		int midPoint = halfWaveHeight;

		double startx = option->exposedRect.left();
		double endx = option->exposedRect.right();

		painter->setPen( Qt::darkGreen );
		painter->drawLine( QLineF( startx, midPoint, endx, midPoint ) );
		painter->setPen( Qt::green );

		//inverse of the view's horizontal scaling factor
		double samplesPerPixel = 1.0 / option->matrix.m11();

		///\todo draw data differently when samplesPerPixel<1

		for ( double i(startx); i < inst_->waveLength && i <= endx; i+=samplesPerPixel ) {
			int sample = halfWaveHeight * *(inst_->waveDataL + (int)i) / (1<<15);
			painter->drawLine( QLineF( i, midPoint, i, midPoint-sample ) );
		}

		if ( inst_->waveStereo ) {
			midPoint *= 3;

			painter->setPen( Qt::darkGreen );
			painter->drawLine( QLineF( startx, midPoint, endx, midPoint ) );
			painter->setPen( Qt::green );
			for ( double i(startx); i < inst_->waveLength && i <= endx; i+=samplesPerPixel ) {
				int sample = halfWaveHeight * *(inst_->waveDataR + (int)i) / (1<<15);
				painter->drawLine( QLineF( i, midPoint, i, midPoint-sample ) );
			}
		}

		if ( inst_->waveLoopType ) {
			painter->setPen( QColor(0, 128, 200) );
			if ( inst_->waveLoopStart >= startx )
				painter->drawLine( QLineF( inst_->waveLoopStart, 0, inst_->waveLoopStart, rect().height() ) );
			if ( inst_->waveLoopEnd <= endx )
				painter->drawLine( QLineF( inst_->waveLoopEnd, 0, inst_->waveLoopEnd, rect().height() ) );
		}

	}
	
}
} // namespace qpsycle
