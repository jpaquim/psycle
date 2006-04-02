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

  NScrollBox* bx = new NScrollBox();
     bx->setPosition(10,10,200,200);
  pane()->add(bx);

 /* fDialog = new NFileDialog();
   add(fDialog);*/

   //testListBox();
   /*Tpnl* tPnl = new Tpnl();
      tPnl->setPosition(10,10,200,200);
   pane()->add(tPnl);*/

   /*NButton* btn = new NButton("hallO");
     btn->setFlat(false);
     btn->setPosition(100,100,200,20);
   pane()->add(btn);*/

  //toolBarTest();
 // testSpinEdit();
 // testBorderLayout();
 // testFrameBorder();
    //testLine();
    //testMenu();
//    testComboBox();
  
  /*NPanel* pnl = new NPanel();
    pnl->setTransparent(false);
    pnl->setBackground(NColor(255,255,255));
    pnl->setPosition(10,50,200,200);
    pnl->setLayout(new NGridLayout());
      pnl->add(new NButton("Test"));
      pnl->add(new NButton("Test1"));
      pnl->add(new NButton("Test2"));
      pnl->add(new NButton("Test3"));
      pnl->add(new NButton("Test4"));
  pane()->add(pnl);
  pnl->resize();
  pnl->setHeight(pnl->preferredHeight());
   /*NToolBar* bar = new NToolBar();
   bar->setAlign(nAlTop);
*/

  


  /*NScrollBox* tbox = new NScrollBox();
    tbox->setPosition(20,20,200,200);
  pane()->add(tbox);


  NBitmap down; down.createFromXpmData(a_xpm);
  NImage* downImg = new NImage();
  downImg->setBitmap(down);
  NButton* btn = new NButton(downImg,10,10);
    btn->setPosition(230,30,100,20);
  pane()->add(btn);

  NLabel* lb = new NLabel("Hallo");
     lb->setPosition(10,40,200,30);
  pane()->add(lb);

   NComboBox* cbox = new NComboBox();
     cbox->setPosition(200,200,100,20);
     cbox->add(new NItem("test"));
     cbox->setWidth(100);
     for (int i = 0; i< 10; i++) {
        cbox->add(new NItem("test"));
     }
   cbox->setIndex(3);
   pane()->add(cbox);*/
/*
   //pane()->add(bar);


/*   NButton* btn = new NButton("Test");
     btn->setPosition(100,100,100,30);
   pane()->add(btn);*/

   /*NTabBook* tBook = new NTabBook();
   tBook->setBackground(NColor(255,255,255));
   tBook->setTransparent(false);
   tBook->addNewPage("Toll");
   tBook->addNewPage("Toll1");
   tBook->setAlign(nAlClient);
   pane()->add(tBook);*/

   /*NLabel* p1 = new NLabel("test");
   p1->setAlign(nAlLeft);
   pane()->add(p1);

   NLabel* p2 = new NLabel("test");
   p2->setAlign(nAlLeft);
   pane()->add(p2);

   pane()->resize();

   

   /*NScrollBox* box = new NScrollBox();
   box->setPosition(100,100,200,200);
   pane()->add(box);*/

   /*pane()->setLayout(new NFlowLayout(nAlLeft,100,20));
   NPanel* pl = new NPanel();
   pl->setLayout(new NFlowLayout(nAlLeft,1,10));
   pl->setBackground(NColor(0,0,255));
   pane()->add(pl);*/

   //pane()->add(new NLabel("Test"));
   //pane()->add(new NLabel("Test1"));
   //pane()->add(new NLabel("Test2"));

   /*NToolBar* bar = new NToolBar();
   bar->setMinimumHeight(20);
   bar->setAlign(nAlTop);
   pane()->add(bar);

   bar->setHeight(20);

   bar-> add(new NToolBarSeparator());*/

   /*NPanel* p = new NPanel();
   p->setAlign(nAlTop);
   pane()->add(p);*/

   /*NImage* img = new NImage();

   img->loadFromFile("/home/natti/soundtracker-0.6.7/stop.xpm");

   img->setPosition(10,10,200,200);
   pane()->add(img);*/


   
   /*NMenu* fileMenu_ = new NMenu("File",'i'
      ,"New,Open,Import Module,Save,Save as,Render as Wav,|,Song properties,|,revert to Saved,recent Files,Exit");
   menuBar_->add(fileMenu_);//fileMenu_);*/


  /* NMenu* editMenu_ = new NMenu("Edit",'e',
       "Undo,Redo,Pattern Cut,Pattern Copy,Pattern Paste,Pattern Mix Paster,Pattern Delete,|,Block Cut,Block Copy,Block Paste,Block Mix Paste,Block Delete,|,Sequence Cut,Sequence Copy,Sequence Delete");
  // menuBar_->add(editMenu_);

   NMenu* viewMenu_ = new NMenu("View",'v',
       "Toolbar,MachineBar,SequencerBar,StatusBar,|,MachineView,PatternEditor,PatternSequencer,|,Add machine,Instrument Editor");
   //menuBar_->add(viewMenu_);

   NMenu* configurationMenu_ = new NMenu("Configuration",'c',
       "Free Audio,AutoStop,|,Loop Playback,|,Settings");
   //menuBar_->add(configurationMenu_);

   NMenu* performanceMenu_ = new NMenu("Performance",'p',"CPU Monitor ...,Midi Monitor ...");
   //menuBar_->add(performanceMenu_);

   NMenu* helpMenu_ = new NMenu("Help",'h',
       "Help,|,./doc/readme.txt,./doc/tweaking.txt,./doc/keys.txt,./doc/tweaking.txt,./doc/whatsnew.txt,|,About,Greetings");
   //menuBar_->add(helpMenu_);

   */
   
   /*NListBox* lbox = new NListBox();
   pane()->add(lbox);
   lbox->setAlign(nAlClient);
   for (int i = 0; i< 100; i++) {
   //  lbox->add(new NLabel("jslkfdjsd lkjslkd jlkfdsj lkfdsj lkdsj lkjfds kljds lkjds lkj dsflkj "));
   }*/


/*   NMenu* menu1 = new NMenu("Bearbeiten",'b');
   menuBar_->add(menu1);
   NMenuItem* item = new NMenuItem("open");
   menu1->add(item);
   item->click.connect(this,&NTestWindow::onOpen);*/
/*   menu->add(item);
   

   NPanel* pl = new NPanel();
   pl->setWidth(200);
   pl->setAlign(nAlLeft);
   pane()->add(pl);

   pl->setLayout(new NListLayout());

   pl->add(new NLabel("Test hier"));

   NPanel* len = new NPanel();
    len->setWidth(100);
    len->setBorder(new NFrameBorder());
    NFlowLayout* fl = new NFlowLayout();
    len->setLayout(fl);
    len->add(new NLabel("+"));
    len->add(new NLabel("-"));
    len->add(new NLabel("*"));
    len->add(new NLabel("/"));
    len->add(new NLabel("-"));
    len->add(new NLabel("("));
   pl->add(len);
   len->resize();
   printf("%d\n",len->preferredWidth());

   resize();
   pl->resize();
   len->resize()
   printf("%d\n",len->preferredWidth());

   /*NTabBook* book = new NTabBook();
   book->setAlign(nAlClient);
   book->setTabBarAlign(nAlBottom);
   pane()->add(book);

   NPage* p1 = book->addNewPage("Tab1");
   NPage* p2 = book->addNewPage("Tab2");

   NLabel* lb1 = new NLabel("Tab1");
   lb1->setWidth(lb1->preferredWidth());
   lb1->setHeight(lb1->preferredHeight());
   NLabel* lb2 = new NLabel("Tab2");
   lb2->setWidth(lb2->preferredWidth());
   lb2->setHeight(lb2->preferredHeight());

   //p1->add(lb1);
   p2->add(lb2);


   NCheckBox* checkBox = new NCheckBox();
   checkBox->setText("Follow song");
   checkBox->setWidth(checkBox->preferredWidth());
   checkBox->setHeight(checkBox->preferredHeight());
   p1->add(checkBox);*/

//   setDoubleBuffer(false);

   /*scrollWin = new NWindow(*this);
   scrollWin->pane()->setLayout(new NListLayout());
   add(scrollWin);*/


   /*scrollWin->pane()->setBackground(NColor(200,200,200));
   scrollWin->setDoubleBuffer(false);
   scrollWin->setPosition(0,1,100,3000);
   scrollWin->setVisible(true);
   scrollWin->pane()->setClientSizePolicy(nVertical + nHorizontal);


   slider = new NSlider();
   slider->setPosition(1000,10,20,100);
   pane()->add(slider);


   slider->posChanged.connect(this,&NTestWindow::onSliderPosChanged);

 // NImage* img = new NImage();
 // img->setPosition(0,0,100,100);
  //img->loadFromFile("/home/natti/soundtracker-0.6.7/downarrow.xpm");
 // pane()->add(img);

  /*setName("Test1");
  pane()->setName("hier");

*/
  
  

  /*NScrollBox* box = new NScrollBox();
  box->setTransparent(true);
  box->setPosition(0,0,200,200);
  box->setAlign(nAlClient);
  pane()->add(box);

  pane()->updateAlign();

  NPanel* scrollArea = new NPanel();
  scrollArea->setTransparent(true);
  scrollArea->setLayout(new NListLayout());
  scrollArea->setClientSizePolicy(nVertical | nHorizontal);
  box->setScrollPane(scrollArea);
  NFont fnt;
  fnt.setName("8x13");
  fnt.setStyle(fnt.style() - nAntiAlias);//setName("8x13");
  scrollArea->setFont(fnt);*/

 /* for (int i = 0; i< 1000; i++) {
    NPanel* panel = new NPanel();
    panel->setWidth(1000);
    panel->setHeight(20);
    panel->setTransparent(true);
    NFlowLayout* fl = new NFlowLayout();
    fl->setLineBreak(false);
    panel->setLayout(fl);
    panel->setBorder(0);
    scrollArea->add(panel);
    for (int j = 0; j < 1; j++) {
      NEdit* slider = new NEdit();
      slider->setText("00 00 00 00");
      slider->setBorder(0);
      slider->setWidth(100); slider->setHeight(20);
      slider->setTransparent(true);
      //slider->setPosition(0,0,200,20);
      //slider->setOrientation(nHorizontal);
      panel->add(slider);
    }
    panel->updateAlign();
  }
  scrollArea->updateAlign();*/

  /*std::string text = NFile::readFile("/home/natti/ngrs_01a/src/sigslot.h");
  int pos   = 1;
  int start = 0;

  int z = 0;

  while ( z <256 &&    ( pos = text.find("\n",pos)) > -1) {
     std::string textLine = text.substr(start,pos-start);
     NLabel* label = new NLabel(textLine);
     scrollArea->add(label);
     z++;
     pos++;
     start = pos;
  }
  if (start != text.length()) {
     std::string textLine = text.substr(start);
     NLabel* label = new NLabel(textLine);
     scrollArea->add(label);
  }

   scrollArea->updateAlign();*/

  /*for (int i = 0; i < 256; i++) {
     NPanel* panel = new NPanel();
     panel->setTransparent(true);
     NFlowLayout* fl = new NFlowLayout();
     fl->setLineBreak(false);
     panel->setLayout(fl);
     scrollArea->add(panel);
     for (int j = 0; j < 64; j++) {
        NEdit* edit = new NEdit();
        edit->setText("00 00 00 00");
        edit->setTransparent(true);
        edit->setWidth(100);
        if (j==23) edit->setName("test");
        panel->add(edit);
     }
     panel->updateAlign();
  }
  scrollArea->updateAlign();*/

  /*pBar = new NProgressBar();
  pBar->setPosition(10,10,10,100);
  pBar->setOrientation(nHorizontal);
  pBar->setMax(80);
  pane()->add(pBar);

  NSlider* slider = new NSlider();
  slider->posChanged.connect(this,&NTestWindow::onSliderPosChanged);
  slider->setPosition(50,70,20,100);
  pane()->add(slider);*/

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
  pane()->setLayout(new NBorderLayout());

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

  NMenuBar* menuBar_ = new NMenuBar();
  pane()->add(menuBar_);
  NMenu* menu1 = new NMenu("Bearbeiten",'b');
  menuBar_->add(menu1);

  NMenuItem* item = new NMenuItem("open");
  menu1->add(item);
  item->click.connect(this,&NTestWindow::onOpen);
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
   /* NImage* img;
    img = new NImage();
    img->loadFromFile(iconPath+ "new.xpm");
    toolBar1_->add(new NButton(img));

    img = new NImage();
    img->loadFromFile(iconPath+ "new.xpm");
    toolBar1_->add(new NButton(img));

    img = new NImage();
    img->loadFromFile(iconPath+ "new.xpm");
    toolBar1_->add(new NButton(img));*/
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


