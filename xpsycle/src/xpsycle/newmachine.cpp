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

namespace psycle {
	namespace host {

NewMachine::NewMachine( )
  : NWindow()
{
  sampler_ = false;

	id_ = MACH_UNDEFINED;
	pluginIndex_ = 0;

	setTitle("Add New Machine");

  setPosition(100,100,500,500);
  NFont fnt("Suse sans",8,nStraight | nMedium | nAntiAlias);
  pane()->setFont(fnt);

  pane()->setLayout(NBorderLayout());

  setModal(true);
  do_Execute = false;

  NPanel* bPnl = new NPanel();
    bPnl->setAlign(nAlBottom);
    bPnl->setLayout(NFlowLayout(nAlRight));
    NButton* okBtn = new NButton("Open");
      okBtn->clicked.connect(this,&NewMachine::onOkBtn);
      okBtn->setFlat(false);
    bPnl->add(okBtn);
    NButton* cancelBtn = new NButton("Cancel");
      cancelBtn->clicked.connect(this,&NewMachine::onCancelBtn);
      cancelBtn->setFlat(false);
    bPnl->add(cancelBtn);
  pane()->add(bPnl);

  NPanel* properties = new NPanel();
    properties->setPreferredSize(210,100);
    properties->setAlign(nAlRight);
    properties->setLayout(NAlignLayout());
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
  pane()->add(properties);


  tabBook_ = new NTabBook();
    tabBook_->setAlign(nAlClient);
    NPanel* generatorPage = new NPanel();
        generatorPage->setLayout(NAlignLayout());
          generatorfBox_ = new NFileListBox();
            generatorfBox_->addFilter(".so","!S*.so!S*");
            generatorfBox_->setMode(nFiles);
            generatorfBox_->setAlign(nAlClient);
            generatorfBox_->setDirectory(Global::pConfig()->pluginPath);
            generatorfBox_->setActiveFilter(".so");
            generatorfBox_->update();
            generatorfBox_->itemSelected.connect(this,&NewMachine::onGeneratorItemSelected);
        generatorPage->add(generatorfBox_);
    NPanel* effectPage = new NPanel();
    tabBook_->addPage(effectPage,"Effects");
    tabBook_->addPage(generatorPage,"Generators");
    NListBox* internalPage_ = new NListBox();
        internalPage_->add(new NItem("Sampler"));
        internalPage_->itemSelected.connect(this,&NewMachine::onInternalItemSelected);
    tabBook_->addPage(internalPage_,"Internal");

		finder.scanAll();

		NPanel* ladspaPage = new NPanel();
			ladspaPage->setLayout(NAlignLayout());
        ladspaBox_ = new NListBox();
        ladspaBox_->itemSelected.connect(this,&NewMachine::onLADSPAItemSelected);
				std::map< PluginFinderKey, PluginInfo >::const_iterator it = finder.begin();
				for ( ; it != finder.end(); it++ ) {
					PluginFinderKey key = it->first;
					PluginInfo info = it->second;
					if ( info.type() == MACH_LADSPA ) {
						NItem* item = new NItem( info.libName() );
						item->setIntValue( key.index() );
						ladspaBox_->add( item );
					}
				}			
      ladspaPage->add(ladspaBox_, nAlClient);
	tabBook_->addPage(ladspaPage,"ladspaPage");

  pane()->add(tabBook_);


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

void NewMachine::onGeneratorItemSelected( NItemEvent * ev )
{
  sampler_=false;
  Plugin plugin(0, 0 );
  std::cout << ev->item()->text() << std::endl;
  if (plugin.LoadDll(ev->item()->text())) {
    name->setText(plugin.GetName());
  //libName->setText(plugin.GetDllName());
    dllName_ = plugin.GetDllName();
    description->setText(std::string("Psycle Instrument by ")+ std::string(plugin.GetInfo()->Author));
    apiVersion->setText(stringify(plugin.GetInfo()->Version));
		id_ = MACH_PLUGIN;


    pane()->resize();
    pane()->repaint();
  }
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

bool NewMachine::sampler( )
{
  return sampler_;
}

void NewMachine::onInternalItemSelected( NItemEvent * ev )
{
  if (ev->text() == "Sampler") {
		sampler_= true; 
		id_ = MACH_SAMPLER;
	} else {
		sampler_ = false;
	}
}

void NewMachine::onLADSPAItemSelected(NItemEvent* ev) {
		NCustomItem* item = ladspaBox_->itemAt( ladspaBox_->selIndex() );
		
		LADSPAMachine plugin(0, 0 );
		if (plugin.loadDll( item->text(), item->intValue() ) ) {
			pluginIndex_ = item->intValue();
			std::cout << "settext" << std::endl;
			name->setText( plugin.label() );
			std::cout << "description" << std::endl;
			description->setText( plugin.GetName() );
			id_ = MACH_LADSPA;
			std::cout << "dllname" << std::endl;
			dllName_ = plugin.GetDllName();
		}
  		std::cout << "going out" << std::endl;

		pane()->resize();
    pane()->repaint();
}

Machine::id_type NewMachine::selectedType() const {
	return id_;
}

int NewMachine::pluginIndex() const {
  return pluginIndex_;
}

}
}
