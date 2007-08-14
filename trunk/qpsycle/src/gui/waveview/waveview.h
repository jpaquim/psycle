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
	#ifndef WAVEVIEW_H
	#define WAVEVIEW_H

#include "wavedisplay.h"
#include <QVBoxLayout>
#include <QToolBar>
#include <QWidget>
#include <QSlider>
#include <QLabel>


class QStandardItemModel;
class InstrumentsModel;

class WaveView : public QWidget
{
	Q_OBJECT

public:
	WaveView( InstrumentsModel *instrumentsModel_,
			QWidget *parent = 0);
	QLabel *sampName_;

public slots:
	void onLoadButtonClicked();
	void onSaveButtonClicked();
	void onPlusButtonClicked();
	void onMinusButtonClicked();
	void onAmpButtonClicked();
	void onSelectorButtonClicked();
	void onZoomButtonClicked();

signals:
	void sampleAdded();

private:
	QVBoxLayout *layout_;
	
	QToolBar *toolBar_;
	QAction *loadSmp_;
	QAction *saveSmp_;
	QAction *playSmp_;																
	QAction *playsSmp_;
	QAction *stopSmp_;
	
	QAction *cutAct_;
	QAction *copyAct_;
	QAction *pasteAct_;
	QAction *selAct_;
	
	QAction *tselAct_;
	QAction *tzoomAct_;
	
	QToolBar *processBar_;
	QAction *ampEfx_;
	QAction *convEfx_;
	QAction *remdcEfx_;
	QAction *invEfx_;
	
	QToolBar *zoomBar_;
	QAction *zoomMore_;
	QSlider *zoomSlide_;
	QAction *zoomLess_;


	InstrumentsModel *instrumentsModel_;
};

#endif
