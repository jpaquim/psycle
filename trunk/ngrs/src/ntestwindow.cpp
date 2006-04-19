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
#include "ntestwindow.h"
#include "napp.h"
#include "nautoscrolllayout.h"
#include "nline.h"
#include "ntoolbar.h"
#include "ntoolbarseparator.h"
#include "ncombobox.h"
#include "ngridlayout.h"
#include "ncheckbox.h"
#include "ncheckmenuitem.h"
#include "nitem.h"
#include "ngroupbox.h"
#include "nframeborder.h"
#include "nborderlayout.h"
#include "nspinbutton.h"
#include "nspinedit.h"
#include "n7segment.h"
#include "n7segdisplay.h"
#include "ndockpanel.h"
#include "nsplitbar.h"
#include "nflipbox.h"
#include "ncustomtreeview.h"
#include "ntreenode.h"
#include "nlabel.h"

const char * a_xpm[] = {
"12 6 2 1",
" 	c None",
".	c black",
"            ",
"  .......   ",
"   .....    ",
"    ...     ",
"     .      ",
"            "};

NTestWindow::NTestWindow()
 : NWindow()
{
  setPosition(0,0,1000,700);

  /*NCustomTreeView* view = new NCustomTreeView();
    NLabel* lb = new NLabel("Test");
    view->rootNode()->pane()->add(lb,nAlClient);

      for (int i = 0; i < 10; i++) {
        NTreeNode* tr = new NTreeNode();
        tr->pane()->add(new NLabel("sub"),nAlClient);
        view->rootNode()->childNodePane()->add(tr,nAlTop);
      }
  pane()->add(view);

  view->setPosition(10,50,200,200);*/


  //testMenu();
  testSegDisplay();

}


NTestWindow::~NTestWindow()
{
}


void NTestWindow::onQuit( NObject * sender )
{
  exit(0);
}

void NTestWindow::onSliderPosChanged( double v )
{
  int dy = (int) ( ((scrollWin->pane()->clientHeight() - pane()->clientHeight()) / ((double) slider->clientHeight())) * v);

  XMoveResizeWindow (NApp::system().dpy(),
                          scrollWin->win(),
                          0,-dy, 900, 2000);
  //printf("%f\n",v); fflush(stdout);
  //scrollWin->setLeft((int) v);
}

void NTestWindow::onOpen( NButtonEvent * ev )
{
  fDialog->execute();
}

void NTestWindow::testBorderLayout( )
{
  pane()->setLayout(new NBorderLayout(),true);

  NButton* button = new NButton("Button 1 (PAGE_START)");
  pane()->add(button, nAlTop);

  button = new NButton("Button 2 (CENTER)");
  button->setPreferredSize(10, 10);
  pane()->add(button, nAlClient);

  button = new NButton("Button 3 (LINE_START)");
  pane()->add(button, nAlLeft);

  button = new NButton("Long-Named Button 4 (PAGE_END)");
  pane()->add(button, nAlBottom);

  button = new NButton("5 (LINE_END)");
  pane()->add(button, nAlRight);

  pack();
}

void NTestWindow::testFrameBorder( )
{
  NPanel* pnl = new NPanel();
    pnl->setPosition(10,10,200,200);
    NFrameBorder* fr = new NFrameBorder();
    fr->setOval();
    pnl->setBorder(fr,true);
  pane()->add(pnl);
}

void NTestWindow::testLine( )
{
  NLine* line = new NLine();
    line->setPoints(NPoint(10,10),NPoint(100,100));
    line->setMoveable(NMoveable());
    fDialog = new NFileDialog();
    line->add(fDialog);
  pane()->add(line);
}

void NTestWindow::testMenu( )
{
  fDialog = new NFileDialog();
    fDialog->addFilter("*.psy [psy3 song format]","!S*.psy");
  add(fDialog);

 // NWindow* popupMenu_ = new NWindow();

  //NWindow* win1 = new NWindow();

  NMenuBar* menuBar_ = new NMenuBar();
  menuBar_->setName("menuBar_");
  pane()->add(menuBar_);
  NMenu* menu1 = new NMenu("Bearbeiten",'b');
  menuBar_->add(menu1);

  /*NPanel* test = test = new NPanel();
    test->skin_.gradientStyle=1;
    test->skin_.gradStartColor = NColor(200,200,200);
    test->setHeight(20);
    test->setLayout(new NFlowLayout(nAlLeft));
  pane()->add(test, nAlTop);
  test->add(new NButton("Hallo"));*/



  NMenuItem* item = new NMenuItem("open");
    //NMenu* menu = new NMenu();
    //item->add(menu);
  menu1->add(item);
  item->click.connect(this,&NTestWindow::onOpen);

  NCheckMenuItem* item1 = new NCheckMenuItem("test");
  menu1->add(item1);
}

void NTestWindow::testSpinButton( )
{
   NSpinButton* btn = new NSpinButton();
     std::cout << btn->preferredWidth() << std::endl;
     btn->setPosition(10,10,btn->preferredWidth(),btn->preferredHeight());
   pane()->add(btn);
}

void NTestWindow::testEdit( )
{
  NEdit* edit = new NEdit();
    edit->setPosition(10,10,200,20);
  pane()->add(edit);

}

void NTestWindow::testSpinEdit( )
{
  NSpinEdit* edit = new NSpinEdit();
    edit->setPosition(10,10,200,20);
  pane()->add(edit);
}

void NTestWindow::toolBarTest( )
{

  std::string iconPath = "~/xpsycle/icons/";

  /*NPanel* toolBarPanel_ = new NPanel();
  NFlowLayout* fl = new NFlowLayout(nAlLeft,0,0);
    toolBarPanel_->setLayout(fl);
    toolBarPanel_->setWidth(500);
    toolBarPanel_->setAlign(nAlTop);*/

  NToolBar* toolBar1_ = new NToolBar();
  pane()->add(toolBar1_,nAlTop); //toolBar1_->add(lb);

    NButton* lb = new NButton("Hallo");
    std::cout << "this" << lb->preferredHeight() << std::endl;
    toolBar1_->add(lb);
    std::cout << "after" << lb->preferredHeight() << std::endl; 

    std::cout << toolBar1_->preferredHeight() << std::endl;
    std::cout << "y" << lb->preferredHeight() << std::endl;

    NImage* img;
    img = new NImage();
    img->loadFromFile(iconPath+ "new.xpm");
    img->setPreferredSize(40,80);
    toolBar1_->add(new NButton(img));

    img = new NImage();
    img->loadFromFile(iconPath+ "new.xpm");
    toolBar1_->add(new NButton(img));

    img = new NImage();
    img->loadFromFile(iconPath+ "new.xpm");
    toolBar1_->add(new NButton(img));
   //toolBarPanel_->add(toolBar1_);
   //toolBar1_->resize();
  
  //pane()->add(toolBarPanel_);
  pane()->resize();
}

void NTestWindow::testListBox( )
{
  NListBox* lbox = new NListBox();
     lbox->setMultiSelect(true);
     lbox->setPosition(10,10,200,200);
     for (int i = 0; i < 100; i++) {
        lbox->add(new NItem(stringify(i)));
     }
  pane()->add(lbox);
}

void NTestWindow::testSegDisplay( )
{
  N7SegDisplay* disp = new N7SegDisplay();
   pane()->setPosition(0,0,200,200);
  pane()->add(disp);
}


