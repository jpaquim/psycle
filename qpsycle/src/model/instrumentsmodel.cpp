
#include <psycle/core/song.h>

#include "instrumentsmodel.h"

#include <iostream>
#include <iomanip>

InstrumentsModel::InstrumentsModel( psy::core::Song *song, int rows, int columns, QObject *parent )
	: QStandardItemModel( rows, columns, parent ),
	  song_( song )
{
	std::ostringstream buffer;
	buffer.setf(std::ios::uppercase);

	for (int row = 0; row < psy::core::MAX_INSTRUMENTS; ++row) {
		buffer.str("");
		buffer << std::setfill('0') << std::hex << std::setw(2);
		buffer << row << ": " << song_->_pInstrument[row]->_sName;
		QString name = QString::fromStdString( buffer.str() );
		QStandardItem *item = new QStandardItem( name );		
		setItem( row, 0, item );
	}
}

InstrumentsModel::~InstrumentsModel()
{}

bool InstrumentsModel::loadInstrument( int instrIndex, QString pathToWavfile )
{
	if ( song_->WavAlloc( instrIndex, pathToWavfile.toStdString().c_str() ) )
	{
		QStandardItem *tempItem = item( instrIndex );
		std::ostringstream buffer;
		buffer.setf(std::ios::uppercase);			       
		buffer.str("");
		buffer << std::setfill('0') << std::hex << std::setw(2);
		buffer << instrIndex << ": " << song_->_pInstrument[instrIndex]->_sName;
		QString name = QString::fromStdString( buffer.str() );
		tempItem->setText( name );

		return true;
	}
	return false;
}

void InstrumentsModel::clearInstrument( int instrIndex )
{
	song_->DeleteInstrument( instrIndex );
	std::ostringstream buffer;
	buffer.setf(std::ios::uppercase);			       
	buffer.str("");
	buffer << std::setfill('0') << std::hex << std::setw(2);
	buffer << instrIndex << ": " << song_->_pInstrument[instrIndex]->_sName;
	QString name = QString::fromStdString( buffer.str() );
	item( instrIndex )->setText( name );
}
