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
#ifndef NEWMACHINE_H
#define NEWMACHINE_H

#include <ngrs/nwindow.h>
#include <ngrs/ntabbook.h>
#include <ngrs/nfilelistbox.h>
#include <ngrs/ngroupbox.h>
#include <ngrs/nlabel.h>
#include <ngrs/nbevelborder.h>
#include "machine.h"
#include "pluginfinder.h"

namespace psycle {
namespace host {

class Song;

class InfoLine : public NPanel {
public:

    InfoLine(const std::string & info) {
      add( infoLb = new NLabel(info) );
      add( textLb = new NLabel() );
      textLb->setWordWrap(true);
      textLb->setBorder(NBevelBorder(nNone,nLowered));
    }

    ~InfoLine() {
    }

    void setText(const std::string & info) { textLb->setText(info);}

    virtual int preferredWidth () const { return 200;}
    virtual int preferredHeight() const {
      return textLb->preferredHeight();
    }

    virtual void resize() {
      infoLb->setPosition(0,0,100,clientHeight());
      textLb->setSpacing(2,2,2,2);
      textLb->setPosition(100,0,clientWidth()-100,clientHeight());
    }

private:

    NLabel* infoLb;
    NLabel* textLb;

};

/**
@author Stefan
*/
class NewMachine : public NWindow
{
public:
    NewMachine( );

    ~NewMachine();

    virtual int onClose();

    bool execute();

    std::string getDllName();
    bool outBus();

		Machine::id_type selectedType() const;
		int pluginIndex() const;

private:

  std::string dllName_;

  bool do_Execute;

	PluginFinder finder;

	Machine::id_type id_;
  int pluginIndex_;

  InfoLine* name;
  InfoLine* libName;
  InfoLine* description;
  InfoLine* apiVersion;
  NGroupBox* macProperty;

  NTabBook* tabBook_;

  NListBox* generatorfBox_;
  NListBox* effectfBox_;
	NListBox* ladspaBox_;

  void onGeneratorItemSelected(NItemEvent* ev);
	void onEffectItemSelected(NItemEvent* ev);
  void onInternalItemSelected(NItemEvent* ev);
	void onLADSPAItemSelected(NItemEvent* ev);	

  void onOkBtn(NButtonEvent* sender);
  void onCancelBtn(NButtonEvent* sender);

	std::map< NCustomItem*, PluginFinderKey > pluginIdentify_;

	void setPlugin( NCustomItem* item );

  void onEffectTabChange( NButtonEvent* ev );
  void onGeneratorTabChange( NButtonEvent* ev );
  void onLADSPATabChange( NButtonEvent* ev );
  void onInternalTabChange( NButtonEvent* ev );

};

}
}

#endif
