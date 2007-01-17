/***************************************************************************
*   Copyright (C) 2006 by  Stefan   *
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
#include <ngrs/dialog.h>
#include <ngrs/tabbook.h>
#include <ngrs/filelistbox.h>
#include <ngrs/groupbox.h>
#include <ngrs/label.h>
#include <ngrs/bevelborder.h>

namespace psycle {
  namespace host {

    class Song;

    class InfoLine : public ngrs::Panel {
    public:
      InfoLine( const std::string& info ) {
        add( infoLb = new ngrs::Label(info) );
        add( textLb = new ngrs::Label() );
        textLb->setWordWrap( true );
        textLb->setBorder( ngrs::BevelBorder( ngrs::nNone, ngrs::nLowered ) );
      }

      ~InfoLine() {
      }

      void setText( const std::string& info ) { 
        textLb->setText(info);
      }

      virtual int preferredWidth () const { 
        return 200;
      }

      virtual int preferredHeight() const {
        return textLb->preferredHeight();
      }

      virtual void resize() {
        infoLb->setPosition( 0, 0, 100, clientHeight() );
        textLb->setSpacing( 2, 2, 2, 2 );
        textLb->setPosition( 100, 0, clientWidth()-100, clientHeight() );
      }

    private:

      ngrs::Label* infoLb;
      ngrs::Label* textLb;

    };

    /**
    @author  Stefan Nattkemper
    */

    class NewMachine : public ngrs::Dialog
    {
    public:
      NewMachine( const PluginFinder& finder );

      ~NewMachine();

      const PluginFinderKey& pluginKey() const;

      virtual int onClose();

    private:

      std::string dllName_;

      PluginFinderKey selectedKey_;
      const PluginFinder& finder_;

      InfoLine* name;
      InfoLine* libName;
      InfoLine* description;
      InfoLine* apiVersion;
      ngrs::GroupBox* macProperty;

      ngrs::TabBook* tabBook_;

      ngrs::ListBox* generatorfBox_;
      ngrs::ListBox* effectfBox_;
      ngrs::ListBox* ladspaBox_;

      void onGeneratorItemSelected( ngrs::ItemEvent* ev );
      void onEffectItemSelected( ngrs::ItemEvent* ev );
      void onInternalItemSelected( ngrs::ItemEvent* ev );
      void onLADSPAItemSelected( ngrs::ItemEvent* ev );

      void onOkBtn( ngrs::ButtonEvent* sender );
      void onItemDblClick( ngrs::ButtonEvent* sender );	
      void onCancelBtn( ngrs::ButtonEvent* sender );

      std::map< ngrs::CustomItem*, PluginFinderKey > pluginIdentify_;

      void setPlugin( ngrs::CustomItem* item );
      void onGeneratorTabChange( ngrs::ButtonEvent* ev );
      void onEffectTabChange( ngrs::ButtonEvent* ev );
      void onLADSPATabChange( ngrs::ButtonEvent* ev );
      void onInternalTabChange( ngrs::ButtonEvent* ev );

    };

  } // end of host namespace
} // end of psycle namespace

#endif
