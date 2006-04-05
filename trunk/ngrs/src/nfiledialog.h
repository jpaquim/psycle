/***************************************************************************
 *   Copyright (C) 2005 by Stefan   *
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
#ifndef NFILEDIALOG_H
#define NFILEDIALOG_H

#include <nwindow.h>
#include "nfilelistbox.h"
#include "nflowlayout.h"
#include "nedit.h"
#include "nlabel.h"
#include "ncombobox.h"
#include "nregexp.h"

/**
@author Stefan
*/
class NFileDialog : public NWindow
{
public:
    NFileDialog();

    ~NFileDialog();


   bool execute();

   int onClose();

   std::string fileName();

   void setDirectory(const std::string & dir);

   virtual void setVisible(bool on);

   void addFilter(const std::string & name ,const std::string & regexp);

private:

   NPanel* toolPanel;

   bool do_Execute;
   NFileListBox* fBox_;
   NFileListBox* dBox_;
   NEdit* fNameCtrl;
   NEdit* dNameCtrl;
   NComboBox* fiNameCtrl;

   void onItemSelected(NItemEvent* ev);
   void onParentDirItemSelected(NItemEvent* ev);

   void onOkBtn(NButtonEvent* sender);
   void onCancelBtn(NButtonEvent* sender);

   void onUpBtnPressed(NButtonEvent* sender);
   void onLeftBtnPressed(NButtonEvent* sender);
   void onRightBtnPressed(NButtonEvent* sender);

   std::vector<std::string> history;
};

#endif
