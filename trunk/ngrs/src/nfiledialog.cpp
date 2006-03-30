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
#include "nfiledialog.h"
#include "ntoolbarseparator.h"
#include "ngroupbox.h"
#include "napp.h"
#include "nconfig.h"

/* XPM */
const char* left_nav[] = {
/* width height num_colors chars_per_pixel */
"    37    37        3            1",
/* colors */
". c #000000",
"# c #0000fc",
"a c None",
/* pixels */
"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
"aaaaaaaaaaaaaaaaaa.aaaaaaaaaaaaaaaaaa",
"aaaaaaaaaaaaaaaaa..aaaaaaaaaaaaaaaaaa",
"aaaaaaaaaaaaaaaa.#.aaaaaaaaaaaaaaaaaa",
"aaaaaaaaaaaaaaa.##.aaaaaaaaaaaaaaaaaa",
"aaaaaaaaaaaaaa.###.aaaaaaaaaaaaaaaaaa",
"aaaaaaaaaaaaa.####.aaaaaaaaaaaaaaaaaa",
"aaaaaaaaaaaa.#####.............aaaaaa",
"aaaaaaaaaaa.##################.aaaaaa",
"aaaaaaaaaa.###################.aaaaaa",
"aaaaaaaaa.####################.aaaaaa",
"aaaaaaaa.#####################.aaaaaa",
"aaaaaaa.######################.aaaaaa",
"aaaaaaaa.#####################.aaaaaa",
"aaaaaaaaa.####################.aaaaaa",
"aaaaaaaaaa.###################.aaaaaa",
"aaaaaaaaaaa.##################.aaaaaa",
"aaaaaaaaaaaa.#####.............aaaaaa",
"aaaaaaaaaaaaa.####.aaaaaaaaaaaaaaaaaa",
"aaaaaaaaaaaaaa.###.aaaaaaaaaaaaaaaaaa",
"aaaaaaaaaaaaaaa.##.aaaaaaaaaaaaaaaaaa",
"aaaaaaaaaaaaaaaa.#.aaaaaaaaaaaaaaaaaa",
"aaaaaaaaaaaaaaaaa..aaaaaaaaaaaaaaaaaa",
"aaaaaaaaaaaaaaaaaa.aaaaaaaaaaaaaaaaaa",
"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
};

/* XPM */
const char* right_nav[] = {
/* width height num_colors chars_per_pixel */
"    37    37        3            1",
/* colors */
". c #000000",
"# c #0000fc",
"a c None",
/* pixels */
"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
"aaaaaaaaaaaaaaaaaaa.aaaaaaaaaaaaaaaaa",
"aaaaaaaaaaaaaaaaaaa..aaaaaaaaaaaaaaaa",
"aaaaaaaaaaaaaaaaaaa.#.aaaaaaaaaaaaaaa",
"aaaaaaaaaaaaaaaaaaa.##.aaaaaaaaaaaaaa",
"aaaaaaaaaaaaaaaaaaa.###.aaaaaaaaaaaaa",
"aaaaaaaaaaaaaaaaaaa.####.aaaaaaaaaaaa",
"aaaaaaa.............#####.aaaaaaaaaaa",
"aaaaaaa.##################.aaaaaaaaaa",
"aaaaaaa.###################.aaaaaaaaa",
"aaaaaaa.####################.aaaaaaaa",
"aaaaaaa.#####################.aaaaaaa",
"aaaaaaa.######################.aaaaaa",
"aaaaaaa.#####################.aaaaaaa",
"aaaaaaa.####################.aaaaaaaa",
"aaaaaaa.###################.aaaaaaaaa",
"aaaaaaa.##################.aaaaaaaaaa",
"aaaaaaa.............#####.aaaaaaaaaaa",
"aaaaaaaaaaaaaaaaaaa.####.aaaaaaaaaaaa",
"aaaaaaaaaaaaaaaaaaa.###.aaaaaaaaaaaaa",
"aaaaaaaaaaaaaaaaaaa.##.aaaaaaaaaaaaaa",
"aaaaaaaaaaaaaaaaaaa.#.aaaaaaaaaaaaaaa",
"aaaaaaaaaaaaaaaaaaa..aaaaaaaaaaaaaaaa",
"aaaaaaaaaaaaaaaaaaa.aaaaaaaaaaaaaaaaa",
"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
};

/* XPM */
const char *up_nav[] = {
/* width height num_colors chars_per_pixel */
"    37    37        3            1",
/* colors */
". c #000000",
"# c #0000fc",
"a c None",
/* pixels */
"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
"aaaaaaaaaaaaaaaaaa.aaaaaaaaaaaaaaaaaa",
"aaaaaaaaaaaaaaaaa.#.aaaaaaaaaaaaaaaaa",
"aaaaaaaaaaaaaaaa.###.aaaaaaaaaaaaaaaa",
"aaaaaaaaaaaaaaa.#####.aaaaaaaaaaaaaaa",
"aaaaaaaaaaaaaa.#######.aaaaaaaaaaaaaa",
"aaaaaaaaaaaaa.#########.aaaaaaaaaaaaa",
"aaaaaaaaaaaa.###########.aaaaaaaaaaaa",
"aaaaaaaaaaa.#############.aaaaaaaaaaa",
"aaaaaaaaaa.###############.aaaaaaaaaa",
"aaaaaaaaa.#################.aaaaaaaaa",
"aaaaaaaa.###################.aaaaaaaa",
"aaaaaaa.......#########.......aaaaaaa",
"aaaaaaaaaaaaa.#########.aaaaaaaaaaaaa",
"aaaaaaaaaaaaa.#########.aaaaaaaaaaaaa",
"aaaaaaaaaaaaa.#########.aaaaaaaaaaaaa",
"aaaaaaaaaaaaa.#########.aaaaaaaaaaaaa",
"aaaaaaaaaaaaa.#########.aaaaaaaaaaaaa",
"aaaaaaaaaaaaa.#########.aaaaaaaaaaaaa",
"aaaaaaaaaaaaa.#########.aaaaaaaaaaaaa",
"aaaaaaaaaaaaa.#########.aaaaaaaaaaaaa",
"aaaaaaaaaaaaa.#########.aaaaaaaaaaaaa",
"aaaaaaaaaaaaa.#########.aaaaaaaaaaaaa",
"aaaaaaaaaaaaa.#########.aaaaaaaaaaaaa",
"aaaaaaaaaaaaa...........aaaaaaaaaaaaa",
"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
};


NFileDialog::NFileDialog()
 : NWindow()
{
  NFile::cdHome();

  pane()->setSpacing(5,5,5,5);
  setModal(true);

  NSkin paneSkin;
  NApp::config()->setSkin(&paneSkin,"filedlgpane");
  pane()->setSkin(paneSkin);

  NPanel* navPanel = new NPanel();
    navPanel->setLayout(new NFlowLayout(nAlLeft));
    navPanel->setAlign(nAlTop);
    NImage* leftXpm  = new NImage();
       leftXpm->createFromXpmData(left_nav);
    NImage* rightXpm = new NImage();
       rightXpm->createFromXpmData(right_nav);
    NImage* upXpm = new NImage();
       upXpm->createFromXpmData(up_nav);
    NButton* upBtn = new NButton(upXpm,10,10);
      upBtn->clicked.connect(this,&NFileDialog::onUpBtnPressed);
    navPanel->add(upBtn);
    navPanel->add(new NButton(leftXpm,10,10));
    navPanel->add(new NButton(rightXpm,10,10));
    navPanel->add(new NLabel("Folder"));
    navPanel->add(new NButton("New"));
    navPanel->add(new NButton("Del"));
    navPanel->add(new NToolBarSeparator());
    navPanel->add(new NButton("Home"));
  pane()->add(navPanel);

  NGroupBox* dirPanel = new NGroupBox();
  dirPanel->setHeaderText("Parent Dir");
  dirPanel->setLayout(new NAlignLayout());
  dirPanel->setAlign(nAlLeft);
  dirPanel->setWidth(150);    
    dBox_ = new NFileListBox();
     dBox_->setMode(nDirs);
     dBox_->setDirectory(NFile::parentWorkingDir());
     dBox_->setAlign(nAlClient);
     dBox_->itemSelected.connect(this,&NFileDialog::onParentDirItemSelected);
    dirPanel->add(dBox_);
  pane()->add(dirPanel);

  NPanel* bPnl = new NPanel();
    bPnl->setAlign(nAlBottom);
    bPnl->setLayout(new NFlowLayout(nAlRight));
    NButton* okBtn = new NButton("Open");
      okBtn->clicked.connect(this,&NFileDialog::onOkBtn);
      okBtn->setFlat(false);
    bPnl->add(okBtn);
    NButton* cancelBtn = new NButton("Cancel");
      cancelBtn->clicked.connect(this,&NFileDialog::onCancelBtn);
      cancelBtn->setFlat(false);
    bPnl->add(cancelBtn);
  pane()->add(bPnl);

  NPanel* fiPnl = new NPanel();
    fiPnl->setAlign(nAlBottom);
    fiPnl->setLayout(new NFlowLayout(nAlLeft));
    fiPnl->add(new NLabel("Filters"));
    fiNameCtrl = new NComboBox();
    fiNameCtrl->setWidth(200);
    fiPnl->add(fiNameCtrl);
  pane()->add(fiPnl);


  NPanel* fPnl = new NPanel();
    fPnl->setAlign(nAlBottom);
    fPnl->setLayout(new NFlowLayout(nAlLeft));
    fPnl->add(new NLabel("File"));
    fNameCtrl = new NEdit();
    fNameCtrl->setWidth(200);
    fPnl->add(fNameCtrl);
  pane()->add(fPnl);

  NPanel* dPnl = new NPanel();
    dPnl->setAlign(nAlBottom);
    dPnl->setLayout(new NFlowLayout(nAlLeft));
    dPnl->add(new NLabel("Dir"));
    dNameCtrl = new NEdit();
    dNameCtrl->setWidth(200);
    dPnl->add(dNameCtrl);
  pane()->add(dPnl);

  NGroupBox* filePanel = new NGroupBox();
  filePanel->setAlign(nAlClient);
  filePanel->setHeaderText("Files");
     filePanel->setLayout(new NAlignLayout());
     filePanel->setAlign(nAlClient);
        fBox_ = new NFileListBox();
        fBox_->setMode(nFiles | nDirs);
        fBox_->setDirectory(NFile::workingDir());
        fBox_->itemSelected.connect(this,&NFileDialog::onItemSelected);
        fBox_->setAlign(nAlClient);
     filePanel->add(fBox_);
  pane()->add(filePanel);

  pane()->resize();

  do_Execute = false;

  setWidth(500);
}


NFileDialog::~NFileDialog()
{
}

bool NFileDialog::execute( )
{
  setVisible(true);
  return do_Execute;
}

void NFileDialog::onItemSelected( NItemEvent * ev )
{
  if (!fBox_->isDirItem()) {
    fNameCtrl->setText(fBox_->fileName());
    fNameCtrl->repaint();
  } else {
     NFile::cd(NFile::workingDir());
     NFile::cd(fBox_->fileName());
     setDirectory(NFile::workingDir());
  }
}

void NFileDialog::onParentDirItemSelected( NItemEvent * ev )
{
  if (dBox_->fileName()=="..") {
    setDirectory(NFile::parentWorkingDir());
  } else {
    dNameCtrl->setText(dBox_->fileName());
    fNameCtrl->repaint();
    NFile::cd(NFile::parentWorkingDir());
    NFile::cd(dBox_->fileName());
    fBox_->setDirectory(NFile::workingDir());
    fBox_->repaint();
  }
}


void NFileDialog::onOkBtn( NButtonEvent * sender )
{
  do_Execute = true;
  setVisible(false);
  setExitLoop(nDestroyWindow);
}

int NFileDialog::onClose( )
{
  return nDestroyWindow;
}

std::string NFileDialog::fileName( )
{
  return NFile::workingDir()+"/"+fNameCtrl->text();
}

void NFileDialog::onCancelBtn( NButtonEvent * sender )
{
  do_Execute = false;
  setVisible(false);
  setExitLoop(nDestroyWindow);
}

void NFileDialog::setVisible(bool on )
{
  setPositionToScreenCenter();
  NWindow::setVisible(on);
}

void NFileDialog::setDirectory( const std::string & dir )
{
  history.push_back(dir);
  NFile::cd(dir);
  fBox_->setDirectory(NFile::workingDir());
  dBox_->setDirectory(NFile::parentWorkingDir());
  dBox_->repaint();
  fBox_->repaint();
}

void NFileDialog::onUpBtnPressed( NButtonEvent * sender )
{
  NFile::cd("..");
  setDirectory(NFile::workingDir());
}

void NFileDialog::onLeftBtnPressed( NButtonEvent * sender )
{

}

void NFileDialog::onRightBtnPressed( NButtonEvent * sender ) {

}





