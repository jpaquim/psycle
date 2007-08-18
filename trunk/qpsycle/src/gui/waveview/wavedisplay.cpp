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
#include "wavedisplay.h"
#include <QGraphicsView>
#include <QGraphicsLineItem>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsScene> 

WaveDisplay::WaveDisplay(bool mini, QWidget *parent) //inizialization file for WaveDisplay.
{
	setParent( parent);
	wavescene = new QGraphicsScene();
	wavescene->setBackgroundBrush(Qt::black);
	setScene(wavescene);
	setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
	Reset();
}

void WaveDisplay::LoadStereo()
{
	//we need to add a line at the first quarter and one at the last quarter of GraphicsView To Make that loads a Stereo file...
}

void WaveDisplay::LoadMono()
{
	//we need to add al line at the middle GraphicsView To Make that loads a Mono file...
	QGraphicsLineItem *line = new QGraphicsLineItem(0,height()/2,width(),height()/2, 0, wavescene);
	wavescene->addItem(line);
}

void WaveDisplay::Reset()
{
	//Clear Oprions Not Avable Now
	nodata = new QGraphicsTextItem("No Wave Data", 0);
	nodata->setDefaultTextColor(Qt::white);
	wavescene->addItem(nodata);
}

void WaveDisplay::Update()
{
	
}
