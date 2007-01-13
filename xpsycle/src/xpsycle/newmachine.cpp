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
#include <ngrs/nborderlayout.h>
#include <ngrs/nalignlayout.h>
#include <ngrs/nlabel.h>
#include <ngrs/nlistbox.h>
#include <ngrs/nitem.h>
#include <ngrs/nbutton.h>
#include <ngrs/nframeborder.h>

namespace psycle {
	namespace host {

NewMachine::NewMachine( const PluginFinder & finder )
: ngrs::NDialog(), finder_( finder )
{
  setTitle("Add New Machine");
  
  setPosition(100,100,700,500);

  pane()->setLayout( ngrs::NAlignLayout(10,5) );

  ngrs::NPanel* bPnl = new ngrs::NPanel();
    bPnl->setAlign(ngrs::nAlBottom);
    bPnl->setLayout( ngrs::NAlignLayout(5,10) );
    ngrs::NButton* okBtn = new ngrs::NButton("Open");
      okBtn->clicked.connect(this,&NewMachine::onOkBtn);
      okBtn->setFlat(false);
    bPnl->add(okBtn, ngrs::nAlRight );
    ngrs::NButton* cancelBtn = new ngrs::NButton("Cancel");
      cancelBtn->clicked.connect(this,&NewMachine::onCancelBtn);
      cancelBtn->setFlat(false);
    bPnl->add(cancelBtn, ngrs::nAlRight);
  pane()->add(bPnl, ngrs::nAlBottom);

  ngrs::NPanel* properties = new ngrs::NPanel();
		properties->setBorder ( ngrs::NFrameBorder( true,5,5 ) );
    properties->setPreferredSize(240,100);
    properties->setAlign(ngrs::nAlRight);
    properties->setLayout( ngrs::NAlignLayout(5,5) );
    macProperty = new ngrs::NGroupBox();
      macProperty->setAlign(ngrs::nAlTop);
      macProperty->setHeaderText("Machine Properties");
      macProperty->setWidth(200);
      macProperty->setHeight(300);
      macProperty->setLayout(ngrs::NListLayout());
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

  tabBook_ = new ngrs::NTabBook();
    tabBook_->setAlign(ngrs::nAlClient);
    ngrs::NPanel* generatorPage = new ngrs::NPanel();
        generatorPage->setLayout(ngrs::NAlignLayout());
          generatorfBox_ = new ngrs::NListBox();
            generatorfBox_->setAlign(ngrs::nAlClient);
            generatorfBox_->itemSelected.connect(this,&NewMachine::onGeneratorItemSelected);
				std::map< PluginFinderKey, PluginInfo >::const_iterator it = finder.begin();
				for ( ; it != finder.end(); it++ ) {
					const PluginFinderKey & key = it->first;
					const PluginInfo & info = it->second;
					if ( info.type() == MACH_PLUGIN && info.mode() == MACHMODE_GENERATOR ) {
						ngrs::NItem* item = new ngrs::NItem( info.name() );
							item->mouseDoublePress.connect(this,&NewMachine::onItemDblClick);
							item->setIntValue( key.index() );
						generatorfBox_->add( item );
						pluginIdentify_[item] = key;
				}
		}
        generatorPage->add(generatorfBox_);

    ngrs::NPanel* effectPage = new ngrs::NPanel();
      effectPage->setLayout ( ngrs::NAlignLayout() );
      effectfBox_ = new ngrs::NListBox();
      effectfBox_->setAlign(ngrs::nAlClient);
      effectfBox_->itemSelected.connect(this,&NewMachine::onEffectItemSelected);

      it = finder.begin();
      for ( ; it != finder.end(); it++ ) {
        const PluginFinderKey & key = it->first;
        const PluginInfo & info = it->second;
        if ( info.type() == MACH_PLUGIN && info.mode() == MACHMODE_FX ) {
         ngrs::NItem* item = new ngrs::NItem( info.name() );
         item->mouseDoublePress.connect(this,&NewMachine::onItemDblClick);
         effectfBox_->add( item );
         pluginIdentify_[item] = key;
        }					
      }
    effectPage->add( effectfBox_ , ngrs::nAlClient);


    tabBook_->addPage(generatorPage,"Generators");
    tabBook_->addPage(effectPage,"Effects");
    ngrs::NListBox* internalPage_ = new ngrs::NListBox();
    ngrs::NItem* item = new ngrs::NItem("Sampler");
    item->mouseDoublePress.connect(this,&NewMachine::onItemDblClick);
        internalPage_->add(item);
        internalPage_->itemSelected.connect(this,&NewMachine::onInternalItemSelected);
    tabBook_->addPage(internalPage_,"Internal");
	  
  	
		ngrs::NPanel* ladspaPage = new ngrs::NPanel();
			ladspaPage->setLayout(ngrs::NAlignLayout());
        ladspaBox_ = new ngrs::NListBox();
        ladspaBox_->itemSelected.connect(this,&NewMachine::onLADSPAItemSelected);

				it = finder.begin();
				for ( ; it != finder.end(); it++ ) {
					const PluginFinderKey & key = it->first;
					const PluginInfo & info = it->second;
					if ( info.type() == MACH_LADSPA ) {
						ngrs::NItem* item = new ngrs::NItem( info.name() );
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

void NewMachine::onOkBtn( ngrs::NButtonEvent * sender )
{
  doClose( true );
}

void NewMachine::onItemDblClick( ngrs::NButtonEvent * sender )
{
  doClose( true );
}

void NewMachine::onCancelBtn( ngrs::NButtonEvent * sender )
{
  doClose( false );
}

void NewMachine::onInternalItemSelected( ngrs::NItemEvent * ev )
{
  if (ev->text() == "Sampler") {
    selectedKey_ = PluginFinderKey::internalSampler();
  }
}

void NewMachine::onGeneratorItemSelected( ngrs::NItemEvent* ev )
{
  ngrs::NCustomItem* item = generatorfBox_->itemAt( generatorfBox_->selIndex() );
  setPlugin ( item );
}

void NewMachine::onEffectItemSelected( ngrs::NItemEvent* ev ) {
  ngrs::NCustomItem* item = effectfBox_->itemAt( effectfBox_->selIndex() );
  setPlugin( item );
}

void NewMachine::onLADSPAItemSelected( ngrs::NItemEvent* ev ) {
   ngrs::NCustomItem* item = ladspaBox_->itemAt( ladspaBox_->selIndex() );
   setPlugin( item );
}

void NewMachine::setPlugin( ngrs::NCustomItem* item ) {
    std::map< ngrs::NCustomItem*, PluginFinderKey >::iterator it;		
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

void NewMachine::onEffectTabChange( ngrs::NButtonEvent * ev )
{
  ngrs::NCustomItem* item = effectfBox_->itemAt( effectfBox_->selIndex() );
  if  (item) setPlugin ( item );
}

void NewMachine::onGeneratorTabChange( ngrs::NButtonEvent * ev )
{
  ngrs::NCustomItem* item = generatorfBox_->itemAt( generatorfBox_->selIndex() );
  if  (item) setPlugin ( item );
}

void NewMachine::onLADSPATabChange( ngrs::NButtonEvent * ev )
{
 ngrs::NCustomItem* item = ladspaBox_->itemAt( ladspaBox_->selIndex() );
  if  (item) setPlugin ( item );
}

void NewMachine::onInternalTabChange( ngrs::NButtonEvent * ev )
{
}

const PluginFinderKey & NewMachine::pluginKey() const {
	return selectedKey_;
}

}
}


