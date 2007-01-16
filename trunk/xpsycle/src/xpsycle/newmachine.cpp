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
#include "configuration.h"
#include "global.h"
#include "newmachine.h"
#include <ngrs/borderlayout.h>
#include <ngrs/alignlayout.h>
#include <ngrs/label.h>
#include <ngrs/listbox.h>
#include <ngrs/item.h>
#include <ngrs/button.h>
#include <ngrs/frameborder.h>

namespace psycle {
	namespace host {

NewMachine::NewMachine( const PluginFinder & finder )
: ngrs::Dialog(), finder_( finder )
{
  setTitle("Add New Machine");
  
  setPosition(100,100,700,500);

  pane()->setLayout( ngrs::AlignLayout(10,5) );

  ngrs::Panel* bPnl = new ngrs::Panel();
    bPnl->setAlign(ngrs::nAlBottom);
    bPnl->setLayout( ngrs::AlignLayout(5,10) );
    ngrs::Button* okBtn = new ngrs::Button("Open");
      okBtn->clicked.connect(this,&NewMachine::onOkBtn);
      okBtn->setFlat(false);
    bPnl->add(okBtn, ngrs::nAlRight );
    ngrs::Button* cancelBtn = new ngrs::Button("Cancel");
      cancelBtn->clicked.connect(this,&NewMachine::onCancelBtn);
      cancelBtn->setFlat(false);
    bPnl->add(cancelBtn, ngrs::nAlRight);
  pane()->add(bPnl, ngrs::nAlBottom);

  ngrs::Panel* properties = new ngrs::Panel();
		properties->setBorder ( ngrs::FrameBorder( true,5,5 ) );
    properties->setPreferredSize(240,100);
    properties->setAlign(ngrs::nAlRight);
    properties->setLayout( ngrs::AlignLayout(5,5) );
    macProperty = new ngrs::GroupBox();
      macProperty->setAlign(ngrs::nAlTop);
      macProperty->setHeaderText("Machine Properties");
      macProperty->setWidth(200);
      macProperty->setHeight(300);
      macProperty->setLayout(ngrs::ListLayout());
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
  pane()->add(properties, ngrs::nAlRight);

  tabBook_ = new ngrs::TabBook();
    tabBook_->setAlign(ngrs::nAlClient);
    ngrs::Panel* generatorPage = new ngrs::Panel();
        generatorPage->setLayout(ngrs::AlignLayout());
          generatorfBox_ = new ngrs::ListBox();
            generatorfBox_->setAlign(ngrs::nAlClient);
            generatorfBox_->itemSelected.connect(this,&NewMachine::onGeneratorItemSelected);
				std::map< PluginFinderKey, PluginInfo >::const_iterator it = finder.begin();
				for ( ; it != finder.end(); it++ ) {
					const PluginFinderKey & key = it->first;
					const PluginInfo & info = it->second;
					if ( info.type() == MACH_PLUGIN && info.mode() == MACHMODE_GENERATOR ) {
						ngrs::Item* item = new ngrs::Item( info.name() );
							item->mouseDoublePress.connect(this,&NewMachine::onItemDblClick);
							item->setIntValue( key.index() );
						generatorfBox_->add( item );
						pluginIdentify_[item] = key;
				}
		}
        generatorPage->add(generatorfBox_);

    ngrs::Panel* effectPage = new ngrs::Panel();
      effectPage->setLayout ( ngrs::AlignLayout() );
      effectfBox_ = new ngrs::ListBox();
      effectfBox_->setAlign(ngrs::nAlClient);
      effectfBox_->itemSelected.connect(this,&NewMachine::onEffectItemSelected);

      it = finder.begin();
      for ( ; it != finder.end(); it++ ) {
        const PluginFinderKey & key = it->first;
        const PluginInfo & info = it->second;
        if ( info.type() == MACH_PLUGIN && info.mode() == MACHMODE_FX ) {
         ngrs::Item* item = new ngrs::Item( info.name() );
         item->mouseDoublePress.connect(this,&NewMachine::onItemDblClick);
         effectfBox_->add( item );
         pluginIdentify_[item] = key;
        }					
      }
    effectPage->add( effectfBox_ , ngrs::nAlClient);


    tabBook_->addPage(generatorPage,"Generators");
    tabBook_->addPage(effectPage,"Effects");
    ngrs::ListBox* internalPage_ = new ngrs::ListBox();
    ngrs::Item* item = new ngrs::Item("Sampler");
    item->mouseDoublePress.connect(this,&NewMachine::onItemDblClick);
        internalPage_->add(item);
        internalPage_->itemSelected.connect(this,&NewMachine::onInternalItemSelected);
    tabBook_->addPage(internalPage_,"Internal");
	  
  	
		ngrs::Panel* ladspaPage = new ngrs::Panel();
			ladspaPage->setLayout(ngrs::AlignLayout());
        ladspaBox_ = new ngrs::ListBox();
        ladspaBox_->itemSelected.connect(this,&NewMachine::onLADSPAItemSelected);

				it = finder.begin();
				for ( ; it != finder.end(); it++ ) {
					const PluginFinderKey & key = it->first;
					const PluginInfo & info = it->second;
					if ( info.type() == MACH_LADSPA ) {
						ngrs::Item* item = new ngrs::Item( info.name() );
                                                item->mouseDoublePress.connect(this,&NewMachine::onItemDblClick);
					 	item->setIntValue( key.index() );
						ladspaBox_->add( item );
						pluginIdentify_[item] = key;
					}					
				}
      ladspaPage->add(ladspaBox_, ngrs::nAlClient);
	tabBook_->addPage(ladspaPage,"ladspaPage");

  pane()->add(tabBook_);

  ngrs::NTab* tab = tabBook_->tab( effectPage );
    tab->click.connect(this,&NewMachine::onEffectTabChange);
  tab = tabBook_->tab( generatorPage );
    tab->click.connect(this,&NewMachine::onGeneratorTabChange);
  //tab = tabBook_->tab( internalPage );
  //  tab->click.connect(this,&NewMachine::onInternalTabChange);
  tab = tabBook_->tab( ladspaPage );
	tab->click.connect(this,&NewMachine::onLADSPATabChange);


  tabBook_->setActivePage(0); // generator page.
}


NewMachine::~NewMachine()
{
}

int NewMachine::onClose( )
{
  setVisible(false);
  setExitLoop( ngrs::nDestroyWindow );
  return ngrs::nDestroyWindow;
}

void NewMachine::onOkBtn( ngrs::ButtonEvent * sender )
{
  doClose( true );
}

void NewMachine::onItemDblClick( ngrs::ButtonEvent * sender )
{
  doClose( true );
}

void NewMachine::onCancelBtn( ngrs::ButtonEvent * sender )
{
  doClose( false );
}

void NewMachine::onInternalItemSelected( ngrs::ItemEvent * ev )
{
  if (ev->text() == "Sampler") {
    selectedKey_ = PluginFinderKey::internalSampler();
  }
}

void NewMachine::onGeneratorItemSelected( ngrs::ItemEvent* ev )
{
  ngrs::CustomItem* item = generatorfBox_->itemAt( generatorfBox_->selIndex() );
  setPlugin ( item );
}

void NewMachine::onEffectItemSelected( ngrs::ItemEvent* ev ) {
  ngrs::CustomItem* item = effectfBox_->itemAt( effectfBox_->selIndex() );
  setPlugin( item );
}

void NewMachine::onLADSPAItemSelected( ngrs::ItemEvent* ev ) {
   ngrs::CustomItem* item = ladspaBox_->itemAt( ladspaBox_->selIndex() );
   setPlugin( item );
}

void NewMachine::setPlugin( ngrs::CustomItem* item ) {
    std::map< ngrs::CustomItem*, PluginFinderKey >::iterator it;		
	it = pluginIdentify_.find( item );

    if ( it != pluginIdentify_.end() ) {
		const PluginInfo & info = finder_.info( it->second );
		const PluginFinderKey & key = it->second;

		name->setText( info.name() );
    	dllName_ = info.libName();
		libName->setText( dllName_ );
		description->setText( "Psycle Instrument by "+ info.author() );
		apiVersion->setText( info.version() ); 

		selectedKey_ = key;
    }

	pane()->resize();
    pane()->repaint();
}

void NewMachine::onEffectTabChange( ngrs::ButtonEvent * ev )
{
  ngrs::CustomItem* item = effectfBox_->itemAt( effectfBox_->selIndex() );
  if  (item) setPlugin ( item );
}

void NewMachine::onGeneratorTabChange( ngrs::ButtonEvent * ev )
{
  ngrs::CustomItem* item = generatorfBox_->itemAt( generatorfBox_->selIndex() );
  if  (item) setPlugin ( item );
}

void NewMachine::onLADSPATabChange( ngrs::ButtonEvent * ev )
{
 ngrs::CustomItem* item = ladspaBox_->itemAt( ladspaBox_->selIndex() );
  if  (item) setPlugin ( item );
}

void NewMachine::onInternalTabChange( ngrs::ButtonEvent * ev )
{
}

const PluginFinderKey & NewMachine::pluginKey() const {
	return selectedKey_;
}

}
}


