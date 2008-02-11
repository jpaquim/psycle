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
#ifndef PATTERNBOX_H
#define PATTERNBOX_H

namespace psy { namespace core {
class Song;
class SinglePattern;
class PatternCategory;
}}


class QToolBar;
#include <QAction>
#include <QToolBar>
#include <QTreeWidget>
#include <QPushButton>
#include <QLabel>
#include <QContextMenuEvent>
#include <QMenu>

namespace qpsycle {

class PatternBox;

class PatternTree : public QTreeWidget {
Q_OBJECT
	public:
	PatternTree( PatternBox *patBox );
protected:
	void contextMenuEvent( QContextMenuEvent *ev );

	public slots:
	void onEditPatternNameActionTriggered();
	void onEditCategoryNameActionTriggered();

private:
	PatternBox *patBox_;
	QAction *editPatNameAct_;
	QAction *editCatNameAct_;
	QAction *editCatColorAct_;
};

class CategoryItem : public QTreeWidgetItem {
public:
	CategoryItem();
	CategoryItem( PatternTree *patTree );

	enum { Type = QTreeWidgetItem::UserType + 2 };
	int type() const { return Type; }
};

class PatternItem : public QTreeWidgetItem {
public:
	PatternItem();
	PatternItem( CategoryItem *parent );

	enum { Type = QTreeWidgetItem::UserType + 1 };
	int type() const { return Type; }
};


class PatternBox : public QWidget {
Q_OBJECT
public:
	PatternBox( psy::core::Song *song, QWidget *parent = 0);

	void populatePatternTree();
	PatternTree* patternTree() { return patternTree_; }
	psy::core::Song* song() { return song_; }
	psy::core::SinglePattern* currentPattern() { return currentPattern_; }

public slots:
	void currentItemChanged( QTreeWidgetItem *currItem, QTreeWidgetItem *prevItem );
	void onItemEdited( QTreeWidgetItem *item );
	void newCategory();
	void newPattern();
	void clonePattern();
	void deletePattern();
	void addPatternToSequencer();
	void onPatternNameEdited( const QString & newText );
	void onEditCategoryColorActionTriggered();

protected:
	bool event( QEvent *event );

signals:
	void patternSelectedInPatternBox( psy::core::SinglePattern *selectedPattern );
	void patternDeleted();
	void addPatternToSequencerRequest( psy::core::SinglePattern *selectedPattern );
	void patternNameChanged();
	void categoryColorChanged();

private:
	void createActions();
	void createToolbar();
	void createItemPropertiesBox();
	const QColor QColorFromLongColor( long longCol );
	long QColorToLongColor( const QColor & qCol );

	psy::core::Song *song_;
	psy::core::SinglePattern* currentPattern_;
	std::map<CategoryItem*, psy::core::PatternCategory*> categoryMap;
	std::vector<CategoryItem*> catItems;
	std::map<PatternItem*, psy::core::SinglePattern*> patternMap;

	QToolBar *toolBar_;
	PatternTree *patternTree_;

	QAction *newCatAct;
	QAction *newPatAct;
	QAction *clnPatAct;
	QAction *delPatAct;
	QAction *addPatToSeqAct;

};

} // namespace qpsycle

#endif
