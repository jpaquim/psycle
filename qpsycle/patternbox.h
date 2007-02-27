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

 #include <QWidget>

 class QToolBar;

 class PatternBox : public QWidget
 {
     Q_OBJECT

 public:
     PatternBox(QWidget *parent = 0);

 private slots:
     void newCategory();
     void newPattern();
     void clonePattern();
     void deletePattern();
     void addPatternToSequencer();

 private:
     void createActions();

     QToolBar *patToolBar;

     QAction *newCatAct;
     QAction *newPatAct;
     QAction *clnPatAct;
     QAction *delPatAct;
     QAction *addPatToSeqAct;

 };

 #endif
