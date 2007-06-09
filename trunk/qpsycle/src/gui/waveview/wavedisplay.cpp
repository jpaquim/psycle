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

WaveDisplay::WaveDisplay(bool *mini, QWidget *parent); //inizialization file for WaveDisplay.
{
	setParent( parent);
	if (mini == true)
	{
		//if it is a miniwaveview set fixed sizes and dimension
		setSizePolicies( QSizePolicy::Fixed , QSizePolicy::Preferred);
		resize (this.width(), 200);
    }        
    else
	{
		//if it is not mini set size policies only
		setSizePolicies( QSizePolicy::Preferred, QSizePolicy::Preferred);
	}
	scene = new QGraphicsScene(this);
	scene->setBackgroundBrush(Qt::black);
    adjustSize();
	Clear();
	return;
}

void WaveDisplay::LoadStereo()
{
	//we need to add a line at the first quarter and one at the last quarter of GraphicsView To Make that loads a Stereo file...
}

void WaveDisplay::LoadMono()
{
	//we need to add al line at the middle GraphicsView To Make that loads a Mono file...
	QGraphicsLineItem *line = new QGraphicsLineItem(0,this.height()/2,this.width(),this.height()/2, 0, scene);
	scene->addItem(line);
}

void WaveDisplay::Clear()
{	
	//Clear Oprions Not Avable Now
	nodata = new QGraphicsTextItem ("No Wave Data", this, scene);
	scnen->addItem(nodata);
}