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

#include <ngrs/napp.h>
#include <ngrs/nautoscrolllayout.h>
#include <ngrs/nline.h>
#include <ngrs/ntoolbar.h>
#include <ngrs/ntoolbarseparator.h>
#include <ngrs/ncombobox.h>
#include <ngrs/ngridlayout.h>
#include <ngrs/ncheckbox.h>
#include <ngrs/ncheckmenuitem.h>
#include <ngrs/nitem.h>
#include <ngrs/ngroupbox.h>
#include <ngrs/nframeborder.h>
#include <ngrs/nborderlayout.h>
#include <ngrs/nspinbutton.h>
#include <ngrs/nspinedit.h>
#include <ngrs/n7segment.h>
#include <ngrs/n7segdisplay.h>
#include <ngrs/ndockpanel.h>
#include <ngrs/nsplitbar.h>
#include <ngrs/nflipbox.h>
#include <ngrs/ncustomtreeview.h>
#include <ngrs/ntreenode.h>
#include <ngrs/nlabel.h>
#include <ngrs/nmessagebox.h>
#include <ngrs/nmenu.h>
#include <ngrs/npackageinfo.h>
#include <ngrs/ntablelayout.h>
#include <ngrs/ntable.h>
#include <ngrs/nedit.h>
#include <ngrs/nmemo.h>
#include <ngrs/nradiobutton.h>
#include <ngrs/ntextstatusitem.h>
#include <ngrs/nstatusbar.h>
#include <ngrs/ncolorchooser.h>
#include <ngrs/ncolorcombobox.h>
#include <ngrs/nobjectinspector.h>
#include <ngrs/nproperty.h>
#include <ngrs/nxmlparser.h>
#include <ngrs/nsplitbar.h>

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

	std::cout << "start of malloc" << std::endl;

	NBitmap bitmap;
	bitmap.createFromXpmData(a_xpm);

  NBitmap bitmap2;
 	bitmap2 = bitmap;

  NImage* img = new NImage(bitmap2);
		img->setPosition(10,50,200,200);
  pane()->add(img);

	std::cout << "end of malloc" << std::endl;


  testMenu();

  setName("testwindow");

  fDialog = new NFileDialog();
     fDialog->setName("fdialog");
  add(fDialog);

  NButton* btn1 = new NButton("open FileDialog");
  btn1->setPosition(10,10,100,20);
  pane()->add(btn1);

  btn1->clicked.connect(this, &NTestWindow::onBtnClick);

/*  NXmlParser parser;

  std::string text = "<hallo></hallo>";

  parser.parseString(text);

  NApp::system().clipBoard().setAsText("Hallo");

  std::cout << NApp::system().clipBoard().asText() << std::endl;

  /*
  NObjectInspector* ins = new NObjectInspector();
    ins->setPosition(10,10,200,200);
    NLabel* lb = new NLabel();
    lb->properties()->publish("name");
    ins->setControlObject(lb);

  pane()->add(ins);
  */

  // tabOrderTest

  /*for (int i = 0; i < 5 ; i++) {
    NButton* btn = new NButton("Button"+stringify(i));
    btn->setFlat( false );
    btn->setEvents(true);
    pane()->add(  btn , nAlTop );
  }

  NPanel* panel = new NPanel();
    panel->setLayout( NAlignLayout() );
    for (int i = 5; i < 10 ; i++) {
      NButton* btn = new NButton("Button"+stringify(i));
      btn->setFlat( false );
      btn->setEvents(true);
      panel->add(  btn , nAlTop );
    }
  pane()->add(panel, nAlClient);*/

  /*
  NColorComboBox* colorBox = new NColorComboBox();
     colorBox->setPosition(10,10,100,15);
  pane()->add(colorBox);
  */

  /*NColorChooser* chooser = new NColorChooser();
    chooser->setPosition(0,0,500,500);
    chooser->colorSelected.connect(this, &NTestWindow::onColorSelected);
  pane()->add(chooser);*/
  //testScrollBar();
  // testTimerButton();

//testMemo();
  //testListBox();

  //panel->setPosition(10,10,panel->preferredWidth(),panel->preferredHeight());
  //panel->setBackground(NColor(250,250,250));
  //std::cout << book->preferredHeight() << std::endl;

  /*NComboBox* box = new NComboBox();
    box->setPosition(10,40,100,20);
  pane()->add(box);*/

  /*NCustomTreeView* view = new NCustomTreeView();

  NTreeNode* node1 = new NTreeNode();
  view->addNode(node1);

  node1->setHeader(new NItem("Header1"));

  for (int i = 0; i < 10; i++) {
     node1->addEntry(new NItem("entry"+stringify(i)));
  }

  NTreeNode* node2 = new NTreeNode();
  view->addNode(node2);

  node2->setHeader(new NItem("Header2"));

  for (int i = 0; i < 10; i++) {
     node2->addEntry(new NItem("entryB"+stringify(i)));
  }

  itemD = new NItem("delme");
  node2->addEntry(itemD);

  pane()->add(view);

  view->setPosition(10,50,200,200);


  /*NLabel* lb = new NLabel("Hallo");
    lb->setPosition(10,120,100,120);
  pane()->add(lb);*/

  /*NButton* btn = new NButton("delete");
    btn->setPosition(220,50,100,20);
    btn->setFlat(false);
    btn->clicked.connect(this,&NTestWindow::onDelete);
  pane()->add(btn);*/
  //testMenu();
  //testSegDisplay();

  //testEdit();
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
  testMsgBox();
}

void NTestWindow::testBorderLayout( )
{
  pane()->setLayout(NBorderLayout());

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
    NFrameBorder fr;
    fr.setOval();
    pnl->setBorder(fr);
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
  NMenu* menu1 = new NMenu("Bearbeiten");//,'b');
     menu1->setName("m1");
  menuBar_->add(menu1);
  NMenu* menu2 = new NMenu("File");//,'b');
     menu2->setName("m2");
  menuBar_->add(menu2);
  NMenu* menu3 = new NMenu("Debug");//,'b');
     menu3->setName("m3");
  menuBar_->add(menu3);

  NMenuItem* item = new NMenuItem("Open");
     item->click.connect(this,&NTestWindow::onOpen);
     //item->setEnable(false);
  menu1->add(item);

  std::cout << item->captionLbl_->font().textColor().red() << std::endl;
  item->captionLbl_->setName("menu_lbl2");

  /*NMenuItem* item1 = new NMenuItem("close");
  menu1->add(item1);

  NMenuItem* item2 = new NMenuItem("crass");
  menu1->add(item2);

    NMenuItem* item3 = new NMenuItem("jo");
  menu1->add(item3);*/

  /*NPanel* test = test = new NPanel();
    test->skin_.gradientStyle=1;
    test->skin_.gradStartColor = NColor(200,200,200);
    test->setHeight(20);
    test->setLayout(new NFlowLayout(nAlLeft));
  pane()->add(test, nAlTop);
  test->add(new NButton("Hallo"));*/



  //NMenuItem* item = new NMenuItem("open");
    //NMenu* menu = new NMenu();
    //item->add(menu);
//  menu1->add(item);
//  item->click.connect(this,&NTestWindow::onOpen);

  NCheckMenuItem* item5 = new NCheckMenuItem("test disabled");
   item5->setEnable(false);
  menu1->add(item5);

  NMenuItem* item6 = new NMenuItem("subtest");
  menu1->add(item6);

  NMenu* subMenu = new NMenu();
  item6->add(subMenu);

  NCheckMenuItem* sub_item = new NCheckMenuItem("test");
  subMenu->add(sub_item);
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
     for (int i = 0; i < 10; i++) {
        lbox->add(new NItem(stringify(i)));
     }
  pane()->add(lbox);
  lbox->resize();
}

void NTestWindow::testSegDisplay( )
{
  N7SegDisplay* disp = new N7SegDisplay();
   pane()->setPosition(0,0,200,200);
  pane()->add(disp);
}

void NTestWindow::testTable( )
{
  NTable* table = new NTable();
  pane()->add(table,nAlClient);

  for (int y = 0; y < 100; y++) {
    for (int x = 0; x < 10; x++) {
         NEdit* edt = new NEdit(stringify(x)+","+stringify(y));
         table->add(edt,x,y, false);
      }
    }

  table->scrollPane()->resize();
}

void NTestWindow::testMemo( )
{
  NMemo* memo = new NMemo();
     memo->setPosition(10,10,400,400);
  pane()->add(memo, nAlClient);
  memo->setWordWrap(true);
//  memo->setReadOnly(true);

  memo->setText("hallo\ngenau\n\n\nhier\n");

  std::cout << memo->text() << std::endl;
}

void NTestWindow::testFlowLayout( )
{

}

void NTestWindow::testComboBox( )
{
  NComboBox* box = new NComboBox();
    box->setPosition(10,40,200,20);
    for (int i = 0; i < 1000; i++) {
       box->add(new NItem(stringify(i)+" : tester"));
    }
  pane()->add(box);
}

void NTestWindow::testMsgBox( )
{
  NMessageBox* box = new NMessageBox("Save changes of : name ?");
    box->setTitle("New Song");
    box->setButtonText("Yes","No","Abort");
  add(box);
  bool result = false;
  int choice = box->execute();

}

void NTestWindow::testTimerButton( )
{
  info = new NLabel();
   info->setPosition(200,50,200,20);
  pane()->add(info);

  counter = 0;
  NButton* btn1 = new NButton("press me");
    btn1->setPosition(10,100,200,20);
    btn1->click.connect(this, &NTestWindow::onBtnClick);
    btn1->setRepeatMode(true);
  pane()->add(btn1);
}

void NTestWindow::onBtnClick( NButtonEvent * ev )
{
  fDialog->execute();
}

void NTestWindow::testScrollBar( )
{
  NScrollBar* scrollBar = new NScrollBar();
  scrollBar->setOrientation(nVertical);
  pane()->add(scrollBar,nAlRight);
}

void NTestWindow::onColorSelected( const NColor & color )
{
  pane()->setBackground(color);
  pane()->repaint();
}

void NTestWindow::onDelete( NButtonEvent * ev )
{
  ((NVisualComponent*)itemD->parent())->erase(itemD);
  pane()->repaint();
}


