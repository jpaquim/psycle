/***************************************************************************
*   Copyright (C) 2006 by  Stefan   *
*   natti@linux   *
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

#include <QtGui>

 #include "patternbox.h"

 PatternBox::PatternBox(QWidget *parent) 
    : QWidget(parent)
 {
     createActions();

     QGridLayout *layout = new QGridLayout();

     patToolBar = new QToolBar();
     patToolBar->addAction(newCatAct);
     patToolBar->addSeparator();
     patToolBar->addAction(newPatAct);
     patToolBar->addAction(clnPatAct);
     patToolBar->addAction(delPatAct);
     patToolBar->addSeparator();
     patToolBar->addAction(addPatToSeqAct);

     QTreeView *patBox = new QTreeView();

     layout->addWidget(patToolBar, 0, 0);
     layout->addWidget(patBox, 1, 0, 2, 0);
     layout->setRowStretch(1, 10);
     setLayout(layout);
 }

 void PatternBox::newCategory() { }
 void PatternBox::newPattern() { }
 void PatternBox::clonePattern() { }
 void PatternBox::deletePattern() { }
 void PatternBox::addPatternToSequencer() { }

 void PatternBox::createActions()
 {
     newCatAct = new QAction(QIcon(":/images/pb_newcat.png"), tr("New Category"), this);
     newCatAct->setStatusTip(tr("Create a new category"));
     connect(newCatAct, SIGNAL(triggered()), this, SLOT(newCategory()));

     newPatAct = new QAction(QIcon(":/images/pb_newpat.png"), tr("New Pattern"), this);
     newPatAct->setStatusTip(tr("Create a new pattern"));
     connect(newPatAct, SIGNAL(triggered()), this, SLOT(newPattern()));

     clnPatAct = new QAction(QIcon(":/images/pb_clnpat.png"), tr("Clone Pattern"), this);
     clnPatAct->setStatusTip(tr("Clone selected pattern"));
     connect(clnPatAct, SIGNAL(triggered()), this, SLOT(clonePattern()));

     delPatAct = new QAction(QIcon(":/images/pb_delpat.png"), tr("Delete Pattern"), this);
     delPatAct->setStatusTip(tr("Delete selected pattern"));
     connect(delPatAct, SIGNAL(triggered()), this, SLOT(deletePattern()));

     addPatToSeqAct = new QAction(QIcon(":/images/pb_addpattoseq.png"), tr("Add Pattern To Sequencer"), this);
     addPatToSeqAct->setStatusTip(tr("Add selected pattern to sequencer"));
     connect(addPatToSeqAct, SIGNAL(triggered()), this, SLOT(addPatternToSequencer()));
 }
