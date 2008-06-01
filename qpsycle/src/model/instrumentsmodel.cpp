// -*- mode:c++; indent-tabs-mode:t -*-
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
#include <psycle/core/song.h>
#include <psycle/core/instrument.h>

#include "instrumentsmodel.hpp"

#include <QTextCodec>
#include <iostream>
#include <iomanip>
#include <sstream>

/** 
 * The InstrumentsModel provides an interface to the
 * Instrument data in the CoreSong.  This model can be
 * loaded by Qt widgets and changes made by one widget will be
 * automatically propagated to the other widgets using the 
 * same model.
 *
 * You can still access the CoreSong instrument data directly in the GUI if
 * you want to, but you'll probably need to manually alert
 * any widgets that are supposed to be watching it.
 */
namespace qpsycle {

	InstrumentsModel::InstrumentsModel( psy::core::Song *song )
		: song_( song )
	{}

	InstrumentsModel::~InstrumentsModel()
	{}

	int InstrumentsModel::rowCount( const QModelIndex & /* parent */ ) const
	{
		return psy::core::MAX_INSTRUMENTS;
	}

	QVariant InstrumentsModel::data( const QModelIndex & index, int role ) const
	{
		if ( !index.isValid() )
			return QVariant();

		if ( role == Qt::DisplayRole )
		{
			std::ostringstream buffer;
			int instIndex = index.row();
			buffer.setf(std::ios::uppercase);
			buffer.str("");
			buffer << std::setfill('0') << std::hex << std::setw(2);
			buffer << instIndex << ": ";
			if ( slotIsEmpty( instIndex ) ) {
				buffer << "empty";
			}
			else {
				buffer << song_->_pInstrument[instIndex]->_sName;
			}
			return QString::fromStdString( buffer.str() );
		}

		return QVariant();
	}

	void InstrumentsModel::setName( int instrIndex, const QString & newname )
	{
		if (
			instrIndex < 0 || instrIndex >= psy::core::MAX_INSTRUMENTS ||
			slotIsEmpty( instrIndex )
			)
			return;

		psy::core::Instrument *inst = song_->_pInstrument[instrIndex];
		QTextCodec::setCodecForCStrings(QTextCodec::codecForLocale());
		inst->setName( newname.toStdString() );

		// Let interested views know that something happenened.
		dataChanged( index( instrIndex ), index( instrIndex ) );
	}

	/**
	 * Loads a wave file into the CoreSong, and updates the
	 * model accordingly.
	 */
	bool InstrumentsModel::loadInstrument( int instrIndex, QString pathToWavfile )
	{
		// WavAlloc tries to load the wav into the CoreSong.
		QTextCodec::setCodecForCStrings(QTextCodec::codecForLocale());
		if ( song_->WavAlloc( instrIndex, pathToWavfile.toStdString().c_str() ) )
		{
			emit selectedInstrumentChanged(instrIndex);

			// Let interested views know that something happenened.
			dataChanged( index( instrIndex ), index( instrIndex ) );

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
	 * the model accordingly.  Note that updating the model
	 * automatically alerts any widgets using the model, so
	 * we don't need to send signals out.
	 */
	void InstrumentsModel::clearInstrument( int instrIndex )
	{
		assert( instrIndex >= 0 );
		assert( instrIndex < psy::core::MAX_INSTRUMENTS );
		
		// Clear the instrument from the CoreSong.
		song_->DeleteInstrument( instrIndex );

		// Let interested views know that something happenened.
		dataChanged( index( instrIndex ), index( instrIndex ) );
	}

	int InstrumentsModel::selectedInstrumentIndex()
	{
		return song_->instSelected();
	}

	void InstrumentsModel::setSelectedInstrumentIndex( int newIndex )
	{
		assert( newIndex >= 0 );
		assert( newIndex < psy::core::MAX_INSTRUMENTS );
		assert( song_->_pInstrument[newIndex] != NULL );

		song_->instSelected( newIndex );
		song_->auxcolSelected = newIndex;

		emit selectedInstrumentChanged(newIndex);
	}

	// Find out if a particular slot is free in the CoreSong.
	bool InstrumentsModel::slotIsEmpty( int instrIndex ) const
	{
		assert( instrIndex >= 0 );
		assert( instrIndex < psy::core::MAX_INSTRUMENTS );
		return song_->_pInstrument[instrIndex]->Empty();
	}

}
