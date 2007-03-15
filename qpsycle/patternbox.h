/***************************************************************************
*   Copyright (C) 2006 by  Neil Mather   *
*   nmather@sourceforge   *
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

 #include <QAction>
 #include <QToolBar>
 #include <QTreeWidget>

 #include "psycore/song.h"

 class QToolBar;

 class PatternBox : public QWidget
 {
     Q_OBJECT

 public:
     PatternBox( psy::core::Song *song, QWidget *parent = 0);

    void populatePatternTree();
    QTreeWidget* patternTree() { return patternTree_; }
    psy::core::Song* song() { return song_; }

public slots:
     void currentItemChanged( QTreeWidgetItem *currItem, QTreeWidgetItem *prevItem );

 private slots:
     void newCategory();
     void newPattern();
     void clonePattern();
     void deletePattern();
     void addPatternToSequencer();
     void onPatternNameEdited( const QString & newText );

signals:
    void patternSelectedInPatternBox( psy::core::SinglePattern *selectedPattern );
    void patternDeleted();
    void addPatternToSequencerRequest( psy::core::SinglePattern *selectedPattern );
    void patternNameChanged();

 private:
    void createActions();
    void createToolbar();
    void createItemPropertiesBox();

    psy::core::Song *song_;
    std::map<QTreeWidgetItem*, psy::core::PatternCategory*> categoryMap;
    std::vector<QTreeWidgetItem*> catItems;
    std::map<QTreeWidgetItem*, psy::core::SinglePattern*> patternMap;

     QToolBar *toolBar_;
     QWidget *itemProps_;
     QLineEdit *nameEdit_;
     QTreeWidget *patternTree_;

     QAction *newCatAct;
     QAction *newPatAct;
     QAction *clnPatAct;
     QAction *delPatAct;
     QAction *addPatToSeqAct;

 };

 #endif
