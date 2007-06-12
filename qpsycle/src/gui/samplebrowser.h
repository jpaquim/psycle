/***************************************************************************
*   Copyright (C) 2007 by Psycledelics Community   *
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

#ifndef SAMPLEBROWSER_H
#define SAMPLEBROWSER_H

namespace psy { namespace core {
		class Song;
	}}

#include <QWidget>

class QPushButton;
class QStandardItemModel;
class QListView;
class QDirModel;
class QTreeView;

class SampleBrowser : public QWidget {
Q_OBJECT
public:
	SampleBrowser( psy::core::Song *song, QWidget *parent = 0 );
	~SampleBrowser();

	psy::core::Song *song() { return song_; }

public slots:
	void onAddToLoadedSamples();
	void onClearInstrument();

signals:
	void sampleAdded();

private:
	void createLoadedSamplesList();
	void createActionsWidget();
	void createSampleBrowserTree();

	QStandardItemModel *loadedSamplesModel_;
	QWidget *actionsWidget_;
	QListView *loadedSamplesView_;

	QDirModel *dirModel_;
	QTreeView *dirTree_;

	QPushButton *button_addToLoadedSamples;
	QPushButton *button_clearInstrument;

	psy::core::Song *song_;

};

#endif
