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

	PatternBox::PatternBox( psycle::core::Song *song, QWidget *parent ) 
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
		catItems.clear();
		patternMap.clear();

		bool isFirst = true;

		psycle::core::Sequence::patterniterator it = song_->patternSequence().patternbegin();
		std::map<std::string, CategoryItem *> categoryMap;
		for ( ; it < song_->patternSequence().patternend(); ++it) {
		  psycle::core::Pattern *pattern = *it;
		  const std::string& categoryName = pattern->category();
		  CategoryItem *categoryItem = categoryMap[categoryName];
		  if (categoryItem == NULL) {
		    categoryItem = new CategoryItem();
		    categoryItem->name(categoryName);
		    categoryMap[categoryName] = categoryItem;
		    catItems.push_back(categoryItem);
		    patternTree()->addTopLevelItem( categoryItem );
		    categoryItem->setText(0, QString::fromStdString(categoryName));
		    //QColor col = QColorFromLongColor( category->color() );
		    //categoryItem->setBackground( 0, QBrush( col ) );
		  }

		  PatternItem *patternItem = new PatternItem();
		  categoryItem->addChild( patternItem );
		  patternItem->setText( 0, QString::fromStdString( pattern->name() ) );
		  patternMap[patternItem] = pattern;
		  if (isFirst) {
		    isFirst = false;
		    patternTree_->setCurrentItem( patternItem );
		    //QColor color = QColorFromLongColor( category->color() );
		  }
		}
	}

	void PatternBox::newCategory() 
	{ 
	  const char* categoryName = "New Category";
	  //long defaultColor = 0x29D6DE;
	  //category->setColor( defaultColor );

		CategoryItem* catItem = new CategoryItem();
		catItem->name("New Category");
		patternTree()->addTopLevelItem( catItem );
		patternTree()->setCurrentItem(catItem);
		catItem->setText( 0, "New Category" );
		//QColor col = QColorFromLongColor( category->color() );
		//catItem->setBackground( 0, QBrush( col ) );
		//categoryMap[catItem] = category;
		catItems.push_back( catItem );
	
	}

	void PatternBox::newPattern() 
	{ 
		if ( patternTree()->currentItem() ) {
			QTreeWidgetItem *item = patternTree()->currentItem();

			CategoryItem *catItem = 0;
			if ( item->type() == PatternItem::Type)
			{
				PatternItem *currentPatItem = (PatternItem*)item;
				catItem = (CategoryItem*)currentPatItem->parent();
			} else if ( item->type() == CategoryItem::Type )
			{
				catItem = (CategoryItem*)item;
			}

			psycle::core::Pattern* pattern = new psycle::core::Pattern();
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

	void PatternBox::clonePattern() 
	{ 
		if ( patternTree()->currentItem() )
		{
			QTreeWidgetItem *item = patternTree()->currentItem();

			if ( item->type() == QTreeWidgetItem::UserType + 1 )
			{
				PatternItem *patItem = (PatternItem*)item;
				std::map<PatternItem*, psycle::core::Pattern*>::iterator itr 
					= patternMap.find( patItem ); 

				if ( itr!=patternMap.end() ) 
				{
					psycle::core::Pattern* pattern = itr->second;
					std::string clonedPatName = pattern->name()+" Clone";

					// Clone the pattern in the song.
					psycle::core::Pattern* clonedPat = new psycle::core::Pattern(*pattern);

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

	void PatternBox::onNewPatternCreated( psycle::core::Pattern *newPattern )
	{
		// Add a clone item to the pattern tree.
		PatternItem* newItem = new PatternItem();
		newItem->setText( 0, QString::fromStdString( newPattern->name() ) );

		CategoryItem *parentCat = 0;
		std::vector<CategoryItem*>::const_iterator it;
		for ( it = catItems.begin(); it != catItems.end(); it++ )
		{
		  CategoryItem *thisCat = *it;
		  if ( thisCat->name() == newPattern->category() ) {

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
			std::map<PatternItem*, psycle::core::Pattern*>::iterator patItr = patternMap.find( patItem );

			if ( patItr != patternMap.end() ) // only remove if it is a recognisable pattern item.
			{
				psycle::core::Pattern* pattern = patItr->second;
				patternMap.erase( patItr );

				song()->patternSequence().removePattern( pattern );
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
			std::map<PatternItem*, psycle::core::Pattern*>::iterator itr = patternMap.find( (PatternItem*)item );
			if ( itr!=patternMap.end() ) {
				psycle::core::Pattern *pattern = itr->second;
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
		  if ( currItem->type() == PatternItem::Type)
			{
				std::map<PatternItem*, psycle::core::Pattern*>::iterator patItr = patternMap.find( (PatternItem*)currItem );
				if( patItr != patternMap.end() ) {
					psycle::core::Pattern *pattern = patItr->second;
					currentPattern_ = pattern;
					// emit a signal for main window to tell pat view.
					emit patternSelectedInPatternBox( pattern );
				}
				else {
					std::cerr << "Warning: " << __FILE__ << ": internal error on line" << __LINE__ << ": An unknown pattern item was selected." << std::endl;
				}
			}

			// If new item is a category...
			if ( currItem->type() == CategoryItem::Type)
			{
			  // FIXME: this needs to do something?
			}
		}
	}
	void PatternBox::onPatternNameEdited( const QString & newText )
	{
		QTreeWidgetItem *item = patternTree_->currentItem();
		QTextCodec::setCodecForCStrings(QTextCodec::codecForLocale());
		// If current item is a pattern...
		if (item->type() == PatternItem::Type) {
		  std::map<PatternItem*, psycle::core::Pattern*>::iterator itr = patternMap.find( (PatternItem*)item );
		  if( itr!=patternMap.end() ) {
		    psycle::core::Pattern *pattern = itr->second;
		    item->setText( 0, newText );
		    pattern->setName( newText.toStdString() );
		    emit patternNameChanged();
		    return;
		  }
		}

		if (item->type() == CategoryItem::Type) {
		  CategoryItem *categoryItem = (CategoryItem *)item;
		  std::string categoryName(newText.toStdString());

		  categoryItem->setText( 0, newText );
		  categoryItem->name(categoryName);

		  // Set category name on child patterns as well.
		  for (int ix = 0; ix < categoryItem->childCount(); ++ix) {
		    QTreeWidgetItem *childItem = categoryItem->child(ix);
		    if (childItem->type() == PatternItem::Type) {
		      PatternItem *patternItem = (PatternItem *)childItem;
		      patternMap[patternItem]->setCategory(categoryName);
		    } else {
		      qDebug() << "Child of CategoryItem is not a PatternItem??";
		    }
		  }
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
	  if ( item->type() == PatternItem::Type)
		{
			std::map<PatternItem*, psycle::core::Pattern*>::iterator itr = patternMap.find( (PatternItem*)item );
			if( itr!=patternMap.end() ) {
				psycle::core::Pattern *pattern = itr->second;
				pattern->setName( item->text( 0 ).toStdString() );
				emit patternNameChanged();
				return;
			}
		}

	  if ( item->type() == CategoryItem::Type)
		{
		  CategoryItem *categoryItem = (CategoryItem *)item;
		  const std::string &categoryName = categoryItem->name();

		  // Set category name on child patterns as well.
		  for (int ix = 0; ix < categoryItem->childCount(); ++ix) {
		    QTreeWidgetItem *childItem = categoryItem->child(ix);
		    if (childItem->type() == PatternItem::Type) {
		      PatternItem *patternItem = (PatternItem *)childItem;
		      patternMap[patternItem]->setCategory(categoryName);
		    } else {
		      qDebug() << "Child of CategoryItem is not a PatternItem??";
		    }
		  }

		  return;
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
			/*
			std::map<CategoryItem*, psycle::core::PatternCategory*>::iterator itr = categoryMap.find( (CategoryItem*) item );
			if( itr != categoryMap.end() ) 
			{
				psycle::core::PatternCategory *category = itr->second;
				category->setColor( QColorToLongColor( color ) );
			}
			*/
			item->setBackground( 0, QBrush( color ) );
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
