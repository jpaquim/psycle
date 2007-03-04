/***************************************************************************
*   Copyright (C) 2006 by  Neil Mather   *
*   nmather@sourceforge   *
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

#include <QtGui>

 #include "newmachinedlg.h"
 #include "psycore/pluginfinder.h"

 NewMachineDlg::NewMachineDlg(QWidget *parent) 
    : QDialog(parent)
 {
     setWindowTitle(tr("Choose New Machine"));
     resize(500, 500);
     
     QGridLayout *layout = new QGridLayout();

     QTabWidget *machineTabs = new QTabWidget();

     finder_ = new psy::core::PluginFinder();

     QListWidget *genList = new QListWidget();
     std::map< psy::core::PluginFinderKey, psy::core::PluginInfo >::const_iterator it = finder_->begin();
				for ( ; it != finder_->end(); it++ ) {
					const psy::core::PluginFinderKey & key = it->first;
					const psy::core::PluginInfo & info = it->second;
					if ( info.type() == psy::core::MACH_PLUGIN && info.mode() == psy::core::MACHMODE_GENERATOR ) {
						QListWidgetItem *item = new QListWidgetItem( QString::fromStdString( info.name() ) );
						genList->addItem( item );
						pluginIdentify_[item] = key;
                    }
                }
     connect( genList, SIGNAL( currentItemChanged( QListWidgetItem*, QListWidgetItem* ) ), 
              this, SLOT( currentItemChanged( QListWidgetItem*, QListWidgetItem* ) ) );

     QListWidget *efxList = new QListWidget();
                it = finder_->begin();
				for ( ; it != finder_->end(); it++ ) {
					const psy::core::PluginFinderKey & key = it->first;
					const psy::core::PluginInfo & info = it->second;
					if ( info.type() == psy::core::MACH_PLUGIN && info.mode() == psy::core::MACHMODE_FX ) {
						QListWidgetItem *item = new QListWidgetItem( QString::fromStdString( info.name() ) );
						efxList->addItem( item );
						pluginIdentify_[item] = key;
                    }
                }
     connect( efxList, SIGNAL( currentItemChanged( QListWidgetItem*, QListWidgetItem* ) ), 
              this, SLOT( currentItemChanged( QListWidgetItem*, QListWidgetItem* ) ) );

     QListWidget *intList = new QListWidget();
     connect( intList, SIGNAL( currentItemChanged( QListWidgetItem*, QListWidgetItem* ) ), 
              this, SLOT( currentItemChanged( QListWidgetItem*, QListWidgetItem* ) ) );

     QListWidget *ladList = new QListWidget();
                it = finder_->begin();
				for ( ; it != finder_->end(); it++ ) {
					const psy::core::PluginFinderKey & key = it->first;
					const psy::core::PluginInfo & info = it->second;
					if ( info.type() == psy::core::MACH_LADSPA ) {
						QListWidgetItem *item = new QListWidgetItem( QString::fromStdString( info.name() ) );
						ladList->addItem( item );
						pluginIdentify_[item] = key;
                    }
                }
     connect( ladList, SIGNAL( currentItemChanged( QListWidgetItem*, QListWidgetItem* ) ), 
              this, SLOT( currentItemChanged( QListWidgetItem*, QListWidgetItem* ) ) );

     buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                      | QDialogButtonBox::Cancel);
     connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
     connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

     machineTabs->addTab(genList, "Generators");
     machineTabs->addTab(efxList, "Effects");
     machineTabs->addTab(intList, "Internal");
     machineTabs->addTab(ladList, "Ladspa");

     layout->addWidget(machineTabs);
     layout->addWidget(buttonBox);
     setLayout(layout);
 }

void NewMachineDlg::currentItemChanged( QListWidgetItem *current, QListWidgetItem *previous )
{
    qDebug( "setting plugin." );
	setPlugin ( current );
}

void NewMachineDlg::setPlugin( QListWidgetItem* item ) 
{
		std::map< QListWidgetItem*, psy::core::PluginFinderKey >::iterator it;		
	it = pluginIdentify_.find( item );

		if ( it != pluginIdentify_.end() ) {
		const psy::core::PluginInfo & info = finder_->info( it->second );
		const psy::core::PluginFinderKey & key = it->second;

/*		name->setText( info.name() );
			dllName_ = info.libName();
		libName->setText( dllName_ );
		description->setText( "Psycle Instrument by "+ info.author() );
		apiVersion->setText( info.version() ); */

		selectedKey_ = key;
		}

}

const psy::core::PluginFinderKey & NewMachineDlg::pluginKey() const {
	return selectedKey_;
}
