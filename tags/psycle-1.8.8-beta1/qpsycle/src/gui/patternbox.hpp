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

namespace psycle { namespace core {
class Song;
class Pattern;
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

	  const std::string& name() { return name_; }
	  void name(const std::string& name) { name_ = name;} 
	private:
	  std::string name_;
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
		PatternBox( psycle::core::Song *song, QWidget *parent = 0);

		void populatePatternTree();
		PatternTree* patternTree() const { return patternTree_; }
		psycle::core::Song* song() const { return song_; }
		psycle::core::Pattern* currentPattern() const { return currentPattern_; }

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
		void onNewPatternCreated( psycle::core::Pattern *newPattern );

	protected:
		bool event( QEvent *event );

	signals:
		void patternSelectedInPatternBox( psycle::core::Pattern *selectedPattern );
		void patternDeleted();
		void addPatternToSequencerRequest( psycle::core::Pattern *selectedPattern );
		void patternNameChanged();
		void categoryColorChanged();

	private:
		void createActions();
		void createToolbar();
		void createItemPropertiesBox();
		const QColor QColorFromLongColor( long longCol );
		long QColorToLongColor( const QColor & qCol );

		psycle::core::Song *song_;
		psycle::core::Pattern* currentPattern_;

		std::vector<CategoryItem*> catItems;

	  // TODO put this pointer in PatternItem instead?
		std::map<PatternItem*, psycle::core::Pattern*> patternMap;

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
