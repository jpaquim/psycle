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
#include <nborderlayout.h>
#include <nlabel.h>

NewMachine::NewMachine()
 : NWindow()
{
  setPosition(100,100,500,500);
  NFont fnt("Suse sans",8,nStraight | nMedium | nAntiAlias);
  pane()->setFont(fnt);

  pane()->setLayout(new NBorderLayout());

  setModal(true);
  do_Execute = false;

  NPanel* bPnl = new NPanel();
    bPnl->setAlign(nAlBottom);
    bPnl->setLayout(new NFlowLayout(nAlRight));
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
    properties->setWidth(210);
    properties->setAlign(nAlRight);
    macProperty = new NGroupBox();
      macProperty->setAlign(nAlTop);
      macProperty->setHeaderText("Machine Properties");
      macProperty->setWidth(200);
      macProperty->setHeight(300);
      macProperty->setLayout(new NListLayout());
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
    NPage* generatorPage = new NPage();
       generatorPage->setLayout(new NAlignLayout());
         generatorfBox_ = new NFileListBox();
           generatorfBox_->addFilter(".so","lib-psycle.plugin.!S*.so");
           generatorfBox_->setMode(nFiles);
           generatorfBox_->setAlign(nAlClient);
           generatorfBox_->setDirectory(Global::pConfig()->pluginPath);
           generatorfBox_->setActiveFilter(".so");
           generatorfBox_->itemSelected.connect(this,&NewMachine::onGeneratorItemSelected);
       generatorPage->add(generatorfBox_);
    NPage* effectPage = new NPage();
    tabBook_->addPage(effectPage,"Effects");
    tabBook_->addPage(generatorPage,"Generators");
  pane()->add(tabBook_);

}


NewMachine::~NewMachine()
{
}

int NewMachine::onClose( )
{
  setVisible(false);
}

void NewMachine::onGeneratorItemSelected( NItemEvent * ev )
{
  Plugin plugin(0);
  std::cout << ev->item()->text() << std::endl;
  if (plugin.LoadDll(ev->item()->text())) {;
    plugin.GetName();
    name->setText(plugin.GetName());
  //libName->setText(plugin.GetDllName());
    dllName_ = plugin.GetDllName();
    description->setText(std::string("Psycle Instrument by \n")+ std::string(plugin.GetInfo()->Author));
    apiVersion->setText(stringify(plugin.GetInfo()->Version));
    macProperty->resize();
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


