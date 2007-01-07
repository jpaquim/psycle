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

#include "pluginfinder.h"
#include <ngrs/ndialog.h>
#include <ngrs/ntabbook.h>
#include <ngrs/nfilelistbox.h>
#include <ngrs/ngroupbox.h>
#include <ngrs/nlabel.h>
#include <ngrs/nbevelborder.h>

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
@author Stefan Nattkemper
*/

class NewMachine : public NDialog
{
public:
	NewMachine( const PluginFinder & finder );

    ~NewMachine();

	const PluginFinderKey & pluginKey() const;

	virtual int onClose();

private:

	std::string dllName_;

	PluginFinderKey selectedKey_;
	const PluginFinder &  finder_;

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
	void onItemDblClick(NButtonEvent* sender);	
	void onCancelBtn(NButtonEvent* sender);

	std::map< NCustomItem*, PluginFinderKey > pluginIdentify_;

	void setPlugin( NCustomItem* item );
	void onGeneratorTabChange( NButtonEvent* ev );
	void onEffectTabChange( NButtonEvent* ev );
	void onLADSPATabChange( NButtonEvent* ev );
	void onInternalTabChange( NButtonEvent* ev );

};

} // end of host namespace
} // end of psycle namespace

#endif
