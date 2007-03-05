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

#include <QTreeWidget>
#include <QAction>
#include <QGridLayout>
#include <QToolBar>
#include <QLineEdit>
#include <QLabel>
#include <QComboBox>

 #include "patternbox.h"

 PatternBox::PatternBox(QWidget *parent) 
    : QWidget(parent)
 {
     createActions();

     QGridLayout *layout = new QGridLayout();
     layout->setAlignment( Qt::AlignTop );

    createToolbar();
    createPatternTree();
    createItemPropertiesBox();

    layout->addWidget( toolBar_, 0, 0);
    layout->addWidget( patternTree_, 1, 0, 2, 0 );
    layout->addWidget( itemProps_, 3, 0 );
    layout->setRowStretch(1, 10);
    layout->setRowStretch(3, 5);
    setLayout(layout);
 }

void PatternBox::createToolbar()
{
     toolBar_ = new QToolBar();
     toolBar_->addAction(newCatAct);
     toolBar_->addSeparator();
     toolBar_->addAction(newPatAct);
     toolBar_->addAction(clnPatAct);
     toolBar_->addAction(delPatAct);
     toolBar_->addSeparator();
     toolBar_->addAction(addPatToSeqAct);
}

void PatternBox::createPatternTree()
{
    patternTree_ = new QTreeWidget();
    patternTree_->setHeaderLabel( "Patterns" );

    QTreeWidgetItem *testCat0 = new QTreeWidgetItem( patternTree_ );
    testCat0->setForeground( 0, QBrush( Qt::white ) );
    testCat0->setBackground( 0, QBrush( Qt::red ) );
    testCat0->setText( 0, "Category0" );

    QTreeWidgetItem *testCat1 = new QTreeWidgetItem( patternTree_ );
    testCat1->setForeground( 0, QBrush( Qt::white ) );
    testCat1->setBackground( 0, QBrush( Qt::blue ) );
    testCat1->setText( 0, "Category1" );

    QTreeWidgetItem *testPattern0 = new QTreeWidgetItem( testCat0 );
    QTreeWidgetItem *testPattern1 = new QTreeWidgetItem( testCat1 );
    testPattern0->setText( 0, "Pattern0" );
    testPattern1->setText( 0, "Pattern1" );
}

void PatternBox::createItemPropertiesBox()
{
    itemProps_ = new QWidget( this );
    QGridLayout *itemPropsLayout = new QGridLayout();
    itemPropsLayout->setAlignment( Qt::AlignTop );
    itemProps_->setLayout( itemPropsLayout );
    //itemPropsLayout->addWidget( new QLabel( "Item Properties" ), 0, 0 );
    itemPropsLayout->addWidget( new QLabel( "Name:" ), 1, 0, 1, 1 );
    itemPropsLayout->addWidget( new QLineEdit(), 1, 1, 1, 3 );
    itemPropsLayout->addWidget( new QLabel( "Colour:" ), 2, 0, 2, 1 );
    itemPropsLayout->addWidget( new QComboBox(), 2, 1, 2, 3 );
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
