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
	  m_song( song )
{
	QHBoxLayout *layout = new QHBoxLayout();
	setLayout( layout );

	loadedSamplesView = new QListView( this );
	loadedSamplesModel = new QStandardItemModel( psy::core::MAX_INSTRUMENTS, 1 );

	std::ostringstream buffer;
	buffer.setf(std::ios::uppercase);

	for (int row = 0; row < psy::core::MAX_INSTRUMENTS; ++row) {

		buffer.str("");
		buffer << std::setfill('0') << std::hex << std::setw(2);
		buffer << row << ": " << song->_pInstrument[row]->_sName;
		QString name = QString::fromStdString( buffer.str() );
		QStandardItem *item = new QStandardItem( name );		
		loadedSamplesModel->setItem( row, 0, item );
	}
	loadedSamplesView->setModel( loadedSamplesModel );

	QWidget *actionsWidget = new QWidget( this );
	QVBoxLayout *actionsLayout = new QVBoxLayout();
	actionsWidget->setLayout( actionsLayout );
	button_addToLoadedSamples = new QPushButton( "<<" );
	connect( button_addToLoadedSamples, SIGNAL( clicked() ),
		 this, SLOT( onAddToLoadedSamples() ) );

	actionsLayout->addWidget( button_addToLoadedSamples );
	actionsLayout->addWidget( new QPushButton("Remove sample") );


	QStringList nameFilters("*");
	dirModel = new QDirModel( nameFilters, QDir::AllEntries, QDir::Name );
	dirTree = new QTreeView();
	dirTree->setModel( dirModel );
	dirTree->setSelectionMode( QAbstractItemView::ExtendedSelection );
	dirTree->setColumnHidden( 1, true );
	dirTree->setColumnHidden( 2, true );
	dirTree->setColumnHidden( 3, true );
	QString defaultSamplePath = QString::fromStdString( Global::configuration().samplePath() );
	dirTree->setRootIndex( dirModel->index( defaultSamplePath ) );

	layout->addWidget( loadedSamplesView );
	layout->addWidget( actionsWidget );
	layout->addWidget( dirTree );
}

SampleBrowser::~SampleBrowser()
{
}

void SampleBrowser::onAddToLoadedSamples()
{
	QItemSelectionModel *selModel = dirTree->selectionModel();
	QModelIndexList selList = selModel->selectedRows();

	for (int i = 0; i < selList.size(); ++i) 
	{
		if ( loadedSamplesView->currentIndex().isValid() ) 
		{
			QFileInfo fileinfo = dirModel->fileInfo( selList.at(i) );
			if ( fileinfo.isFile() ) // Don't try to load directories.
			{
				QString pathToWavfile = dirModel->filePath( selList.at(i) );
				int curInstrIndex = loadedSamplesView->currentIndex().row();
				if ( song()->WavAlloc( curInstrIndex, pathToWavfile.toStdString().c_str() ) )
				{
					QModelIndex currentIndex = loadedSamplesView->currentIndex();
					QStandardItem *item = loadedSamplesModel->itemFromIndex( currentIndex );
					std::ostringstream buffer;
					buffer.setf(std::ios::uppercase);			       
					buffer.str("");
					buffer << std::setfill('0') << std::hex << std::setw(2);
					buffer << curInstrIndex << ": " << song()->_pInstrument[curInstrIndex]->_sName;
					QString name = QString::fromStdString( buffer.str() );
					item->setText( name );
				
			
					QModelIndex nextIndex = loadedSamplesModel->sibling( loadedSamplesView->currentIndex().row() + 1, 0, currentIndex );
					loadedSamplesView->setCurrentIndex( nextIndex );
				}
			}
		}
	}
	emit sampleAdded();
}

