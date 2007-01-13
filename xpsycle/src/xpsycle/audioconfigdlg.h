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

#include <ngrs/nwindow.h>
#include <ngrs/ntabbook.h>
#include <ngrs/ngroupbox.h>
#include <ngrs/ncombobox.h>
#include <ngrs/nnotebook.h>
#include <ngrs/nedit.h>

/**
@author Stefan Nattkemper
*/

class NListBox;
class NObjectInspector;
class NItemEvent;

namespace psycle {
	namespace host	{	

		class Configuration;
		class AudioDriver;

        class AudioConfigDlg : public ngrs::NWindow {
		public:
				AudioConfigDlg( Configuration* cfg );

				~AudioConfigDlg();

				virtual int onClose();
				virtual void setVisible( bool on );

		private:

                ngrs::NTabBook* tabBook_;
                ngrs::NPanel* audioPage_;
                ngrs::NPanel* midiPage_;

                ngrs::NGroupBox* driverBox_;
                ngrs::NComboBox* driverCbx_; // contains the drivers
                ngrs::NButton* restartBtn_; // re/starts the driver

                ngrs::NButton* cancelBtn_;
                ngrs::NButton* closeBtn_;
                ngrs::NButton* okBtn_;

				ngrs::NLabel* audioHeaderLbl_;
				ngrs::NLabel* audioDescriptionLbl_;

				ngrs::NNoteBook* noteBook_; // contains different pages for drivers
				ngrs::NPanel* jackPage_; // special page for jack
				ngrs::NPanel* esdPage_;  // special page for esound
				ngrs::NPanel* generalPage_;   // general page for drivers to set bit/rate etc ..
				ngrs::NEdit* deviceEdt_;
				ngrs::NComboBox* sampleRateCbx_;
				ngrs::NComboBox* bitDepthCbx_;
				ngrs::NComboBox* channelModeCbx_;

				Configuration* config_;
				AudioDriver* selectedDriver_;

				void init();
				void initAudioDriverBox();
				void initDriverList();
				void initJackPage();
				void initEsdPage();
				void initGeneralPage();

				void updateGeneralPage();

				void onDriverSelected( ngrs::NItemEvent* ev );
				void onRestartDriver( ngrs::NButtonEvent* ev );

				void onOkBtn( ngrs::NButtonEvent* ev );
				void onCancelBtn( ngrs::NButtonEvent* ev );
				void onCloseBtn( ngrs::NButtonEvent* ev );

				void onChannelCbx( ngrs::NItemEvent* ev );
				void onSampleRateCbx( ngrs::NItemEvent* ev );
				void onDepthCbx( ngrs::NItemEvent* ev );


		};

	}
}

#endif
