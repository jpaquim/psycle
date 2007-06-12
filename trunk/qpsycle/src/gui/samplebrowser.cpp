/***************************************************************************
*   Copyright (C) 2007 by Psycledelics Community   *
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

#include <psycle/core/constants.h>
#include <psycle/core/song.h>

#include "global.h"
#include "configuration.h"
#include "samplebrowser.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QTreeView>
#include <QDirModel>
#include <QListView>
#include <QPushButton>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QItemSelectionModel>
#include <QModelIndexList>

#include <iostream>
#include <iomanip>


SampleBrowser::SampleBrowser( psy::core::Song *song, QWidget *parent )
	: QWidget( parent ),
	  song_( song )
{
	QHBoxLayout *layout = new QHBoxLayout();
	setLayout( layout );

	createLoadedSamplesList();
	createActionsWidget();
	createSampleBrowserTree();

	layout->addWidget( loadedSamplesView_ );
	layout->addWidget( actionsWidget_ );
	layout->addWidget( dirTree_ );
}

SampleBrowser::~SampleBrowser() 
{}

void SampleBrowser::createLoadedSamplesList()
{
	loadedSamplesView_ = new QListView( this );
	loadedSamplesModel_ = new QStandardItemModel( psy::core::MAX_INSTRUMENTS, 1 );

	std::ostringstream buffer;
	buffer.setf(std::ios::uppercase);

	for (int row = 0; row < psy::core::MAX_INSTRUMENTS; ++row) {

		buffer.str("");
		buffer << std::setfill('0') << std::hex << std::setw(2);
		buffer << row << ": " << song()->_pInstrument[row]->_sName;
		QString name = QString::fromStdString( buffer.str() );
		QStandardItem *item = new QStandardItem( name );		
		loadedSamplesModel_->setItem( row, 0, item );
	}
	loadedSamplesView_->setModel( loadedSamplesModel_ );
	loadedSamplesView_->setCurrentIndex( loadedSamplesModel_->index( 0, 0 ) );
}

void SampleBrowser::createActionsWidget()
{
	actionsWidget_ = new QWidget( this );
	QVBoxLayout *actionsLayout = new QVBoxLayout();
	actionsWidget_->setLayout( actionsLayout );
	button_addToLoadedSamples = new QPushButton( "<<" );
	connect( button_addToLoadedSamples, SIGNAL( clicked() ),
		 this, SLOT( onAddToLoadedSamples() ) );

	actionsLayout->addWidget( button_addToLoadedSamples );
	actionsLayout->addWidget( new QPushButton("Remove sample") );
}

void SampleBrowser::createSampleBrowserTree()
{
	QStringList nameFilters("*");
	dirModel_ = new QDirModel( nameFilters, QDir::AllEntries, QDir::Name );
	dirTree_ = new QTreeView();
	dirTree_->setModel( dirModel_ );
	dirTree_->setSelectionMode( QAbstractItemView::ExtendedSelection );
	dirTree_->setColumnHidden( 1, true );
	dirTree_->setColumnHidden( 2, true );
	dirTree_->setColumnHidden( 3, true );
	QString defaultSamplePath = QString::fromStdString( Global::configuration().samplePath() );
	dirTree_->setRootIndex( dirModel_->index( defaultSamplePath ) );
}

void SampleBrowser::onAddToLoadedSamples()
{
	QItemSelectionModel *selModel = dirTree_->selectionModel();
	QModelIndexList selList = selModel->selectedRows();

	for (int i = 0; i < selList.size(); ++i) 
	{
		if ( loadedSamplesView_->currentIndex().isValid() ) 
		{
			QFileInfo fileinfo = dirModel_->fileInfo( selList.at(i) );
			if ( fileinfo.isFile() ) // Don't try to load directories.
			{
				QString pathToWavfile = dirModel_->filePath( selList.at(i) );
				int curInstrIndex = loadedSamplesView_->currentIndex().row();
				if ( song()->WavAlloc( curInstrIndex, pathToWavfile.toStdString().c_str() ) )
				{
					QModelIndex currentIndex = loadedSamplesView_->currentIndex();
					QStandardItem *item = loadedSamplesModel_->itemFromIndex( currentIndex );
					std::ostringstream buffer;
					buffer.setf(std::ios::uppercase);			       
					buffer.str("");
					buffer << std::setfill('0') << std::hex << std::setw(2);
					buffer << curInstrIndex << ": " << song()->_pInstrument[curInstrIndex]->_sName;
					QString name = QString::fromStdString( buffer.str() );
					item->setText( name );
				
			
					QModelIndex nextIndex = loadedSamplesModel_->sibling( loadedSamplesView_->currentIndex().row() + 1, 0, currentIndex );
					loadedSamplesView_->setCurrentIndex( nextIndex );
				}
			}
		}
	}
	emit sampleAdded();
}

