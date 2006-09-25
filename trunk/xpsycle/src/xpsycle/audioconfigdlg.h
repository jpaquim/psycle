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

		class AudioConfigDlg : public NWindow {
		public:
				AudioConfigDlg( Configuration* cfg );

				~AudioConfigDlg();

				virtual int onClose();
				virtual void setVisible( bool on );

		private:

				NTabBook* tabBook_;
				NPanel* audioPage_;
				NPanel* midiPage_;

				NGroupBox* driverBox_;
				NComboBox* driverCbx_; // contains the drivers
				NButton* restartBtn_; // re/starts the driver

				NButton* cancelBtn_;
				NButton* okBtn_;

				NLabel* audioHeaderLbl_;
				NLabel* audioDescriptionLbl_;

				NNoteBook* noteBook_; // contains different pages for drivers
				NPanel* jackPage_; // special page for jack
				NPanel* esdPage_;  // special page for esound
				NPanel* generalPage_;   // general page for drivers to set bit/rate etc ..
				NEdit* deviceEdt_;
				NComboBox* sampleRateCbx_;
				NComboBox* bitDepthCbx_;
				NComboBox* channelModeCbx_;

				Configuration* config_;
				AudioDriver* selectedDriver_;

				void init();
				void initAudioDriverBox();
				void initDriverList();
				void initJackPage();
				void initEsdPage();
				void initGeneralPage();

				void updateGeneralPage();

				void onDriverSelected( NItemEvent* ev );
				void onRestartDriver( NButtonEvent* ev );

				void onOkBtn( NButtonEvent* ev );
				void onCancelBtn( NButtonEvent* ev );

				void onChannelCbx( NItemEvent* ev );
				void onSampleRateCbx( NItemEvent* ev );
				void onDepthCbx( NItemEvent* ev );


		};

	}
}

#endif
