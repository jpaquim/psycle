/* -*- mode:c++, indent-tabs-mode:t -*- */
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
#include <qpsyclePch.hpp>

#include <psycle/core/constants.h>

#include "global.hpp"
#include "configuration.hpp"
#include "samplebrowser.hpp"
#include "../model/instrumentsmodel.hpp"

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

namespace qpsycle {

SampleBrowser::SampleBrowser( InstrumentsModel *instrumentsModel,
					QWidget *parent )
	: QWidget( parent ),
		instrumentsModel_( instrumentsModel )
{
	QHBoxLayout *layout = new QHBoxLayout();
	setLayout( layout );

	createLoadedSamplesList();
	createActionsWidget();
	createSampleBrowserTree();

	layout->addWidget( instrumentsList_ );
	layout->addWidget( actionsWidget_ );
	layout->addWidget( dirTree_ );
}

SampleBrowser::~SampleBrowser() 
{
}

// Creates the list widget holding the samples currently
// loaded in the CoreSong.
void SampleBrowser::createLoadedSamplesList()
{
	instrumentsList_ = new QListView( this );
	// InstrumentsModel contains info on the samples loaded
	// in the CoreSong.
	instrumentsList_->setModel( instrumentsModel_ );
	instrumentsList_->setCurrentIndex( instrumentsModel_->index( 0, 0 ) );
	instrumentsList_->setEditTriggers( QAbstractItemView::NoEditTriggers );
}

// Creates the widget that holds the buttons that
// do stuff.
void SampleBrowser::createActionsWidget()
{
	actionsWidget_ = new QWidget( this );
	QVBoxLayout *actionsLayout = new QVBoxLayout();
	actionsWidget_->setLayout( actionsLayout );

	button_addToLoadedSamples = new QPushButton( "<<" );
	button_clearInstrument = new QPushButton( "Clear instrument" );

	connect( button_addToLoadedSamples, SIGNAL( clicked() ), this, SLOT( onAddToLoadedSamples() ) );
	connect( button_clearInstrument, SIGNAL( clicked() ), this, SLOT( onClearInstrument() ) );

	actionsLayout->addWidget( button_addToLoadedSamples );
	actionsLayout->addWidget( button_clearInstrument );
}

// Creates the tree widget that allows the user
// to browse through their filesystem for samples.
void SampleBrowser::createSampleBrowserTree()
{
	dirModel_ = new QDirModel( this );

	dirModel_->setNameFilters( QStringList() << "*.wav" );
	dirModel_->setFilter( QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot );
	// Note: NoDotAndDotDot means we can't navigate above the
	// provided root index.  We could allow dot and dot dot, but
	// it's a bit weird with a tree view (try it.)  
	dirModel_->setSorting( QDir::Name );


	dirTree_ = new QTreeView();
	dirTree_->setModel( dirModel_ );
	dirTree_->setSelectionMode( QAbstractItemView::ExtendedSelection );
	dirTree_->setColumnHidden( 1, true ); // hide file size column
	dirTree_->setColumnHidden( 2, true ); // hide file type column 
	dirTree_->setColumnHidden( 3, true ); // hide data modified column

	// Set the top directory of the tree.
	QString samplesPath = QString::fromStdString( Global::configuration().samplePath() );
	if ( samplesPath.isEmpty() ) { // if sample path isn't set in config.
		samplesPath = QDir::homePath();
	}
	dirTree_->setRootIndex( dirModel_->index( samplesPath ) );
}

// Try to load selected samples in the sample tree into
// the CoreSong.
void SampleBrowser::onAddToLoadedSamples()
{
	// Find out what items are selected in the sample tree.
	QItemSelectionModel *selModel = dirTree_->selectionModel();
	QModelIndexList selList = selModel->selectedRows();


	for ( int i = 0; i < selList.size(); ++i ) 
	{
		if ( instrumentsList_->currentIndex().isValid() ) 
		{
			QFileInfo fileinfo = dirModel_->fileInfo( selList.at(i) );
			if ( fileinfo.isFile() ) // Don't try to load directories.
			{
				QString pathToWavfile = dirModel_->filePath( selList.at(i) );
				int curInstrIndex = instrumentsList_->currentIndex().row();
				if ( instrumentsModel_->loadInstrument( curInstrIndex, pathToWavfile ) )
				{
					QModelIndex nextIndex = instrumentsModel_->sibling( instrumentsList_->currentIndex().row() + 1, 0, instrumentsList_->currentIndex() );
					instrumentsList_->setCurrentIndex( nextIndex );
				}
			}
		}
	}
	emit sampleAdded();
}

// Clear an instrument out of the CoreSong.
void SampleBrowser::onClearInstrument() 
{
	int curInstrIndex = instrumentsList_->currentIndex().row();
	instrumentsModel_->clearInstrument( curInstrIndex );
	emit sampleAdded();
}

}
