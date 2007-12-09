/* -*- mode:c++, indent-tabs-mode:t -*- */
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
#include <qpsyclePch.hpp>

#include "newmachinedlg.h"

#include <psycle/core/pluginfinder.h>
#include "../global.h"
#include "../configuration.h"

#include <QtGui>

NewMachineDlg::NewMachineDlg(QWidget *parent) 
	:
		QDialog(parent),
		selectedItem(NULL)
{
		setWindowTitle(tr("Choose New Machine"));
		resize(500, 500);
		
		QGridLayout *layout = new QGridLayout();

		// Should we use a tree layout instead of tabs?
		QTabWidget *machineTabs = new QTabWidget();

		finder_ = new psy::core::PluginFinder(Global::configuration().pluginPath(), Global::configuration().ladspaPath());

		genList = new QListWidget();
		efxList = new QListWidget();
		intList = new QListWidget();
		ladList = new QListWidget();

		std::map< psy::core::PluginFinderKey, psy::core::PluginInfo >::const_iterator it = finder_->begin();
		for ( ; it != finder_->end(); it++ ) {
		const psy::core::PluginFinderKey & key = it->first;
		const psy::core::PluginInfo & info = it->second;
		QListWidget* list=NULL;

		switch(info.type()) {
		case psy::core::MACH_SAMPLER:
			list=intList;
			break;
		case psy::core::MACH_PLUGIN:
			switch(info.mode()) {
			case psy::core::MACHMODE_GENERATOR: list = genList; break;
			case psy::core::MACHMODE_FX: list = efxList; break;
			}
			break;
		case psy::core::MACH_LADSPA: list = ladList; break;
		};

		if (list) {
			QListWidgetItem *item = new QListWidgetItem( QString::fromStdString( info.name() ) );
			list->addItem(item);
			pluginIdentify_[item] = key;
		}
		}

		inItemSelectionChanged = false;

		QListWidget* lists[4] = { genList, efxList, intList, ladList };

		for(int i=0;i<4;i++) {
		connect( lists[i], SIGNAL( itemSelectionChanged( ) ), 
				this, SLOT( itemSelectionChanged( ) ) );

		connect( lists[i], SIGNAL( itemActivated( QListWidgetItem* ) ),
				this, SLOT( tryAccept() ) );
		}

		buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
										| QDialogButtonBox::Cancel);
		connect(buttonBox, SIGNAL(accepted()), this, SLOT(tryAccept()));
		connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
		
		machineTabs->addTab(genList, QIcon(":images/gen-native.png"), "Generators");
		machineTabs->addTab(efxList, QIcon(":images/efx-native.png"), "Effects");
		machineTabs->addTab(intList, QIcon(":images/gen-internal.png"), "Internal");
		machineTabs->addTab(ladList, "Ladspa");
		
		layout->addWidget(machineTabs);
		layout->addWidget(buttonBox);
		setLayout(layout);
	}

void NewMachineDlg::keyPressEvent( QKeyEvent *event )
{
	if ( event->key() == Qt::Key_W && event->modifiers() == Qt::ControlModifier ) {
		reject(); // closes the dialog
	} else if ( event->key() == Qt::Key_Escape ) {
		reject(); // closes the dialog
	}
}

void NewMachineDlg::itemSelectionChanged()
{
	// prevent reentry
	if(inItemSelectionChanged) {
		return;
	}
	inItemSelectionChanged = true;

	QListWidget* lists[4] = { genList, efxList, intList, ladList };
	QListWidgetItem* newItem=0;

	for(int i=0;i<4;i++) {
	QList<QListWidgetItem*> selections = lists[i]->selectedItems();
	if (selections.count() > 0) {
		assert(selections.count() <= 1);
		QListWidgetItem* item=selections.at(0);
		if (item != selectedItem) {
		newItem = item;
		break;
		}
	}
	}
	if (newItem) {
	// remove selection from previous listWidget
	if (selectedItem) {
		selectedItem->listWidget()->setItemSelected(selectedItem,false);
	}
	// remember which item is selected now
	selectedItem = newItem;
	setPlugin(selectedItem);
	}

	inItemSelectionChanged = false;
}

void NewMachineDlg::setPlugin( QListWidgetItem* item ) 
{
	std::map< QListWidgetItem*, psy::core::PluginFinderKey >::iterator it;
	it = pluginIdentify_.find( item );
	
	if ( it != pluginIdentify_.end() ) {
	const psy::core::PluginInfo & info = finder_->info( it->second );
		const psy::core::PluginFinderKey & key = it->second;

		#if 0
		name->setText( info.name() );
		dllName_ = info.libName();
		libName->setText( dllName_ );
		description->setText( "Psycle Instrument by "+ info.author() );
		apiVersion->setText( info.version() );
		#endif

		selectedKey_ = key;
	}
	else {
		std::fprintf(stderr,"Unable to find plugin for QListWidgetItem\n");
	}
}

void NewMachineDlg::tryAccept() {
	if (selectedItem) {
		accept();
	}
}


const psy::core::PluginFinderKey & NewMachineDlg::pluginKey() const {
	return selectedKey_;
}
