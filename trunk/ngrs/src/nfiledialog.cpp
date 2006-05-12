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
#include "nitem.h"
#include "ncheckbox.h"
#include "nlabel.h"
#include "nedit.h"
#include "ncombobox.h"
#include "nframeborder.h"
#include "nsplitbar.h"
#include "nalignlayout.h"

/* XPM */
const char * left_nav[] = {
"20 20 15 1",
" 	c None",
".	c #FFF2DD",
"+	c #FED58F",
"@	c #FDB943",
"#	c #FCA715",
"$	c #FCAB1F",
"%	c #FDC86C",
"&	c #FEE7BE",
"*	c #FFFBF3",
"=	c #FEE1AE",
"-	c #FCA612",
";	c #FED793",
">	c #FFFAF1",
",	c #F4F4F4",
"'	c #3A3A3A",
"                    ",
"                    ",
"                    ",
"                    ",
"                    ",
"        '           ",
"       ''           ",
"      ''''''''      ",
"     '''''''''      ",
"    ''''''''''      ",
"     '''''''''      ",
"      ''''''''      ",
"       ''           ",
"        '           ",
"                    ",
"                    ",
"                    ",
"                    ",
"                    ",
"                    "};


/* XPM */
const char * right_nav[] = {
"20 20 15 1",
" 	c None",
".	c #FFF2DD",
"+	c #FED58F",
"@	c #FDB943",
"#	c #FCA715",
"$	c #FCAB1F",
"%	c #FDC86C",
"&	c #FEE7BE",
"*	c #FFFBF3",
"=	c #FEE1AE",
"-	c #FCA612",
";	c #FED793",
">	c #FFFAF1",
",	c #3A3A3A",
"'	c #F4F4F4",
"                    ",
"                    ",
"                    ",
"                    ",
"                    ",
"           ,        ",
"           ,,       ",
"      ,,,,,,,,      ",
"      ,,,,,,,,,     ",
"      ,,,,,,,,,,    ",
"      ,,,,,,,,,     ",
"      ,,,,,,,,      ",
"           ,,       ",
"           ,        ",
"                    ",
"                    ",
"                    ",
"                    ",
"                    ",
"                    "};




/* XPM */
const char * up_nav[] = {
"20 20 15 1",
" 	c None",
".	c #FFF2DD",
"+	c #FED58F",
"@	c #FDB943",
"#	c #FCA715",
"$	c #FCAB1F",
"%	c #FDC86C",
"&	c #FEE7BE",
"*	c #FFFBF3",
"=	c #FEE1AE",
"-	c #FCA612",
";	c #FED793",
">	c #FFFAF1",
",	c #F4F4F4",
"'	c #3A3A3A",
"                    ",
"                    ",
"                    ",
"                    ",
"          '         ",
"         '''        ",
"        '''''       ",
"       '''''''      ",
"      '''''''''     ",
"        '''''       ",
"        '''''       ",
"        '''''       ",
"        '''''       ",
"        '''''       ",
"                    ",
"                    ",
"                    ",
"                    ",
"                    ",
"                    "};


NFileDialog::NFileDialog()
 : NWindow()
{
  mode_ = nLoad;

  pane()->setLayout(NAlignLayout(5,5));

  setTitle("Load");

  NFile::cdHome();

  pane()->setSpacing(5,5,5,5);
  setModal(true);

  NSkin paneSkin;
  paneSkin = NApp::config()->skin("filedlgpane");
  pane()->setSkin(paneSkin);


  NPanel* navPanel = new NPanel();
    NFlowLayout navLayout(nAlLeft);
      navLayout.setBaseLine(nAlCenter);
    navPanel->setLayout(navLayout);
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
  pane()->add(navPanel,nAlTop);

  NPanel* mainPnl = new NPanel();
  mainPnl->setLayout(NAlignLayout(5,5));

  NPanel* dirPanel = new NPanel();
    dirPanel->setLayout(NAlignLayout());
    NFrameBorder dirPanelBorder;
    dirPanelBorder.setOval(true,5,5);
    dirPanelBorder.setColor(NColor(220,220,220));
    dirPanel->setBorder(dirPanelBorder);
      dirPanel->setPreferredSize(180,100);
      dirPanel->add(new NLabel("Parentdir"),nAlTop);
      pdBox_ = new NFileListBox();
        pdBox_->setBorder(NFrameBorder());
        pdBox_->setMode(nDirs);
        pdBox_->setHeight(100);
        pdBox_->setDirectory(NFile::parentWorkingDir());
        pdBox_->setAlign(nAlTop);
        pdBox_->itemSelected.connect(this,&NFileDialog::onParentDirItemSelected);
      dirPanel->add(pdBox_);
      NSplitBar* splitBar = new NSplitBar(nHorizontal);
      dirPanel->add(splitBar,nAlTop);
      dirPanel->add(new NLabel("Workingdir"),nAlTop);
      dBox_ = new NFileListBox();
        dBox_->setBorder(NFrameBorder());
        dBox_->setMode(nDirs);
        dBox_->setDirectory(NFile::workingDir());
        dBox_->itemSelected.connect(this,&NFileDialog::onDirItemSelected);
      dirPanel->add(dBox_, nAlClient);
  mainPnl->add(dirPanel,nAlLeft);

  splitBar = new NSplitBar();
  mainPnl->add(splitBar,nAlLeft);

  NPanel* bPnl = new NPanel();
    bPnl->setAlign(nAlBottom);
    bPnl->setLayout(NFlowLayout(nAlRight));
    okBtn_ = new NButton("open");
      okBtn_->clicked.connect(this,&NFileDialog::onOkBtn);
      okBtn_->setFlat(false);
    bPnl->add(okBtn_);
    NButton* cancelBtn = new NButton("cancel");
      cancelBtn->clicked.connect(this,&NFileDialog::onCancelBtn);
      cancelBtn->setFlat(false);
    bPnl->add(cancelBtn);
  mainPnl->add(bPnl);

  NPanel* fiPnl = new NPanel();
    fiPnl->setLayout(NAlignLayout(5,0));
    fiPnl->add(new NLabel("Filters"),nAlLeft);
    fiNameCtrl = new NComboBox();
    fiNameCtrl->setWidth(200);
    fiPnl->add(fiNameCtrl,nAlClient);
  mainPnl->add(fiPnl,nAlBottom);

  NPanel* fPnl = new NPanel();
    fPnl->setLayout(NAlignLayout(5,0));
    fPnl->add(new NLabel("File"),nAlLeft);
    fNameCtrl = new NEdit();
    fNameCtrl->setWidth(200);
    fPnl->add(fNameCtrl,nAlClient);
  mainPnl->add(fPnl,nAlBottom);

  NPanel* dPnl = new NPanel();
    dPnl->setLayout(NAlignLayout(5,0));
    dPnl->add(new NLabel("Dir"),nAlLeft);
    dNameCtrl = new NEdit();
    dNameCtrl->setWidth(200);
    dPnl->add(dNameCtrl,nAlClient);
  mainPnl->add(dPnl,nAlBottom);

  NPanel* hPnl = new NPanel();
    hPnl->setLayout(NAlignLayout(5,0));
    hBox = new NCheckBox("show hidden Files");
      hBox->clicked.connect(this,&NFileDialog::onHiddenCheckBoxClicked);
    hPnl->add(hBox,nAlClient);
  mainPnl->add(hPnl,nAlBottom);


  NGroupBox* filePanel = new NGroupBox();
  filePanel->setHeaderText("Files");
     filePanel->setLayout(NAlignLayout());
        fBox_ = new NFileListBox();
        fBox_->setMode(nFiles);
        fBox_->setDirectory(NFile::workingDir());
        fBox_->itemSelected.connect(this,&NFileDialog::onItemSelected);
        fBox_->setAlign(nAlClient);
     filePanel->add(fBox_,nAlClient);
  mainPnl->add(filePanel,nAlClient);

  pane()->add(mainPnl,nAlClient);

  pane()->resize();

  do_Execute = false;

  setPosition(0,0,550,400);
}


NFileDialog::~NFileDialog()
{
  delete dirPanelBorder;
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
  }
}

void NFileDialog::onDirItemSelected( NItemEvent * ev )
{
  NFile::cd(NFile::workingDir());
  NFile::cd(dBox_->fileName());
  setDirectory(NFile::workingDir());
}

void NFileDialog::onParentDirItemSelected( NItemEvent * ev )
{
  if (pdBox_->fileName()=="..") {
    setDirectory(NFile::parentWorkingDir());
  } else {
    dNameCtrl->setText(pdBox_->fileName());
    fNameCtrl->repaint();
    NFile::cd(NFile::parentWorkingDir());
    NFile::cd(pdBox_->fileName());
    fBox_->setDirectory(NFile::workingDir());
    dBox_->setDirectory(NFile::workingDir());
    fBox_->repaint();
    dBox_->repaint();
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
  do_Execute = false;
  setVisible(false);
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
  pdBox_->setDirectory(NFile::parentWorkingDir());
  dBox_->setDirectory(NFile::workingDir());
  dBox_->repaint();
  pdBox_->repaint();
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

void NFileDialog::addFilter( const std::string & name, const std::string & regexp )
{
  fBox_->addFilter(name,regexp);
  fiNameCtrl->add(new NItem(name));
  fBox_->setActiveFilter(name);
  fiNameCtrl->setIndex(fiNameCtrl->itemCount()-1);
}

void NFileDialog::setMode( int mode )
{
  mode_ = mode;
  if (mode == nLoad) {
    okBtn_->setText("load");
    setTitle("Load");
  } else {
    okBtn_->setText("save");
    setTitle("Save");
  }
}

void NFileDialog::onHiddenCheckBoxClicked( NButtonEvent * ev )
{
  fBox_->setShowHiddenFiles(hBox->checked());
  dBox_->setShowHiddenFiles(hBox->checked());
  pdBox_->setShowHiddenFiles(hBox->checked());
  pane()->repaint();
}




