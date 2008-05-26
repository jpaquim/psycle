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
	#ifndef SEQUENCERVIEW_H
	#define SEQUENCERVIEW_H

namespace psy { namespace core {
class Song;
class SinglePattern;
}}

#include <QtGui/QVBoxLayout>
class QToolBar;
class QCheckBox;

namespace qpsycle {

class SequencerDraw;
class SequencerLine;

class SequencerView : public QWidget
{
Q_OBJECT
public:
	SequencerView( psy::core::Song *song );

	psy::core::Song* song() const { return song_; }
	SequencerLine *selectedLine() const;
	void setSelectedLine( SequencerLine *line ) ;
	void addPattern( psy::core::SinglePattern *pattern );
	SequencerDraw *sequencerDraw() const { return seqDraw_; }
	void updatePlayPos();
	void onPatternNameChanged();
	void onCategoryColorChanged();

	bool gridSnap();

public slots:
	void zoomIn();
	void zoomOut();
	
protected:

private:
	void setupToolbar();

	psy::core::Song *song_;
	QVBoxLayout *layout_;
	SequencerDraw *seqDraw_;

	SequencerLine *selectedLine_;
	std::vector<SequencerLine*> *lines_;

	QToolBar *toolBar_;
	QCheckBox *snapCheckbox_;


	double oldPlayPos_;
};

} // namespace qpsycle

#endif
