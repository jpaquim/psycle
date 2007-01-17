/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by  Stefan Nattkemper  *
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
#ifndef FILEDIALOG_H
#define FILEDIALOG_H

#include "dialog.h"
#include "filelistbox.h"
#include "flowlayout.h"
#include "regexp.h"

namespace ngrs {

  const int nSave = 1;
  const int nLoad = 2;

  class Button;
  class NCheckBox;
  class Label;
  class Edit;
  class ComboBox;
  class FrameBorder;

  /**
  @author  Stefan
  */
  class FileDialog : public Dialog
  {
  public:
    FileDialog();

    ~FileDialog();

    std::string fileName() const;

    void setDirectory(const std::string & dir);

    void addFilter(const std::string & name ,const std::string & regexp);

    void setMode(int mode); // save/load

    virtual void setVisible( bool on );

  private:

    int mode_;

    Panel* toolPanel;

    FileListBox* fBox_;
    FileListBox* pdBox_;
    FileListBox* dBox_;
    Edit* fNameCtrl;
    Edit* dNameCtrl;
    ComboBox* fiNameCtrl;
    NCheckBox* hBox;
    Button* okBtn_;

    void onItemSelected(ItemEvent* ev);
    void onDirItemSelected(ItemEvent* ev);
    void onParentDirItemSelected(ItemEvent* ev);

    void onOkBtn(ButtonEvent* sender);
    void onCancelBtn(ButtonEvent* sender);

    void onUpBtnPressed(ButtonEvent* sender);
    void onLeftBtnPressed(ButtonEvent* sender);
    void onRightBtnPressed(ButtonEvent* sender);

    void onHiddenCheckBoxClicked(ButtonEvent* ev);

    std::vector<std::string> history;
  };

}

#endif
