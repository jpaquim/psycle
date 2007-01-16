/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by Stefan Nattkemper   *
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
#include "filedialog.h"
#include "toolbarseparator.h"
#include "groupbox.h"
#include "app.h"
#include "config.h"
#include "item.h"
#include "checkbox.h"
#include "label.h"
#include "edit.h"
#include "button.h"
#include "combobox.h"
#include "frameborder.h"
#include "splitbar.h"
#include "alignlayout.h"

namespace ngrs {

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
    "                    "
  };


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
    "                    "
  };

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
    "                    "
  };


  FileDialog::FileDialog()
    : Dialog()
  {
    setName("fdlg");

    mode_ = nLoad;

    setMinimumHeight(400);
    setMinimumWidth(300);

    pane()->setLayout(AlignLayout(5,5));

    setTitle("Load");

    ngrs::File::cdHome();

    pane()->setSpacing(5,5,5,5);

    Skin paneSkin;
    paneSkin = App::config()->skin("filedlgpane");
    pane()->setSkin(paneSkin);


    Panel* navPanel = new Panel();
    FlowLayout navLayout(nAlLeft);
    navLayout.setBaseLine(nAlCenter);
    navPanel->setLayout(navLayout);
    Image* leftXpm  = new Image();
    //       leftXpm->createFromXpmData(left_nav);
    Image* rightXpm = new Image();
    //rightXpm->createFromXpmData(right_nav);
    Image* upXpm = new Image();
    //       upXpm->createFromXpmData(up_nav);
    Button* upBtn = new Button(upXpm,10,10);
    upBtn->clicked.connect(this,&FileDialog::onUpBtnPressed);
    navPanel->add(upBtn);
    navPanel->add(new Button(leftXpm,10,10));
    navPanel->add(new Button(rightXpm,10,10));
    navPanel->add(new Label("Folder"));
    navPanel->add(new Button("New"));
    navPanel->add(new Button("Del"));
    navPanel->add(new ToolBarSeparator());
    navPanel->add(new Button("Home"));
    pane()->add(navPanel,nAlTop);

    Panel* mainPnl = new Panel();
    mainPnl->setLayout(AlignLayout(5,5));

    Panel* dirPanel = new GroupBox("Directory");
    dirPanel->setLayout(AlignLayout());
    dirPanel->setPreferredSize(180,100);
    dirPanel->add(new Label("Parent"),nAlTop);
    pdBox_ = new FileListBox();
    pdBox_->setBorder(FrameBorder());
    pdBox_->setMode(nDirs);
    pdBox_->setDirectory(ngrs::File::parentWorkingDir());
    pdBox_->setAlign(nAlTop);
    pdBox_->setHeight(100);
    pdBox_->itemSelected.connect(this,&FileDialog::onParentDirItemSelected);
    dirPanel->add(pdBox_);
    SplitBar* splitBar = new SplitBar(nHorizontal);
    dirPanel->add(splitBar,nAlTop);
    dirPanel->add(new Label("Working"),nAlTop);
    dBox_ = new FileListBox();
    dBox_->setBorder(FrameBorder());
    dBox_->setMode(nDirs);
    dBox_->setDirectory(ngrs::File::workingDir());
    dBox_->itemSelected.connect(this,&FileDialog::onDirItemSelected);
    dirPanel->add(dBox_, nAlClient);
    mainPnl->add(dirPanel,nAlLeft);

    splitBar = new SplitBar();
    mainPnl->add(splitBar,nAlLeft);

    Panel* bPnl = new Panel();
    bPnl->setAlign(nAlBottom);
    bPnl->setLayout(FlowLayout(nAlRight));
    okBtn_ = new Button("open");
    okBtn_->clicked.connect(this,&FileDialog::onOkBtn);
    okBtn_->setFlat(false);
    bPnl->add(okBtn_);
    Button* cancelBtn = new Button("cancel");
    cancelBtn->clicked.connect(this,&FileDialog::onCancelBtn);
    cancelBtn->setFlat(false);
    bPnl->add(cancelBtn);
    mainPnl->add(bPnl);

    Panel* fiPnl = new Panel();
    fiPnl->setLayout(AlignLayout(5,0));
    fiPnl->add(new Label("Filters"),nAlLeft);
    fiNameCtrl = new ComboBox();
    fiNameCtrl->setWidth(200);
    fiPnl->add(fiNameCtrl,nAlClient);
    mainPnl->add(fiPnl,nAlBottom);

    Panel* fPnl = new Panel();
    fPnl->setLayout(AlignLayout(5,0));
    fPnl->add(new Label("File"),nAlLeft);
    fNameCtrl = new Edit();
    fNameCtrl->setWidth(200);
    fPnl->add(fNameCtrl,nAlClient);
    mainPnl->add(fPnl,nAlBottom);

    Panel* dPnl = new Panel();
    dPnl->setLayout(AlignLayout(5,0));
    dPnl->add(new Label("Dir"),nAlLeft);
    dNameCtrl = new Edit();
    dNameCtrl->setWidth(200);
    dPnl->add(dNameCtrl,nAlClient);
    mainPnl->add(dPnl,nAlBottom);

    Panel* hPnl = new Panel();
    hPnl->setLayout(AlignLayout(5,0));
    hBox = new NCheckBox("show hidden Files");
    hBox->clicked.connect(this,&FileDialog::onHiddenCheckBoxClicked);
    hPnl->add(hBox,nAlClient);
    mainPnl->add(hPnl,nAlBottom);


    GroupBox* filePanel = new GroupBox();
    filePanel->setHeaderText("Files");
    filePanel->setLayout(AlignLayout());
    fBox_ = new FileListBox();
    fBox_->setMode(nFiles);
    fBox_->setDirectory(ngrs::File::workingDir());
    fBox_->itemSelected.connect(this,&FileDialog::onItemSelected);
    fBox_->setAlign(nAlClient);
    filePanel->add(fBox_,nAlClient);
    mainPnl->add(filePanel,nAlClient);

    pane()->add(mainPnl,nAlClient);

    pane()->resize();

    setPosition(0,0,550,400);
  }

  FileDialog::~FileDialog()
  {
    std::cout << "delete fdialog" << std::endl;
  }

  void FileDialog::onItemSelected( ItemEvent * ev )
  {
    if (!fBox_->isDirItem()) {
      fNameCtrl->setText(fBox_->fileName());
      fNameCtrl->repaint();
    }
  }

  void FileDialog::onDirItemSelected( ItemEvent * ev )
  {
    ngrs::File::cd(ngrs::File::workingDir());
    ngrs::File::cd(dBox_->fileName());
    setDirectory(ngrs::File::workingDir());
  }

  void FileDialog::onParentDirItemSelected( ItemEvent * ev )
  {
    if (pdBox_->fileName()=="..") {
      setDirectory(ngrs::File::parentWorkingDir());
    } else {
      dNameCtrl->setText(pdBox_->fileName());
      fNameCtrl->repaint();
      ngrs::File::cd(ngrs::File::parentWorkingDir());
      ngrs::File::cd(pdBox_->fileName());
      fBox_->setDirectory(ngrs::File::workingDir());
      dBox_->setDirectory(ngrs::File::workingDir());
      fBox_->repaint();
      dBox_->repaint();
    }
  }


  void FileDialog::onOkBtn( ButtonEvent * sender )
  {
    doClose(true);
  }

  std::string FileDialog::fileName( ) const
  {
    return ngrs::File::workingDir()+"/"+fNameCtrl->text();
  }

  void FileDialog::onCancelBtn( ButtonEvent * sender )
  {
    doClose(false);
  }

  void FileDialog::setDirectory( const std::string & dir )
  {
    history.push_back(dir);
    ngrs::File::cd(dir);
    fBox_->setDirectory(ngrs::File::workingDir());
    pdBox_->setDirectory(ngrs::File::parentWorkingDir());
    dBox_->setDirectory(ngrs::File::workingDir());
    if ( mapped() ) {
      fBox_->update();
      pdBox_->update();
      dBox_->update();
    }
    dBox_->repaint();
    pdBox_->repaint();
    fBox_->repaint();
  }

  void FileDialog::onUpBtnPressed( ButtonEvent * sender )
  {
    ngrs::File::cd("..");
    setDirectory(ngrs::File::workingDir());
  }

  void FileDialog::onLeftBtnPressed( ButtonEvent * sender )
  {

  }

  void FileDialog::onRightBtnPressed( ButtonEvent * sender ) {

  }

  void FileDialog::addFilter( const std::string & name, const std::string & regexp )
  {
    fBox_->addFilter(name,regexp);
    fiNameCtrl->add(new Item(name));
    fBox_->setActiveFilter(name);
    fiNameCtrl->setIndex(fiNameCtrl->itemCount()-1);
  }

  void FileDialog::setMode( int mode )
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

  void FileDialog::onHiddenCheckBoxClicked( ButtonEvent * ev )
  {
    fBox_->setShowHiddenFiles(hBox->checked());
    dBox_->setShowHiddenFiles(hBox->checked());
    pdBox_->setShowHiddenFiles(hBox->checked());
    pane()->repaint();
  }

  void FileDialog::setVisible( bool on )
  {
    if (on) {
      fBox_->update();
      pdBox_->update();
      dBox_->update();
    } else {
      //fBox_->removeChilds();
      //dBox_->removeChilds();
    }
    Dialog::setVisible(on);
  }

}


// the class factories
extern "C" ngrs::Object* createFileDialog() {
  return new ngrs::FileDialog();
}

extern "C" void destroyFileDialog( ngrs::Object* p ) {
  delete p;
}
