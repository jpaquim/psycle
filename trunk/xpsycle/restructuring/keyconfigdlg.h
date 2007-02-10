/***************************************************************************
 *   Copyright (C) 2006 by  Stefan Nattkemper   *
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
#ifndef KeyConfigDlg_H
#define KeyConfigDlg_H

#include <ngrs/window.h>
#include <ngrs/tabbook.h>
#include <ngrs/groupbox.h>
#include <ngrs/combobox.h>
#include <ngrs/notebook.h>
#include <ngrs/edit.h>
#include <ngrs/listbox.h>

/**
@author  Stefan Nattkemper
*/

class ListBox;
class ObjectInspector;
class ItemEvent;

namespace psy {
	namespace host	{	

        class KeyConfigDlg : public ngrs::Window {
		public:
				KeyConfigDlg( );

				~KeyConfigDlg();

				virtual int onClose();
				virtual void setVisible( bool on );

		private:

                ngrs::GroupBox* actionsBox_;
                ngrs::TabBook* actionsTabbook_;
                ngrs::Panel* macPage_;
                ngrs::Panel* patPage_;
                ngrs::Panel* wavPage_;
                ngrs::Panel* seqPage_;
                ngrs::Panel* genPage_;
                ngrs::ListBox* macList_;
                ngrs::ListBox* patList_;
                ngrs::ListBox* wavList_;
                ngrs::ListBox* seqList_;
                ngrs::ListBox* genList_;

                ngrs::Panel* bindPnl_;
                ngrs::Label* bindLbl_;
                ngrs::Edit* bindEdit_;
                ngrs::Button* bindClearBtn_;

                ngrs::GroupBox* presetsBox_;
                ngrs::Panel* prsBtnPnl_;
                ngrs::Button* prsDefaultsBtn_;
                ngrs::Button* prsLoadBtn_;
                ngrs::Button* prsSaveBtn_;

                ngrs::Button* okBtn_;
                ngrs::Button* applyBtn_;
                ngrs::Button* cancelBtn_;

				void init();
				void initGeneralPage();

				void onOkBtn( ngrs::ButtonEvent* ev );
				void onApplyBtn( ngrs::ButtonEvent* ev );
				void onCancelBtn( ngrs::ButtonEvent* ev );

		};

	}
}

#endif
