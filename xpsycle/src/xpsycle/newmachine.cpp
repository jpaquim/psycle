/***************************************************************************
  *   Copyright (C) 2006 by Stefan   *
  *   natti@linux   *
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
#include "newmachine.h"
#include "plugin.h"
#include "ladspamachine.h"
#include "configuration.h"
#include "newmachine.h"
#include <ngrs/nborderlayout.h>
#include <ngrs/nalignlayout.h>
#include <ngrs/nlabel.h>
#include <ngrs/nlistbox.h>
#include <ngrs/nitem.h>
#include <ngrs/nbutton.h>
#include <ngrs/nframeborder.h>

namespace psycle {
	namespace host {

NewMachine::NewMachine( )
  : NWindow()
{
	id_ = MACH_UNDEFINED;
	pluginIndex_ = 0;

	setTitle("Add New Machine");

  setPosition(100,100,700,500);

  pane()->setLayout( NAlignLayout(10,5) );

  setModal(true);
  do_Execute = false;

  NPanel* bPnl = new NPanel();
    bPnl->setAlign(nAlBottom);
    bPnl->setLayout( NAlignLayout(5,10) );
    NButton* okBtn = new NButton("Open");
      okBtn->clicked.connect(this,&NewMachine::onOkBtn);
      okBtn->setFlat(false);
    bPnl->add(okBtn, nAlRight );
    NButton* cancelBtn = new NButton("Cancel");
      cancelBtn->clicked.connect(this,&NewMachine::onCancelBtn);
      cancelBtn->setFlat(false);
    bPnl->add(cancelBtn, nAlRight);
  pane()->add(bPnl, nAlBottom);

  NPanel* properties = new NPanel();
		properties->setBorder ( NFrameBorder( true,5,5 ) );
    properties->setPreferredSize(240,100);
    properties->setAlign(nAlRight);
    properties->setLayout(NAlignLayout(5,5));
    macProperty = new NGroupBox();
      macProperty->setAlign(nAlTop);
      macProperty->setHeaderText("Machine Properties");
      macProperty->setWidth(200);
      macProperty->setHeight(300);
      macProperty->setLayout(NListLayout());
      name = new InfoLine("Name");
      macProperty->add(name);
      description = new InfoLine("Description");
      macProperty->add(description);
      libName = new InfoLine("libName");
      macProperty->add(libName);
      apiVersion = new InfoLine("Api Version");
      macProperty->add(apiVersion);
      macProperty->resize();
    properties->add(macProperty);
    properties->resize();
  pane()->add(properties, nAlRight);


  finder.scanAll();

  tabBook_ = new NTabBook();
    tabBook_->setAlign(nAlClient);
    NPanel* generatorPage = new NPanel();
        generatorPage->setLayout(NAlignLayout());
          generatorfBox_ = new NListBox();
            generatorfBox_->setAlign(nAlClient);
            generatorfBox_->itemSelected.connect(this,&NewMachine::onGeneratorItemSelected);
						std::map< PluginFinderKey, PluginInfo >::const_iterator it = finder.begin();
						for ( ; it != finder.end(); it++ ) {
							PluginFinderKey key = it->first;
							PluginInfo info = it->second;
							if ( info.type() == MACH_PLUGIN && info.mode() == MACHMODE_GENERATOR ) {
								NItem* item = new NItem( info.name() );
								item->setIntValue( key.index() );
								generatorfBox_->add( item );
								pluginIdentify_[item] = key;
							}					
						}
        generatorPage->add(generatorfBox_);

    NPanel* effectPage = new NPanel();
      effectPage->setLayout ( NAlignLayout() );
      effectfBox_ = new NListBox();
      effectfBox_->setAlign(nAlClient);
      effectfBox_->itemSelected.connect(this,&NewMachine::onEffectItemSelected);
			it = finder.begin();
      for ( ; it != finder.end(); it++ ) {
        PluginFinderKey key = it->first;
        PluginInfo info = it->second;
        if ( info.type() == MACH_PLUGIN && info.mode() == MACHMODE_FX ) {
         NItem* item = new NItem( info.name() );
         effectfBox_->add( item );
         pluginIdentify_[item] = key;
        }					
      }
    effectPage->add( effectfBox_ , nAlClient);


    tabBook_->addPage(effectPage,"Effects");
    tabBook_->addPage(generatorPage,"Generators");
    NListBox* internalPage_ = new NListBox();
        internalPage_->add(new NItem("Sampler"));
        internalPage_->itemSelected.connect(this,&NewMachine::onInternalItemSelected);
    tabBook_->addPage(internalPage_,"Internal");
	  
  	
		NPanel* ladspaPage = new NPanel();
			ladspaPage->setLayout(NAlignLayout());
        ladspaBox_ = new NListBox();
        ladspaBox_->itemSelected.connect(this,&NewMachine::onLADSPAItemSelected);
				it = finder.begin();
				for ( ; it != finder.end(); it++ ) {
					PluginFinderKey key = it->first;
					PluginInfo info = it->second;
					if ( info.type() == MACH_LADSPA ) {
						NItem* item = new NItem( info.name() );
						item->setIntValue( key.index() );
						ladspaBox_->add( item );
						pluginIdentify_[item] = key;
					}					
				}			
      ladspaPage->add(ladspaBox_, nAlClient);
	tabBook_->addPage(ladspaPage,"ladspaPage");

  pane()->add(tabBook_);

  NTab* tab = tabBook_->tab( effectPage );
    tab->click.connect(this,&NewMachine::onEffectTabChange);
  tab = tabBook_->tab( generatorPage );
    tab->click.connect(this,&NewMachine::onGeneratorTabChange);
  //tab = tabBook_->tab( internalPage );
  //  tab->click.connect(this,&NewMachine::onInternalTabChange);
  tab = tabBook_->tab( ladspaPage );
    tab->click.connect(this,&NewMachine::onLADSPATabChange);


  tabBook_->setActivePage(1);
}


NewMachine::~NewMachine()
{
}

int NewMachine::onClose( )
{
	do_Execute = false;
  setVisible(false);
  setExitLoop(nDestroyWindow);
}

void NewMachine::onOkBtn( NButtonEvent * sender )
{
  do_Execute = true;
  setVisible(false);
  setExitLoop(nDestroyWindow);
}

void NewMachine::onCancelBtn( NButtonEvent * sender )
{
  do_Execute = false;
  setVisible(false);
  setExitLoop(nDestroyWindow);
}

bool NewMachine::execute( )
{
  setVisible(true);
  return do_Execute;
}

std::string NewMachine::getDllName( )
{
  return dllName_;
}

bool NewMachine::outBus( )
{
  return true;   // true = Generator, false = Effect
}

void NewMachine::onInternalItemSelected( NItemEvent * ev )
{
  if (ev->text() == "Sampler") {
		id_ = MACH_SAMPLER;
	} else {

	}
}

void NewMachine::onGeneratorItemSelected( NItemEvent * ev )
{
  NCustomItem* item = generatorfBox_->itemAt( generatorfBox_->selIndex() );
  setPlugin ( item );
}

void NewMachine::onEffectItemSelected(NItemEvent* ev) {
	  NCustomItem* item = effectfBox_->itemAt( effectfBox_->selIndex() );
    setPlugin( item );
}

void NewMachine::onLADSPAItemSelected(NItemEvent* ev) {
   NCustomItem* item = ladspaBox_->itemAt( ladspaBox_->selIndex() );
   setPlugin( item );
}

void NewMachine::setPlugin( NCustomItem* item ) {
    std::map< NCustomItem*, PluginFinderKey >::iterator it;		
		it = pluginIdentify_.find( item );

    if ( it != pluginIdentify_.end() ) {

			PluginInfo info = finder.info( it->second );
			const PluginFinderKey & key = it->second;

			name->setText( info.name() );
    	dllName_ = info.libName();
      libName->setText( dllName_ );
      description->setText( "Psycle Instrument by "+ info.author() );
      apiVersion->setText( info.version() ); 
			id_ = info.type();
		  
    }
		pane()->resize();
    pane()->repaint();
}

Machine::id_type NewMachine::selectedType() const {
	return id_;
}

int NewMachine::pluginIndex() const {
  return pluginIndex_;
}

void NewMachine::onEffectTabChange( NButtonEvent * ev )
{
  NCustomItem* item = effectfBox_->itemAt( effectfBox_->selIndex() );
  if  (item) setPlugin ( item );
}

void NewMachine::onGeneratorTabChange( NButtonEvent * ev )
{
  NCustomItem* item = generatorfBox_->itemAt( generatorfBox_->selIndex() );
  if  (item) setPlugin ( item );
}

void NewMachine::onLADSPATabChange( NButtonEvent * ev )
{
 NCustomItem* item = ladspaBox_->itemAt( ladspaBox_->selIndex() );
  if  (item) setPlugin ( item );
}

void NewMachine::onInternalTabChange( NButtonEvent * ev )
{
  
}

}
}


