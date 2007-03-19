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
#include <QColorDialog>

 #include "patternbox.h"
 #include "psycore/singlepattern.h"

 PatternBox::PatternBox( psy::core::Song *song, QWidget *parent ) 
    : QWidget(parent)
 {
    song_ = song;
     createActions();

     QGridLayout *layout = new QGridLayout();
     layout->setAlignment( Qt::AlignTop );

    createToolbar();
    patternTree_ = new QTreeWidget();
    patternTree_->setSelectionMode( QAbstractItemView::SingleSelection );
    patternTree_->setHeaderLabel( "Patterns" );
    connect( patternTree_, SIGNAL( currentItemChanged( QTreeWidgetItem*, QTreeWidgetItem* ) ), 
             this, SLOT( currentItemChanged( QTreeWidgetItem*, QTreeWidgetItem* ) ) );
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

void PatternBox::populatePatternTree()
{
    categoryMap.clear();
    catItems.clear();
    patternMap.clear();

    bool isFirst = true;

    std::vector<psy::core::PatternCategory*>::iterator it = song_->patternSequence()->patternData()->begin();
    for ( ; it < song_->patternSequence()->patternData()->end(); ++it) {
        psy::core::PatternCategory* category = *it;
        QTreeWidgetItem *categoryItem = new QTreeWidgetItem( patternTree_ );
        categoryItem->setText( 0, QString::fromStdString( category->name() ) );
        QColor col = QColorFromLongColor( category->color() );
        categoryItem->setBackground( 0, QBrush( col ) );
        categoryMap[categoryItem] = category;

        std::vector<psy::core::SinglePattern*>::iterator patIt = category->begin();
        for ( ; patIt < category->end(); patIt++) {
            QTreeWidgetItem *patternItem = new QTreeWidgetItem( categoryItem );
			psy::core::SinglePattern *pattern = *patIt;
			patternItem->setText( 0, QString::fromStdString( pattern->name() ) );
            patternMap[patternItem] = pattern;
            if (isFirst) {
                isFirst = false;
                patternTree_->setCurrentItem( patternItem );
                QColor color = QColorFromLongColor( category->color() );
                // Change the colour of the colorBtn's background.
                QPalette pal = colorBtn_->palette();
                pal.setColor( QPalette::Button, color );
                colorBtn_->setPalette( pal );
            }
        }
    }
}

void PatternBox::createItemPropertiesBox()
{
    itemProps_ = new QWidget( this );
    QGridLayout *itemPropsLayout = new QGridLayout();
    itemPropsLayout->setAlignment( Qt::AlignTop );
    itemProps_->setLayout( itemPropsLayout );
    //itemPropsLayout->addWidget( new QLabel( "Item Properties" ), 0, 0 );
    itemPropsLayout->addWidget( new QLabel( "Name:" ), 1, 0, 1, 1 );
    nameEdit_ = new QLineEdit();
    connect( nameEdit_, SIGNAL( textChanged( const QString & ) ),
             this, SLOT( onPatternNameEdited( const QString & ) ) );
    itemPropsLayout->addWidget( nameEdit_, 1, 1, 1, 3 );
    itemPropsLayout->addWidget( new QLabel( "Colour:" ), 2, 0, 2, 1 );
    colorBtn_ = new QPushButton();
    colorBtn_->setAutoFillBackground( true );
    connect( colorBtn_, SIGNAL( clicked() ), this, SLOT( onColorButtonClicked() ) );
    itemPropsLayout->addWidget( colorBtn_, 2, 1, 2, 3 );
}

void PatternBox::newCategory() 
{ 
    psy::core::PatternCategory* category = song()->patternSequence()->patternData()->createNewCategory("category");
    long defaultColor = 0x0000FF;
    category->setColor( defaultColor );


    QTreeWidgetItem* catItem = new QTreeWidgetItem( patternTree() );
    catItem->setText( 0, "Category" );
    QColor col = QColorFromLongColor( category->color() );
    catItem->setBackground( 0, QBrush( col ) );
    categoryMap[catItem] = category;
    catItems.push_back( catItem );
}

void PatternBox::newPattern() 
{ 
    if ( patternTree()->currentItem() ) {
        QTreeWidgetItem *item = patternTree()->currentItem();

        std::map<QTreeWidgetItem*, psy::core::PatternCategory*>::iterator itr = categoryMap.find( item );
        if( itr != categoryMap.end() ) 
        {
            QTreeWidgetItem* catItem = itr->first;
            psy::core::PatternCategory* cat = itr->second;
            psy::core::SinglePattern* pattern = cat->createNewPattern("Pattern");
            QString patName = QString( "Pattern" + QString::number( pattern->id() ) );
            pattern->setName( patName.toStdString() );
            QTreeWidgetItem *patItem = new QTreeWidgetItem( catItem );
            patItem->setText( 0, QString::fromStdString( pattern->name() ) );
            //item->mouseDoublePress.connect(this,&PatternBox::onPatternItemDblClick);
            patternMap[patItem] = pattern;
        }
    }
}

void PatternBox::clonePattern() { }

void PatternBox::deletePattern() 
{ 
    QTreeWidgetItem* patItem = patternTree()->currentItem();
    std::map<QTreeWidgetItem*, psy::core::SinglePattern*>::iterator patItr = patternMap.find( patItem );

    if ( patItr != patternMap.end() ) // only remove if it is a recognisable pattern item.
    {
        psy::core::SinglePattern* pattern = patItr->second;
        patternMap.erase( patItr );

        song()->patternSequence()->removeSinglePattern( pattern );
        emit patternDeleted();

        QTreeWidgetItem* parentCatItem = patItem->parent();
        int indexOfChild = parentCatItem->indexOfChild( patItem );
        parentCatItem->takeChild( indexOfChild );

// FIXME: need some stuff here when seq gui is in place.
/*        seqGui->removePattern(pattern);
        seqGui->repaint();*/
    }
}

void PatternBox::addPatternToSequencer() 
{ 
    QTreeWidgetItem* item = patternTree()->currentItem();
    if ( item ) {
        std::map<QTreeWidgetItem*, psy::core::SinglePattern*>::iterator itr = patternMap.find( item );
        if ( itr!=patternMap.end() ) {
            psy::core::SinglePattern *pattern = itr->second;
            emit addPatternToSequencerRequest( pattern );
        }
    }
}


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

void PatternBox::currentItemChanged( QTreeWidgetItem *currItem, QTreeWidgetItem *prevItem )
{
    // If new item is a pattern...
    std::map<QTreeWidgetItem*, psy::core::SinglePattern*>::iterator patItr = patternMap.find( currItem );
    if( patItr != patternMap.end() ) {
        psy::core::SinglePattern *pattern = patItr->second;
        nameEdit_->setText( QString::fromStdString( pattern->name() ) );
        // emit a signal for main window to tell pat view.
        emit patternSelectedInPatternBox( pattern );
        return;
    }

    // If new item is a category...
    std::map<QTreeWidgetItem*, psy::core::PatternCategory*>::iterator catItr = categoryMap.find( currItem );
    if( catItr !=categoryMap.end() ) {
        psy::core::PatternCategory *category = catItr->second;
        nameEdit_->setText( QString::fromStdString( category->name() ) );
        // emit a signal for main window to tell pat view.
        // Change the colour of the colorBtn's background.
        QColor color = QColorFromLongColor( category->color() );
        QPalette pal = colorBtn_->palette();
        pal.setColor( QPalette::Button, color );
        colorBtn_->setPalette( pal );
    }
}

void PatternBox::onPatternNameEdited( const QString & newText )
{
    QTreeWidgetItem *item = patternTree_->currentItem();

    // If current item is a pattern...
    std::map<QTreeWidgetItem*, psy::core::SinglePattern*>::iterator itr = patternMap.find(item);
    if( itr!=patternMap.end() ) {
        psy::core::SinglePattern *pattern = itr->second;
        item->setText( 0, newText );
        pattern->setName( newText.toStdString() );
        emit patternNameChanged();
        return;
    }

    // If current item is a category...
    std::map<QTreeWidgetItem*, psy::core::PatternCategory*>::iterator catItr = categoryMap.find(item);
    if( catItr!=categoryMap.end() ) {
        psy::core::PatternCategory *category = catItr->second;
        item->setText( 0, newText );
        category->setName( newText.toStdString() );
        return;
    }
}

// FIXME: this is duplicated in SequencerItem.
const QColor & PatternBox::QColorFromLongColor( long longCol )
{
    unsigned int r, g, b;
    b = (longCol>>16) & 0xff;
    g = (longCol>>8 ) & 0xff;
    r = (longCol    ) & 0xff;

    return QColor( r, g, b );
}

long PatternBox::QColorToLongColor( const QColor & qCol )
{
    unsigned int r, g, b;
    r = qCol.red();
    g = qCol.green();
    b = qCol.blue();
    unsigned long longCol = (b << 16) | (g << 8) | (r); // FIXME: BGR??
    

    return longCol;
}

void PatternBox::onColorButtonClicked()
{  
    QColor color = QColorDialog::getColor();
    QTreeWidgetItem *item = patternTree_->currentItem();
    std::map<QTreeWidgetItem*, psy::core::PatternCategory*>::iterator itr = categoryMap.find( item );
    if( itr != categoryMap.end() ) 
    {
        psy::core::PatternCategory *category = itr->second;
        category->setColor( QColorToLongColor( color ) );
        item->setBackground( 0, QBrush( color ) );
        // Change the colour of the colorBtn's background.
        QPalette pal = colorBtn_->palette();
        pal.setColor( QPalette::Button, color );
        colorBtn_->setPalette( pal );
        emit categoryColorChanged();
    }
}
