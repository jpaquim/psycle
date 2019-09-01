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

#ifndef MACHINESMODEL_H
#define MACHINESMODEL_H

namespace psy { namespace core {
		class Song;
		class Machine;
	}}

#include <QStandardItemModel>

/**
 * InstrumentModel models the instruments array of a CoreSong.
 * It provides a single point of entry for any GUI classes that
 * need access to the song's instruments.
 *
 * Try to use this interface rather than accessing the CoreSong
 * array directly.
 */
class MachinesModel : public QStandardItemModel {
Q_OBJECT
public:
	MachinesModel( psy::core::Song *song );
	~MachinesModel();

	void addMachine( psy::core::Machine *mac );
	QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const ;

private:
	psy::core::Song *song_;
};

#endif
