/***************************************************************************
*   Copyright (C) 2007 Psycledelics Community
*   psycle.sf.net
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

#include <psycle/core/song.h>
#include <psycle/core/instrument.h>

#include "instrumentsmodel.h"

#include <iostream>
#include <iomanip>

InstrumentsModel::InstrumentsModel( psy::core::Song *song )
	: QStandardItemModel(),
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
		appendRow( item );
	}
}

InstrumentsModel::~InstrumentsModel()
{}

/**
	* Loads a wave file into the CoreSong, and updates the
	* model accordingly.
	*/
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


/**
	* Returns an instrument from the CoreSong.
	*/
psy::core::Instrument *InstrumentsModel::getInstrument( int instrIndex )
{
	return song_->_pInstrument[instrIndex];
}


/**
	* Clears the instrument from the CoreSong, and updates
	* the model accordingly.
	*/
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

int InstrumentsModel::selectedInstrumentIndex()
{
	return song_->instSelected;
}

void InstrumentsModel::setSelectedInstrumentIndex( int newIndex )
{
	song_->instSelected   = newIndex;
	song_->auxcolSelected = newIndex;
}


bool InstrumentsModel::slotIsEmpty( int instrIndex )
{
	return song_->_pInstrument[instrIndex]->Empty();
}
