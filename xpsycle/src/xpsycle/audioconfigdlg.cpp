/***************************************************************************
 *   Copyright (C) 2006 by Stefan Nattkemper   *
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
#include "audioconfigdlg.h"
#include "configuration.h"
#include <ngrs/nlistbox.h>
#include <ngrs/nitem.h>
#include <ngrs/nalignlayout.h>
#include <ngrs/nlabel.h>
#include <ngrs/nobjectinspector.h>

namespace psycle {
	namespace host	{	

		AudioConfigDlg::AudioConfigDlg( Configuration* cfg )
		  : NWindow( ),
				config_(cfg)
		{
			setTitle("Audio settings");
		
			setMinimumWidth(300);
			setMinimumHeight(200);

			pane()->setLayout( NAlignLayout( 5, 5 ) );

			NPanel* driverPanel = new NPanel();
				driverPanel->setLayout( NAlignLayout() );
				driverPanel->add(new NLabel("Select A Driver"), nAlTop );
				driverLbx = new NListBox( );
					driverLbx->setPreferredSize(100,100);
				driverPanel->add( driverLbx , nAlClient);
			pane()->add( driverPanel, nAlLeft );

			NPanel* btnPanel = new NPanel();
				btnPanel->setLayout ( NAlignLayout(5,5) );
				NButton* useBtn = new NButton("use");
					useBtn->setFlat( false );
				btnPanel->add( useBtn, nAlRight );
				NButton* saveBtn = new NButton("save");
					saveBtn->setFlat( false );
				btnPanel->add( saveBtn, nAlRight );
				NButton* cancelBtn = new NButton("cancel");
					cancelBtn->setFlat( false );
				btnPanel->add( cancelBtn, nAlRight );
			pane()->add( btnPanel, nAlBottom);

			pane()->add(new NLabel("Set Driver Properties"), nAlTop );

			objInspector_ = new NObjectInspector();
			pane()->add( objInspector_, nAlClient );
			

			initDriverBox();
		}

		AudioConfigDlg::~AudioConfigDlg()
		{
		}

		void AudioConfigDlg::initDriverBox( )
		{
      std::map<std::string, AudioDriver*> & driverMap =  config_->driverMap();
			std::map<std::string, AudioDriver*>::iterator it = driverMap.begin();
			for ( ; it != driverMap.end(); it++ ) {
				std::string driverName = it->first;
				driverLbx->add( new NItem( driverName ) );
			}
		}

		int AudioConfigDlg::onClose( )
		{
				setVisible(false);
  			return nHideWindow;
		}

	}
}




