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
#include <psycle/core/machine.h>

#include "machinesmodel.h"

#include <iostream>
#include <iomanip>

#include <QString>

MachinesModel::MachinesModel( psy::core::Song *song )
	: QStandardItemModel(),
	  song_( song )
{
	bool comboIsEmpty=true;
	std::ostringstream buffer;
	buffer.setf(std::ios::uppercase);

	for (int b=0; b<psy::core::MAX_BUSES; b++) // Generators.
	{
		buffer.str("");
		buffer << std::setfill('0') << std::hex << std::setw(2);
		QStandardItem *item  = new QStandardItem();
		if( song_->machine(b)) {
			buffer << b << ": " << song_->machine(b)->GetEditName();
			comboIsEmpty = false;
		} else {
			buffer << b << ": " << "empty";
		}
		item->setText( QString::fromStdString( buffer.str() ) );
		appendRow( item );
	}

//	appendRow( new QStandardItem("--------------------------") );

	for (int b=psy::core::MAX_BUSES; b<psy::core::MAX_BUSES*2; b++) // Effects.
	{
		buffer.str("");
		buffer << std::setfill('0') << std::hex << std::setw(2);
		QStandardItem *item  = new QStandardItem();
		if( song_->machine(b)) {
			buffer << b << ": " << song_->machine(b)->GetEditName();
			comboIsEmpty = false;
		} else {
			buffer << b << ": " << "empty";
		}
		item->setText( QString::fromStdString( buffer.str() ) );
		appendRow( item );
	}

	if (comboIsEmpty) {
//		appendRow( new QStandardItem( "No Machines Loaded" ) );
	}
}

MachinesModel::~MachinesModel()
{}

void MachinesModel::addMachine( psy::core::Machine *mac )
{
	// find position of machine from its CoreSong id.
	int machinePosition = mac->id();

	std::ostringstream buffer;
	buffer.setf(std::ios::uppercase);
	buffer.str("");
	buffer << std::setfill('0') << std::hex << std::setw(2);
	buffer << machinePosition << ": " << song_->machine( machinePosition )->GetEditName();

	QString machineName = QString::fromStdString( buffer.str() );
	QStandardItem *macItem = item( machinePosition );
	macItem->setText( machineName );

	// insert something represent the machine at that point.
	setItem( machinePosition, macItem );
}

QVariant MachinesModel::data ( const QModelIndex & index, int role ) const 
{
	if ( role == Qt::UserRole+1 ) {
		int machinePosition = index.row();
		std::cout << "mp" << machinePosition << std::endl;
		if ( song_->machine( machinePosition ) ) {
			qDebug("hop");
			return QVariant( machinePosition );
		} else {
			qDebug("hip");
			return QVariant();
		}
	}
	return QStandardItemModel::data( index, role );
}
