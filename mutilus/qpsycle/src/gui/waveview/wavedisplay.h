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
#ifndef WAVEDISPLAY_H
#define WAVEDISPLAY_H

#include <QGraphicsView>
#include <QGraphicsLineItem>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsScene>
	
class WaveDisplay : public QGraphicsView
{
		Q_OBJECT


public:
	WaveDisplay(bool mini, QWidget *parent);

public slots:
	void LoadStereo();
	void LoadMono();
	void Reset();
	void Update();
private:
	QGraphicsScene *wavescene;
	
	QGraphicsTextItem *nodata;
	//QGraphicsTextItem *left;
	//QGraphicsTextItem *right;
	
	
	#if 0
	QGraphicsLineItem *stereo_centerline;
	QGraphicsLineItem *stereo_leftline;
	QGraphicsLineItem *stereo_rightline;
	#endif
	
};

#endif
