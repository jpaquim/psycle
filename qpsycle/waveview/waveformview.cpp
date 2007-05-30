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
#include "waveformview.h"

WaveFormView::WaveFormView(bool *mini, QWidget *parent); //inizialization file for WaveFormView.
{
	if (mini = true)
	{
		//if it is a miniwaveview set fixed sizes and dimension
		this.setSizePolicies( QSizePolicy::Fixed , QSizePolicy::Preferred);
		this.resize (this.width(), 200);
	}
	else
	{
		//if it is not mini set size policies only
		this.setSizePolicies( QSizePolicy::Preferred, QSizePolicy::Preferred);
	}
	*scene = new QGraphicsScene(this);
	scene->setBackgroundBrush(Qt::black);
    this.adjustSize();

}

WaveFormView::LoadStereo()
{
	//we need to add a line at the first quarter and one at the last quarter of GraphicsView To Make that loads a Stereo file...
}

WaveFormView::LoadMono()
{
	//we need to add al line at the middle GraphicsView To Make that loads a Mono file...
}

WaveFormView::Clear()
{
	 
}