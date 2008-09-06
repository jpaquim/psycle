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
#include <psycle/core/song.h>
#include <psycle/core/singlepattern.h>

#include "patternbox.hpp"
#include "mainwindow.hpp"
#include <QTextCodec>
#include <QTreeWidget>
#include <QAction>
#include <QGridLayout>
#include <QToolBar>
#include <QLineEdit>
#include <QLabel>
#include <QComboBox>
#include <QColorDialog>
#include <QDebug>

#include <iostream> // std::cerr

namespace qpsycle {

	PatternBox::PatternBox( psy::core::Song *song, QWidget *parent ) 
		: QWidget(parent)
		, currentPattern_(NULL)
	{
		song_ = song;
		createActions();

		QGridLayout *layout = new QGridLayout();
		layout->setAlignment( Qt::AlignTop );
		setLayout(layout);

		createToolbar();
		patternTree_ = new PatternTree( this );
		patternTree_->setSelectionMode( QAbstractItemView::SingleSelection );
		patternTree_->setHeaderLabel( "Patterns" );
		connect( patternTree_, SIGNAL( currentItemChanged( QTreeWidgetItem*, QTreeWidgetItem* ) ), this, SLOT( currentItemChanged( QTreeWidgetItem*, QTreeWidgetItem* ) ) );
		connect( patternTree_, SIGNAL( itemChanged( QTreeWidgetItem*, int ) ), this, SLOT( onItemEdited( QTreeWidgetItem* ) ) );

		layout->addWidget( toolBar_, 0, 0);
		layout->addWidget( patternTree_, 1, 0, 2, 0 );
		layout->setRowStretch(1,10);
		layout->setRowStretch(3,5);
	}

	void PatternBox::createToolbar()
	{
		toolBar_ = new QToolBar( this );
		toolBar_->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed );
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

		std::vector<psy::core::PatternCategory*>::iterator it = song_->patternSequence().patternPool()->begin();
		for ( ; it < song_->patternSequence().patternPool()->end(); ++it) {
			psy::core::PatternCategory* category = *it;
			CategoryItem *categoryItem = new CategoryItem();
			patternTree()->addTopLevelItem( categoryItem );
			categoryItem->setText( 0, QString::fromStdString( category->name() ) );
			QColor col = QColorFromLongColor( category->color() );
			categoryItem->setBackground( 0, QBrush( col ) );
			categoryMap[categoryItem] = category;

			std::vector<psy::core::SinglePattern*>::iterator patIt = category->begin();
			for ( ; patIt < category->end(); patIt++) {
				PatternItem *patternItem = new PatternItem();
				categoryItem->addChild( patternItem );
				psy::core::SinglePattern *pattern = *patIt;
				patternItem->setText( 0, QString::fromStdString( pattern->name() ) );
				patternMap[patternItem] = pattern;
				if (isFirst) {
					isFirst = false;
					patternTree_->setCurrentItem( patternItem );
					QColor color = QColorFromLongColor( category->color() );
				}
			}
		}
	}

	void PatternBox::newCategory() 
	{ 
		psy::core::PatternCategory* category = song()->patternSequence().patternPool()->createNewCategory("New Category");
		long defaultColor = 0x29D6DE;
		category->setColor( defaultColor );

		CategoryItem* catItem = new CategoryItem();
		patternTree()->addTopLevelItem( catItem );
		patternTree()->setCurrentItem(catItem);
		catItem->setText( 0, "New Category" );
		QColor col = QColorFromLongColor( category->color() );
		catItem->setBackground( 0, QBrush( col ) );
		categoryMap[catItem] = category;
		catItems.push_back( catItem );
	
	}

	void PatternBox::newPattern() 
	{ 
		if ( patternTree()->currentItem() ) {
			QTreeWidgetItem *item = patternTree()->currentItem();

			CategoryItem *catItem = 0;
			if ( item->type() == QTreeWidgetItem::UserType + 1 )
			{
				PatternItem *currentPatItem = (PatternItem*)item;
				catItem = (CategoryItem*)currentPatItem->parent();
			} else if ( item->type() == QTreeWidgetItem::UserType + 2 )
			{
				catItem = (CategoryItem*)item;
			}

			std::map<CategoryItem*, psy::core::PatternCategory*>::iterator itr = categoryMap.find( catItem );
			if( itr != categoryMap.end() ) 
			{
				CategoryItem* catItem = itr->first;
				psy::core::PatternCategory* cat = itr->second;
				psy::core::SinglePattern* pattern = cat->createNewPattern("Pattern");
				QString patName = QString( "Pattern " + QString::number( pattern->id() ) );
				pattern->setName( patName.toStdString() );
				PatternItem *patItem = new PatternItem();

				catItem->addChild( patItem );
				patItem->setText( 0, QString::fromStdString( pattern->name() ) );
				//item->mouseDoublePress.connect(this,&PatternBox::onPatternItemDblClick);
				patternMap[patItem] = pattern;
				patternTree()->setCurrentItem( patItem );
			}
		}

	}

	void PatternBox::clonePattern() 
	{ 
		if ( patternTree()->currentItem() )
		{
			QTreeWidgetItem *item = patternTree()->currentItem();

			if ( item->type() == QTreeWidgetItem::UserType + 1 )
			{
				PatternItem *patItem = (PatternItem*)item;
				std::map<PatternItem*, psy::core::SinglePattern*>::iterator itr 
					= patternMap.find( patItem ); 

				if ( itr!=patternMap.end() ) 
				{
					psy::core::SinglePattern* pattern = itr->second;
					std::string clonedPatName = pattern->name()+" Clone";

					// Clone the pattern in the song.
					psy::core::SinglePattern* clonedPat = pattern->category()->clonePattern( *pattern, clonedPatName );

					// Add a clone item to the pattern tree.
					PatternItem* newItem = new PatternItem();
					newItem->setText( 0, QString::fromStdString( clonedPatName ) );
					CategoryItem *parentCat = (CategoryItem*)patItem->parent();
					// Record the pattern <-> patitem pairing.
					patternMap[newItem] = clonedPat; 
/* must happen before setCurrentItem,
otherwise no signal will be emitted
when the pattern selected changes.
*/
					parentCat->addChild( newItem );
					patternTree()->setCurrentItem( newItem );
				}
			}
		}
	}

	void PatternBox::onNewPatternCreated( psy::core::SinglePattern *newPattern )
	{
		// Add a clone item to the pattern tree.
		PatternItem* newItem = new PatternItem();
		newItem->setText( 0, QString::fromStdString( newPattern->name() ) );

		CategoryItem *parentCat = 0;
		std::map<CategoryItem*, psy::core::PatternCategory*>::const_iterator it;
		for ( it = categoryMap.begin(); it != categoryMap.end(); it++ )
		{
			CategoryItem *thisCat = (*it).first;
			qDebug() << categoryMap[thisCat];
			qDebug() << newPattern->category();
			if ( categoryMap[thisCat] == newPattern->category() ) {

parentCat = thisCat;
			}
		}

		// Record the pattern <-> patitem pairing.
		patternMap[newItem] = newPattern; 
/* must happen before setCurrentItem,
otherwise no signal will be emitted
when the pattern selected changes.
*/
		if ( parentCat != 0 ) {
			parentCat->addChild( newItem );
			patternTree()->setCurrentItem( newItem );
		}
	}

	void PatternBox::deletePattern() 
	{ 
		if ( patternTree()->currentItem() )
		{
			PatternItem* patItem = (PatternItem*)patternTree()->currentItem();
			std::map<PatternItem*, psy::core::SinglePattern*>::iterator patItr = patternMap.find( patItem );

			if ( patItr != patternMap.end() ) // only remove if it is a recognisable pattern item.
			{
				psy::core::SinglePattern* pattern = patItr->second;
				patternMap.erase( patItr );

				song()->patternSequence().removeSinglePattern( pattern );
				emit patternDeleted();

				CategoryItem* parentCatItem = (CategoryItem*)patItem->parent();
				int indexOfChild = parentCatItem->indexOfChild( patItem );
				parentCatItem->takeChild( indexOfChild );

				// FIXME: need some stuff here when seq gui is in place.
				// seqGui->removePattern(pattern);
				//  seqGui->repaint();
			}
		}

	}

	void PatternBox::addPatternToSequencer() 
	{ 
		QTreeWidgetItem* item = patternTree()->currentItem();
		if ( item ) {
			std::map<PatternItem*, psy::core::SinglePattern*>::iterator itr = patternMap.find( (PatternItem*)item );
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
		if(  prevItem != 0 ) {
			patternTree()->closePersistentEditor( prevItem, 0 ); // Closes the persistent editor if it's open.
		}
		if ( currItem != 0 ) { 
			// If new item is a pattern...
			if ( currItem->type() == QTreeWidgetItem::UserType + 1 )
			{
				std::map<PatternItem*, psy::core::SinglePattern*>::iterator patItr = patternMap.find( (PatternItem*)currItem );
				if( patItr != patternMap.end() ) {
					psy::core::SinglePattern *pattern = patItr->second;
					currentPattern_ = pattern;
					// emit a signal for main window to tell pat view.
					emit patternSelectedInPatternBox( pattern );
				}
				else {
					std::cerr << "Warning: " << __FILE__ << ": internal error on line" << __LINE__ << ": An unknown pattern item was selected." << std::endl;
				}
			}

			// If new item is a category...
			if ( currItem->type() == QTreeWidgetItem::UserType + 2 )
			{
				std::map<CategoryItem*, psy::core::PatternCategory*>::iterator catItr = categoryMap.find( (CategoryItem*)currItem );
				if( catItr !=categoryMap.end() ) {
					//psy::core::PatternCategory *category = catItr->second;
					// FIXME: this needs to do something?
				}
				else {
					std::cerr << "Warning: " << __FILE__ << ": internal error on line" << __LINE__ << ": An unknown category item was selected." << std::endl;
				}
			}
		}
	}
	void PatternBox::onPatternNameEdited( const QString & newText )
	{
		QTreeWidgetItem *item = patternTree_->currentItem();
		QTextCodec::setCodecForCStrings(QTextCodec::codecForLocale());
		// If current item is a pattern...
		std::map<PatternItem*, psy::core::SinglePattern*>::iterator itr = patternMap.find( (PatternItem*)item );
		if( itr!=patternMap.end() ) {
			psy::core::SinglePattern *pattern = itr->second;
			item->setText( 0, newText );
			pattern->setName( newText.toStdString() );
			emit patternNameChanged();
			return;
		}

		// If current item is a category...
		std::map<CategoryItem*, psy::core::PatternCategory*>::iterator catItr = categoryMap.find( (CategoryItem*)item );
		if( catItr!=categoryMap.end() ) {
			psy::core::PatternCategory *category = catItr->second;
			item->setText( 0, newText );
			category->setName( newText.toStdString() );
			return;
		}
	}

// FIXME: this is duplicated in SequencerItem.
	const QColor PatternBox::QColorFromLongColor( long longCol )
	{
		unsigned int r, g, b;
		b = (longCol>>16) & 0xff;
		g = (longCol>>8 ) & 0xff;
		r = (longCol    ) & 0xff;

		return  QColor( r, g, b );
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

	void PatternBox::onItemEdited( QTreeWidgetItem *item )
	{
		// If current item is a pattern...
		if ( item->type() == QTreeWidgetItem::UserType + 1 )
		{
			std::map<PatternItem*, psy::core::SinglePattern*>::iterator itr = patternMap.find( (PatternItem*)item );
			if( itr!=patternMap.end() ) {
				psy::core::SinglePattern *pattern = itr->second;
				pattern->setName( item->text( 0 ).toStdString() );
				emit patternNameChanged();
				return;
			}
		}

		if ( item->type() == QTreeWidgetItem::UserType + 2 )
		{
			// If current item is a category...
			std::map<CategoryItem*, psy::core::PatternCategory*>::iterator catItr = categoryMap.find( (CategoryItem*)item );
			if( catItr!=categoryMap.end() ) {
				psy::core::PatternCategory *category = catItr->second;
				category->setName( item->text( 0 ).toStdString() );
				return;
			}
		}

	}

	PatternItem::PatternItem() : QTreeWidgetItem( QTreeWidgetItem::UserType + 1 )
	{
		setFlags( Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled );
	}

	CategoryItem::CategoryItem() : QTreeWidgetItem( QTreeWidgetItem::UserType + 2 )
	{
		setFlags( Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled );
	}


	PatternTree::PatternTree( PatternBox *patBox ) 
		: QTreeWidget( patBox )
	{ 
		patBox_ = patBox;
		editPatNameAct_ = new QAction( "Edit pattern name", this );
		editCatNameAct_ = new QAction( "Edit category name", this );
		editCatColorAct_ = new QAction( "Edit color", this );

		connect( editPatNameAct_, SIGNAL( triggered() ), this, SLOT( onEditPatternNameActionTriggered() ) );
		connect( editCatNameAct_, SIGNAL( triggered() ), this, SLOT( onEditCategoryNameActionTriggered() ) );
		connect( editCatColorAct_, SIGNAL( triggered() ), patBox_, SLOT( onEditCategoryColorActionTriggered() ) );
	}

	void PatternTree::contextMenuEvent( QContextMenuEvent *ev )
	{
		QTreeWidgetItem *item = this->itemAt( ev->pos() );
		if ( item )
		{
			QMenu menu;
			if ( item->type() == QTreeWidgetItem::UserType + 1 )
			{
				menu.addAction( editPatNameAct_ );
			}
			if ( item->type() == QTreeWidgetItem::UserType + 2 )
			{
				menu.addAction( editCatNameAct_ );
				menu.addAction( editCatColorAct_ ); 
			}
			menu.exec( ev->globalPos() );
		}
	}

	void PatternTree::onEditPatternNameActionTriggered()
	{
		PatternItem *patItem = (PatternItem*)currentItem();
		openPersistentEditor( patItem, 0 );
	}

	void PatternTree::onEditCategoryNameActionTriggered()
	{
		CategoryItem *catItem = (CategoryItem*)currentItem();
		openPersistentEditor( catItem, 0 );
	}

	void PatternBox::onEditCategoryColorActionTriggered()
	{  
		QColor color = QColorDialog::getColor();
		if ( color.isValid() )
		{
			CategoryItem *item = (CategoryItem*)patternTree_->currentItem();
			std::map<CategoryItem*, psy::core::PatternCategory*>::iterator itr = categoryMap.find( (CategoryItem*) item );
			if( itr != categoryMap.end() ) 
			{
				psy::core::PatternCategory *category = itr->second;
				category->setColor( QColorToLongColor( color ) );
				item->setBackground( 0, QBrush( color ) );
			}
			emit categoryColorChanged();
		}
	}

	bool PatternBox::event( QEvent *event )
	{
		switch (event->type()) {
		case QEvent::KeyPress: {
			QKeyEvent *k = (QKeyEvent *)event;
			if (
				k->key() == Qt::Key_1 || k->key() == Qt::Key_2
				|| k->key() == Qt::Key_3 || k->key() == Qt::Key_4 )
			{
				return true;
			} else {
				QWidget::keyPressEvent(k);
				return true;
			}
		}
		default:
			return QWidget::event( event );
		}
	}

}
