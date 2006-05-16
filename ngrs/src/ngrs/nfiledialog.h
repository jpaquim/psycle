/***************************************************************************
 *   Copyright (C) 2005 by Stefan Nattkemper  *
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

#include <ngrs/nwindow.h>
#include "ngrs/nfilelistbox.h"
#include "ngrs/nflowlayout.h"
#include "ngrs/nregexp.h"

const int nSave = 1;
const int nLoad = 2;

class NCheckBox;
class NLabel;
class NEdit;
class NComboBox;
class NFrameBorder;

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

   void setMode(int mode); // save/load

private:

   int mode_;

   NPanel* toolPanel;

   bool do_Execute;
   NFileListBox* fBox_;
   NFileListBox* pdBox_;
   NFileListBox* dBox_;
   NEdit* fNameCtrl;
   NEdit* dNameCtrl;
   NComboBox* fiNameCtrl;
   NCheckBox* hBox;

   NButton* okBtn_;
   NFrameBorder* dirPanelBorder;

   void onItemSelected(NItemEvent* ev);
   void onDirItemSelected(NItemEvent* ev);
   void onParentDirItemSelected(NItemEvent* ev);

   void onOkBtn(NButtonEvent* sender);
   void onCancelBtn(NButtonEvent* sender);

   void onUpBtnPressed(NButtonEvent* sender);
   void onLeftBtnPressed(NButtonEvent* sender);
   void onRightBtnPressed(NButtonEvent* sender);

   void onHiddenCheckBoxClicked(NButtonEvent* ev);

   std::vector<std::string> history;
};

#endif
