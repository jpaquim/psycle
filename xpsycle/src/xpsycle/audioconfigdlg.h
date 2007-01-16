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
#ifndef AUDIOCONFIGDLG_H
#define AUDIOCONFIGDLG_H

#include <ngrs/window.h>
#include <ngrs/tabbook.h>
#include <ngrs/groupbox.h>
#include <ngrs/combobox.h>
#include <ngrs/notebook.h>
#include <ngrs/edit.h>

/**
@author Stefan Nattkemper
*/

class NListBox;
class ObjectInspector;
class ItemEvent;

namespace psycle {
	namespace host	{	

		class Configuration;
		class AudioDriver;

        class AudioConfigDlg : public ngrs::Window {
		public:
				AudioConfigDlg( Configuration* cfg );

				~AudioConfigDlg();

				virtual int onClose();
				virtual void setVisible( bool on );

		private:

                ngrs::TabBook* tabBook_;
                ngrs::Panel* audioPage_;
                ngrs::Panel* midiPage_;

                ngrs::GroupBox* driverBox_;
                ngrs::ComboBox* driverCbx_; // contains the drivers
                ngrs::Button* restartBtn_; // re/starts the driver

                ngrs::Button* cancelBtn_;
                ngrs::Button* closeBtn_;
                ngrs::Button* okBtn_;

				ngrs::Label* audioHeaderLbl_;
				ngrs::Label* audioDescriptionLbl_;

				ngrs::NoteBook* noteBook_; // contains different pages for drivers
				ngrs::Panel* jackPage_; // special page for jack
				ngrs::Panel* esdPage_;  // special page for esound
				ngrs::Panel* generalPage_;   // general page for drivers to set bit/rate etc ..
				ngrs::Edit* deviceEdt_;
				ngrs::ComboBox* sampleRateCbx_;
				ngrs::ComboBox* bitDepthCbx_;
				ngrs::ComboBox* channelModeCbx_;

				Configuration* config_;
				AudioDriver* selectedDriver_;

				void init();
				void initAudioDriverBox();
				void initDriverList();
				void initJackPage();
				void initEsdPage();
				void initGeneralPage();

				void updateGeneralPage();

				void onDriverSelected( ngrs::ItemEvent* ev );
				void onRestartDriver( ngrs::ButtonEvent* ev );

				void onOkBtn( ngrs::ButtonEvent* ev );
				void onCancelBtn( ngrs::ButtonEvent* ev );
				void onCloseBtn( ngrs::ButtonEvent* ev );

				void onChannelCbx( ngrs::ItemEvent* ev );
				void onSampleRateCbx( ngrs::ItemEvent* ev );
				void onDepthCbx( ngrs::ItemEvent* ev );


		};

	}
}

#endif
