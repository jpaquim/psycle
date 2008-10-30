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
#ifndef NEWMACHINEDLG_H
#define NEWMACHINEDLG_H

#include "psycle/core/fwd.hpp"
#include "psycle/core/machinekey.hpp"

#include <QWidget>
#include <QDialog>
#include <QDialogButtonBox>
#include <QListWidgetItem>

class QDialog;

namespace qpsycle {

	class NewMachineDlg : public QDialog {
		Q_OBJECT
	
		public:
		NewMachineDlg(QWidget *parent = 0);

		const psy::core::MachineKey & pluginKey() const;

	protected:
		void keyPressEvent( QKeyEvent *event );

	public slots:
		void itemSelectionChanged( );
		void tryAccept( );

	private:
		QDialogButtonBox *buttonBox;
		const psy::core::PluginFinder *finder_;


		QListWidgetItem* selectedItem;
		psy::core::MachineKey selectedKey_;
		std::map< QListWidgetItem* , psy::core::MachineKey > pluginIdentify_;

		void setPlugin( QListWidgetItem* item );

		QListWidget* genList;
		QListWidget* efxList;
		QListWidget* intList;
		QListWidget* ladList;
		bool inItemSelectionChanged;
	};

} // namespace qpsycle

#endif
