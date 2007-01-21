/***************************************************************************
 *   Copyright (C) 2005 by  Stefan   *
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

#include <ngrs/app.h>
#include <ngrs/system.h>
#include <ngrs/atoms.h>
#include <ngrs/autoscrolllayout.h>
#include <ngrs/line.h>
#include <ngrs/toolbar.h>
#include <ngrs/toolbarseparator.h>
#include <ngrs/combobox.h>
#include <ngrs/gridlayout.h>
#include <ngrs/checkbox.h>
#include <ngrs/checkmenuitem.h>
#include <ngrs/item.h>
#include <ngrs/groupbox.h>
#include <ngrs/frameborder.h>
#include <ngrs/borderlayout.h>
#include <ngrs/spinbutton.h>
#include <ngrs/spinedit.h>
#include <ngrs/segment7.h>
#include <ngrs/seg7display.h>
#include <ngrs/dockpanel.h>
#include <ngrs/splitbar.h>
#include <ngrs/flipbox.h>
#include <ngrs/customtreeview.h>
#include <ngrs/treenode.h>
#include <ngrs/label.h>
#include <ngrs/messagebox.h>
#include <ngrs/menu.h>
#include <ngrs/packageinfo.h>
#include <ngrs/tablelayout.h>
#include <ngrs/table.h>
#include <ngrs/edit.h>
#include <ngrs/memo.h>
#include <ngrs/radiobutton.h>
#include <ngrs/textstatusitem.h>
#include <ngrs/statusbar.h>
#include <ngrs/colorchooser.h>
#include <ngrs/colorcombobox.h>
#include <ngrs/objectinspector.h>
#include <ngrs/property.h>
#include <ngrs/xmlparser.h>
#include <ngrs/splitbar.h>
#include <ngrs/scrollbar.h>
#include <ngrs/groupbox.h>

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

using namespace ngrs;

NTestWindow::NTestWindow()
: Window()
{
  testCustomTreeView();
  //testMenu();
  //testListBox();
   // testEdit();
//	testMemo();

/*   std::cout << "testwin-id:" << win() << std::endl;
   testEdit();

   SplitBar* spl = new SplitBar();
   spl->setOrientation( nHorizontal );
   pane()->add( spl, nAlTop);

   */
/*	Button* btn = new Button("hint test");
	 btn->setHint("Save as audio File");
	 btn->clicked.connect(this, &NTestWindow::onBtnClick);
	pane()->add( btn, nAlTop);*/


 /* Slider* slider = new Slider();
    slider->setRange(0,100);
    slider->setPosition(10,10,200,200);
  pane()->add(slider);*/

/*   ScrollBar* bar = new ScrollBar();
     bar->setPosition(10,10,200,20);
     bar->setOrientation( nHorizontal );
     bar->change.connect( this, &NTestWindow::onScrollPosChange);
   pane()->add( bar );*/

   

   

  //testComboBox();

/*  Panel* panel = new Panel();
		panel->setPosition(10,10,100,20);
		panel->setBorder( FrameBorder() );
		panel->setMoveable( Moveable( nMvLeftBorder | nMvRightBorder | nMvHorizontal) );
	pane()->add(panel);*/

/*	Edit* btn = new Edit("Hgenau");
	btn->setVAlign( nAlCenter );
	btn->setBackground(Color(255,255,255));
	btn->setTransparent(false);
	btn->setFont( NFont("Suse Sans",12,nMedium | nAntiAlias) );
	pane()->add( btn , nAlTop );
  Label* btn1 = new Label("Hgenau");
	btn1->setVAlign( nAlCenter );
	btn1->setBackground(Color(100,255,255));
	btn1->setTransparent(false);
	btn1->setFont( NFont("Suse Sans",12,nMedium ) );
	btn1->setPosition(10,100,200,200);
	pane()->add( btn1  );*/

	/*
  testMenu();

  setName("testwindow");

  fDialog = new FileDialog();
     fDialog->setName("fdialog");
  add(fDialog);

  Button* btn1 = new Button("open FileDialog");
  btn1->setPosition(10,10,100,20);
  pane()->add(btn1);

  btn1->clicked.connect(this, &NTestWindow::onBtnClick);*/

/*  XmlParser parser;

  std::string text = "<hallo></hallo>";

  parser.parseString(text);

  App::system().clipBoard().setAsText("Hallo");

  std::cout << App::system().clipBoard().asText() << std::endl;

  /*
  ObjectInspector* ins = new ObjectInspector();
    ins->setPosition(10,10,200,200);
    Label* lb = new Label();
    lb->properties()->publish("name");
    ins->setControlObject(lb);

  pane()->add(ins);
  */

  // tabOrderTest

  /*for (int i = 0; i < 5 ; i++) {
    Button* btn = new Button("Button"+stringify(i));
    btn->setFlat( false );
    btn->setEvents(true);
    pane()->add(  btn , nAlTop );
  }

  Panel* panel = new Panel();
    panel->setLayout( AlignLayout() );
    for (int i = 5; i < 10 ; i++) {
      Button* btn = new Button("Button"+stringify(i));
      btn->setFlat( false );
      btn->setEvents(true);
      panel->add(  btn , nAlTop );
    }
  pane()->add(panel, nAlClient);*/

  /*
  ColorComboBox* colorBox = new ColorComboBox();
     colorBox->setPosition(10,10,100,15);
  pane()->add(colorBox);
  */

  /*ColorChooser* chooser = new ColorChooser();
    chooser->setPosition(0,0,500,500);
    chooser->colorSelected.connect(this, &NTestWindow::onColorSelected);
  pane()->add(chooser);*/
  //testScrollBar();
  // testTimerButton();

//testMemo();
  //testListBox();

  //panel->setPosition(10,10,panel->preferredWidth(),panel->preferredHeight());
  //panel->setBackground(Color(250,250,250));
  //std::cout << book->preferredHeight() << std::endl;

  /*ComboBox* box = new ComboBox();
    box->setPosition(10,40,100,20);
  pane()->add(box);*/

  


  /*Label* lb = new Label("Hallo");
    lb->setPosition(10,120,100,120);
  pane()->add(lb);*/

  /*Button* btn = new Button("delete");
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


void NTestWindow::onQuit( Object * sender )
{
  exit(0);
}

void NTestWindow::onSliderPosChanged( double v )
{
  int dy = (int) ( ((scrollWin->pane()->clientHeight() - pane()->clientHeight()) / ((double) slider->clientHeight())) * v);

  //XMoveResizeWindow (App::system().dpy(),
                          //scrollWin->win(),
                          //0,-dy, 900, 2000);
  //printf("%f\n",v); fflush(stdout);
  //scrollWin->setLeft((int) v);
}

void NTestWindow::onOpen( ButtonEvent * ev )
{
	if ( fDialog->execute() ) {
		
	};
  //testMsgBox();
}

void NTestWindow::testBorderLayout( )
{
  pane()->setLayout(BorderLayout());

  Button* button = new Button("Button 1 (PAGE_START)");
  pane()->add(button, nAlTop);

  button = new Button("Button 2 (CENTER)");
  button->setPreferredSize(10, 10);
  pane()->add(button, nAlClient);

  button = new Button("Button 3 (LINE_START)");
  pane()->add(button, nAlLeft);

  button = new Button("Long-Named Button 4 (PAGE_END)");
  pane()->add(button, nAlBottom);

  button = new Button("5 (LINE_END)");
  pane()->add(button, nAlRight);

  pack();
}

void NTestWindow::testFrameBorder( )
{
  Panel* pnl = new Panel();
    pnl->setPosition(10,10,200,200);
    FrameBorder fr;
    fr.setOval();
    pnl->setBorder(fr);
  pane()->add(pnl);
}

void NTestWindow::testLine( )
{
  Line* line = new Line();
    line->setPoints(NPoint(10,10),NPoint(100,100));
    line->setMoveable(Moveable());
    fDialog = new FileDialog();
    line->add(fDialog);
  pane()->add(line);
}

void NTestWindow::testMenu( )
{
  fDialog = new FileDialog();
    fDialog->addFilter("*.psy [psy3 song format]","!S*.psy");
  add(fDialog);

 // Window* popupMenu_ = new Window();

  //Window* win1 = new Window();

  MenuBar* menuBar_ = new MenuBar();
  menuBar_->setName("menuBar_");
  pane()->add(menuBar_);
  Menu* menu1 = new Menu("Bearbeiten");//,'b');
     menu1->setName("m1");
  menuBar_->add(menu1);
  Menu* menu2 = new Menu("File");//,'b');
     menu2->setName("m2");
  menuBar_->add(menu2);
  Menu* menu3 = new Menu("Debug");//,'b');
     menu3->setName("m3");
  menuBar_->add(menu3);

  MenuItem* item = new MenuItem("Open");
     item->click.connect(this,&NTestWindow::onOpen);
     //item->setEnable(false);
  menu1->add(item);


  /*MenuItem* item1 = new MenuItem("close");
  menu1->add(item1);

  MenuItem* item2 = new MenuItem("crass");
  menu1->add(item2);

    MenuItem* item3 = new MenuItem("jo");
  menu1->add(item3);*/

  /*Panel* test = test = new Panel();
    test->skin_.gradientStyle=1;
    test->skin_.gradStartColor = Color(200,200,200);
    test->setHeight(20);
    test->setLayout(new FlowLayout(nAlLeft));
  pane()->add(test, nAlTop);
  test->add(new Button("Hallo"));*/



  //MenuItem* item = new MenuItem("open");
    //Menu* menu = new Menu();
    //item->add(menu);
//  menu1->add(item);
//  item->click.connect(this,&NTestWindow::onOpen);

  ngrs::CheckMenuItem* item5 = new ngrs::CheckMenuItem("test disabled");
   item5->setEnable(false);
  menu1->add(item5);

  MenuItem* item6 = new MenuItem("subtest");
  menu1->add(item6);

  Menu* subMenu = new Menu();
  item6->add(subMenu);

  ngrs::CheckMenuItem* sub_item = new ngrs::CheckMenuItem("test");
  subMenu->add(sub_item);
}

void NTestWindow::testSpinButton( )
{
  ngrs::SpinButton* btn = new ngrs::SpinButton();
     std::cout << btn->preferredWidth() << std::endl;
     btn->setPosition(10,10,btn->preferredWidth(),btn->preferredHeight());
   pane()->add(btn);
}

void NTestWindow::testEdit( )
{
  Edit* edit = new Edit();
    edit->setPosition(10,10,200,20);
  pane()->add(edit);

}

void NTestWindow::testSpinEdit( )
{
  SpinEdit* edit = new SpinEdit();
    edit->setPosition(10,10,200,20);
  pane()->add(edit);
}

void NTestWindow::toolBarTest( )
{

  std::string iconPath = "~/xpsycle/icons/";

  /*Panel* toolBarPanel_ = new Panel();
  FlowLayout* fl = new FlowLayout(nAlLeft,0,0);
    toolBarPanel_->setLayout(fl);
    toolBarPanel_->setWidth(500);
    toolBarPanel_->setAlign(nAlTop);*/

  ToolBar* toolBar1_ = new ToolBar();
  pane()->add(toolBar1_,nAlTop); //toolBar1_->add(lb);

  ngrs::Button* lb = new ngrs::Button("Hallo");
    std::cout << "this" << lb->preferredHeight() << std::endl;
    toolBar1_->add(lb);
    std::cout << "after" << lb->preferredHeight() << std::endl; 

    std::cout << toolBar1_->preferredHeight() << std::endl;
    std::cout << "y" << lb->preferredHeight() << std::endl;

    ngrs::Image* img;
    img = new ngrs::Image();
    img->loadFromFile(iconPath+ "new.xpm");
    img->setPreferredSize(40,80);
    toolBar1_->add(new Button(img));

    img = new ngrs::Image();
    img->loadFromFile(iconPath+ "new.xpm");
    toolBar1_->add(new Button(img));

    img = new ngrs::Image();
    img->loadFromFile(iconPath+ "new.xpm");
    toolBar1_->add(new Button(img));
   //toolBarPanel_->add(toolBar1_);
   //toolBar1_->resize();
  
  //pane()->add(toolBarPanel_);
  pane()->resize();
}

void NTestWindow::testListBox( )
{
  ngrs::ListBox* lbox = new ngrs::ListBox();
     lbox->setMultiSelect(true);
     lbox->setPosition(10,10,200,200);
     for (int i = 0; i < 10; i++) {
        lbox->add(new Item(stringify(i)));
     }
  pane()->add(lbox);
  lbox->resize();
}

void NTestWindow::testSegDisplay( )
{
  Seg7Display* disp = new Seg7Display();
   pane()->setPosition(0,0,200,200);
  pane()->add(disp);
}

void NTestWindow::testTable( )
{
  Table* table = new Table();
  pane()->add(table,nAlClient);

  for (int y = 0; y < 100; y++) {
    for (int x = 0; x < 10; x++) {
         Edit* edt = new Edit(stringify(x)+","+stringify(y));
         table->add(edt,x,y, false);
      }
    }

  table->scrollPane()->resize();
}

void NTestWindow::testMemo( )
{
  Memo* memo = new Memo();
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
  ComboBox* box = new ComboBox();
    box->setPosition(10,40,200,20);
    for (int i = 0; i < 1000; i++) {
       box->add(new Item(stringify(i)+" : tester"));
    }
  pane()->add(box);
}

void NTestWindow::testMsgBox( )
{
  ngrs::MessageBox* box = new ngrs::MessageBox("Save changes of : name ?");
    box->setTitle("New Song");
    box->setButtonText("Yes","No","Abort");
  add(box);
  bool result = false;
  int choice = box->execute();

}

void NTestWindow::testTimerButton( )
{
  info = new ngrs::Label();
   info->setPosition(200,50,200,20);
  pane()->add(info);

  counter = 0;
  ngrs::Button* btn1 = new ngrs::Button("press me");
    btn1->setPosition(10,100,200,20);
    btn1->click.connect(this, &NTestWindow::onBtnClick);
    btn1->setRepeatMode(true);
  pane()->add(btn1);
}

void NTestWindow::onBtnClick( ngrs::ButtonEvent * ev )
{
  
}

void NTestWindow::testScrollBar( )
{
  ngrs::ScrollBar* scrollBar = new ngrs::ScrollBar();
  scrollBar->setOrientation( ngrs::nVertical);
  pane()->add(scrollBar,nAlRight);
}

void NTestWindow::onColorSelected( const Color & color )
{
  pane()->setBackground(color);
  pane()->repaint();
}

void NTestWindow::onDelete( ButtonEvent * ev )
{
  ((VisualComponent*)itemD->parent())->erase(itemD);
  pane()->repaint();
}

void NTestWindow::onSelection( )
{
  /*Atom actual_type;
  int actual_format;
  unsigned long nitems;
  unsigned long bytes_after;
  unsigned char *ret=0;
			
  int read_bytes = 1024;	

  //Keep trying to read the selection until there are no
  //bytes unread.
  do
   {
     if(ret != 0) XFree(ret);
		 XGetWindowProperty(App::system().dpy(), win(), App::system().atoms().primary_sel(), 0, read_bytes, False, AnyPropertyType,
     &actual_type, &actual_format, &nitems, &bytes_after, 
     &ret);
     read_bytes *= 2;
   }while(bytes_after != 0);
			
   std::cout << std::endl;
//   std::cout << "Actual type: " << GetAtomName(App::system().dpy(), actual_type) << std::endl;
   std::cout << "Actual format: " << actual_format << std::endl;
   std::cout << "Number of items: " << nitems <<  std::endl;
   std::cout << "Bytes left: " << bytes_after <<  std::endl;

   //Dump the binary data
    std::cout.write((char*)ret, nitems * actual_format/8);
    std::cerr << std::endl;*/
}

void NTestWindow::onScrollPosChange( ScrollBar * bar )
{
  std::cout << bar->pos() << std::endl;
}

void NTestWindow::onMousePress( int x, int y, int button )
{
  Window::onMousePress(x,y,button);

  /*Line* line = new Line();
     line->setPoints( NPoint(x,y), NPoint(x,y));
     line->setMoveable( nMvPolygonPicker );
   pane()->add(line);
  line->setMoveFocus(0);*/
}

void NTestWindow::testObjectInspector( )
{
  ObjectInspector* ins = new ObjectInspector();
    ins->setControlObject( this );
  pane()->add( ins, nAlClient );
}

void NTestWindow::testGroupBox( )
{
  GroupBox* box = new GroupBox();
	pane()->add( box, nAlClient );
}

void NTestWindow::testCustomTreeView() {	
  // this creates a customtreeview
  CustomTreeView* view = new CustomTreeView();

  TreeNode* node1 = new TreeNode();
    node1->setHeader(new Item("Header1"));
	for (int i = 0; i < 10; i++) {
     node1->addEntry(new Item("entry"+stringify(i)));
    }
  view->addNode(node1);
    
  TreeNode* node2 = new TreeNode();
    node2->setHeader( new Item("Header2") );
	for (int i = 0; i < 10; i++) {
     node2->addEntry( new Item("entryB"+stringify(i) ) );
    }
	itemD = new Item("delme");
    node2->addEntry(itemD);
  view->addNode(node2);
    
  pane()->add( view, nAlClient );  
}
