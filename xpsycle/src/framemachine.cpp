/***************************************************************************
 *   Copyright (C) 2006 by Stefan   *
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
#include "framemachine.h"
#include "machine.h"
#include "global.h"
#include "configuration.h"
#include "defaultbitmaps.h"
#include "presetsdlg.h"
#include <napp.h>
#include <nmenubar.h>
#include <ntogglepanel.h>
#include <nbutton.h>
#include <nedit.h>
#include <nfiledialog.h>


NBitmap Knob::kbitmap;
int Knob::c = 0;


NewNameDlg::NewNameDlg( )
{

  NPanel* btnPnl = new NPanel();
    btnPnl->setLayout(new NFlowLayout(nAlRight,5,5),true);
    btnPnl->add(new NButton("add"));
    btnPnl->add(new NButton("cancel"));
  pane()->add(btnPnl,nAlBottom);

  NEdit* name = new NEdit();
  name->setText("userprs");
  pane()->add(name,nAlClient);


  setPosition(100,100,300,60);
}

NewNameDlg::~ NewNameDlg( )
{
}

FrameMachine::FrameMachine(Machine* pMachine)
 : NWindow()
{
  pMachine_ = pMachine;

  init();
  initParameterGUI();
  loadPresets();
}


FrameMachine::~FrameMachine()
{
}

int FrameMachine::onClose( )
{
  setVisible(false);
  return nHideWindow;
}

void FrameMachine::init( )
{
  NMenuBar* bar = new NMenuBar();
    pane()->add(bar);
  NMenu* aboutMenu = new NMenu("About",'a',"Help,|,About this machine");
    aboutMenu->itemClicked.connect(this, &FrameMachine::onItemClicked);
  bar->add(aboutMenu);
  NMenu* parameterMenu = new NMenu("Parameters",'p',"Reset,Random,Presets");
    parameterMenu->itemClicked.connect(this, &FrameMachine::onItemClicked);
  bar->add(parameterMenu);

  setTitle(stringify(pMachine_->_macIndex)+std::string(" : ")+pMachine_->GetName());

  NPanel* prs = new NPanel();
    prs->setLayout(new NAlignLayout(),true);
    NButton* savePrsBtn = new NButton("save");
       savePrsBtn->setFlat(false);
    prs->add(savePrsBtn,nAlLeft);
    NButton* loadPrsBtn = new NButton("load");
       loadPrsBtn->setFlat(false);
       loadPrsBtn->clicked.connect(this,&FrameMachine::onLoadPrs);
    prs->add(loadPrsBtn,nAlLeft);
    NButton* addPrsBtn = new NButton("add");
       addPrsBtn->setFlat(false);
       addPrsBtn->clicked.connect(this,&FrameMachine::onAddPrs);
    prs->add(addPrsBtn,nAlLeft);
    defaultPrsBtn = new NButton("User");
       defaultPrsBtn->setFlat(false);
    prs->add(defaultPrsBtn,nAlLeft);
    NButton* rightPrsBtn = new NButton(">");
       rightPrsBtn->setFlat(false);
       rightPrsBtn->clicked.connect(this,&FrameMachine::onRightBtn);
    prs->add(rightPrsBtn,nAlRight);
    NButton* leftPrsBtn = new NButton("<");
       leftPrsBtn->setFlat(false);
       leftPrsBtn->clicked.connect(this,&FrameMachine::onLeftBtn);
    prs->add(leftPrsBtn,nAlRight);
    prsPanel = new NTogglePanel();
      NFlowLayout* fl = new NFlowLayout(nAlLeft,5,5);
        fl->setLineBreak(false);
      prsPanel->setLayout(fl,true);
    prs->add(prsPanel, nAlClient);
  pane()->add(prs, nAlBottom);
}


inline int format(int c, int maxcols, int maxrows) {
   return (c / maxcols) + ( (c%maxcols)*maxrows);
}

void FrameMachine::initParameterGUI( )
{
  knobPanel = new NPanel();
    gridLayout = new NGridLayout();
    knobPanel->setLayout(gridLayout,true);
  pane()->add(knobPanel,nAlClient);

  NFont font("Suse sans",6,nMedium | nStraight | nAntiAlias);
    font.setTextColor(Global::pConfig()->machineGUITopColor);
  knobPanel->setFont(font);

  int numParameters = pMachine_->GetNumParams();
  int cols = pMachine_->GetNumCols();
  int rows = numParameters/cols;

  gridLayout->setRows(rows);
  gridLayout->setColumns(cols);

   for (int c=0; c<numParameters; c++) {
     int min_v,max_v,val_v;
     int newC = format(c,cols,rows);
     pMachine_->GetParamRange(newC,min_v,max_v);
     bool bDrawKnob = (min_v==max_v)?false:true;
     if (!bDrawKnob) {
        Header* cell = new Header();
        char parName[64];
        pMachine_->GetParamName(format(c,cols,rows),parName);
        cell->setText(parName);
        knobPanel->add(cell);
     } else {
        Knob* cell = new Knob(format(c,cols,rows));
        cell->valueChanged.connect(this,&FrameMachine::onKnobValueChange);
        char parName[64];
        pMachine_->GetParamName(format(c,cols,rows),parName);
        char buffer[128];
        pMachine_->GetParamValue(format(c,cols,rows),buffer);
        cell->setText(parName);
        int val_v = pMachine_->GetParamValue(format(c,cols,rows));
        cell->setValue(val_v);
        int min_v; int max_v;
        pMachine_->GetParamRange(format(c,cols,rows),min_v,max_v);
        cell->setRange(min_v,max_v);
        cell->setValueAsText(buffer);
        knobPanel->add(cell);
     }
  }
  pack();
}

// Knob class

Knob::Knob(int param )  : max_range(100), min_range(0), value_(0), istweak(0), finetweak(1), ultrafinetweak(0), sourcepoint(0)  {
  param_ = param;

  if (c==0) {
    if (Global::pConfig()->iconPath=="") 
       kbitmap = Global::pBitmaps()->tbmainknob(); 
    else
       kbitmap.loadFromFile(Global::pConfig()->iconPath+ "TbMainKnob.xpm");
  }
  c++;
  add( label = new NLabel());
  add( vLabel = new NLabel()); // the label that shows the value in %
  NFont font = NFont("Suse sans",6,nMedium | nStraight | nAntiAlias);
  font.setTextColor(Global::pConfig()->machineGUIFontBottomColor);
  vLabel->setFont(font);
  vLabel->setText(stringify(value_)+"%");
  font.setTextColor(Global::pConfig()->machineGUIFontTopColor);
  label->setFont(font);
}

void Knob::setValue( int value )
{
  value_ = value;
  if (tvalue=="") vLabel->setText(stringify(value)+"%");
}

void Knob::setRange( int min, int max )
{
  min_range = min;
  max_range = max;
}

void Knob::paint( NGraphics * g )
{
  Cell::paint(g);

  int amp = max_range - min_range;
  int rel = value_ - min_range;

  if (amp > 0) {
    int frame = (K_NUMFRAMES*rel)/amp;
    int xn = frame*K_XSIZE;
    int CH = clientHeight();

    g->putBitmap(0,(CH - K_YSIZE)/2,K_XSIZE,K_YSIZE, kbitmap, xn,0);
  }
}

void Knob::setText( const std::string & text )
{
  label->setText(text);
}

void Knob::resize( )
{
  int CH = clientHeight();

  label->setPosition(K_XSIZE+5,0,clientWidth()-K_XSIZE-5,CH/2);
  vLabel->setPosition(K_XSIZE+5,CH/2,clientWidth()-K_XSIZE-5,CH/2);
}

int Knob::preferredHeight( ) const
{
  NFontMetrics metrics(font());
  return std::max(2*metrics.textHeight(),K_YSIZE);
}

int Knob::preferredWidth( ) const
{
  NFontMetrics mtr(font());
  return K_XSIZE + std::max(mtr.textWidth(vLabel->text()),mtr.textWidth(label->text())) + 10;
}

void Cell::paint( NGraphics * g )
{
  int CW = clientWidth();
  int CH = clientHeight();

  g->setForeground(Global::pConfig()->machineGUITopColor);
  g->fillRect(0,0,CW, CH / 2);
  g->setForeground(Global::pConfig()->machineGUIBottomColor);
  g->fillRect(0,CH/2,CW, CH / 2);
}

Header::Header( ) : Cell()
{
  label = new NLabel();
    label->setTransparent(false);
    label->setBackground(Global::pConfig()->machineGUITitleColor);
    NFont font = NFont("Suse sans",6,nMedium | nStraight | nAntiAlias);
    font.setTextColor(Global::pConfig()->machineGUITitleFontColor);
    label->setFont(font);
  add(label);
}

void Knob::setValueAsText( const std::string & text )
{
  tvalue = text;
  vLabel->setText(tvalue);
}

void Knob::onMousePress( int x, int y, int button )
{
  int CH = clientHeight();
  if (NRect(0,(CH - K_YSIZE)/2,K_XSIZE,K_YSIZE).intersects(x,y)) {
    istweak = true;
    sourcepoint = y;
  }
}

void Knob::onMouseOver( int x, int y )
{
  if (istweak) {
     if (( ultrafinetweak && !(NApp::system().keyState() & ShiftMask )) || //shift-key has been left.
         ( !ultrafinetweak && (NApp::system().keyState() & ShiftMask))) //shift-key has just been pressed
     {
        sourcepoint=y;
        ultrafinetweak=!ultrafinetweak;
     }
     else if (( finetweak && !(NApp::system().keyState() & ControlMask )) || //control-key has been left.
     ( !finetweak && (NApp::system().keyState() & ControlMask))) //control-key has just been pressed
     {
        sourcepoint = y;
        finetweak=!finetweak;
     }


    int maxval = max_range;
    int minval = min_range;
    int tweakbase = value_;

    int screenh = window()->height();
    double freak = 0.5;
    if ( ultrafinetweak ) freak = 0.5f;
    else if (maxval-minval < screenh/4) freak = (maxval-minval)/float(screenh/4);
    else if (maxval-minval < screenh*2/3) freak = (maxval-minval)/float(screenh/3);
    else freak = (maxval-minval)/float(screenh*3/5);

    if (finetweak) freak/=5;

    double nv = (double)(sourcepoint - y)*freak + (double)tweakbase;
    if (nv < minval) nv = minval;
    if (nv > maxval) nv = maxval;

    value_ = (int) nv;
    valueChanged.emit(this,(int) nv, param_);
  }
}

void Knob::onMousePressed( int x, int y, int button )
{
  istweak = false;
}

void FrameMachine::onKnobValueChange( Knob* sender,int value , int param )
{
  pMachine_->SetParameter(param, value);
  char buffer[128];
  pMachine_->GetParamValue(param,buffer);
  sender->setValueAsText(buffer);
  int val_v = pMachine_->GetParamValue(param);
  sender->setValue(val_v);
  sender->repaint();
}


void FrameMachine::onItemClicked( NEvent * menuEv, NButtonEvent * itemEv )
{
  if (itemEv->text() == "Presets") {
     PresetsDlg* dlg = new PresetsDlg(this);
     add(dlg);
     dlg->execute();
     NApp::addRemovePipe(dlg);
  }
}

Machine * FrameMachine::pMac( )
{
  return pMachine_;
}

void FrameMachine::updateValues( )
{
  int numParameters = pMachine_->GetNumParams();
  int cols = pMachine_->GetNumCols();
  int rows = numParameters/cols;


  for (int c=0; c<numParameters; c++) {
     int min_v,max_v,val_v;
     int newC = format(c,cols,rows);
     pMachine_->GetParamRange(newC,min_v,max_v);
     bool bDrawKnob = (min_v==max_v)?false:true;
     if (!bDrawKnob) {
     } else {
        Knob* cell = (Knob*) knobPanel->visualComponents().at(c);
        char buffer[128];
        pMachine_->GetParamValue(format(c,cols,rows),buffer);
        int val_v = pMachine_->GetParamValue(format(c,cols,rows));
        cell->setValue(val_v);
        int min_v; int max_v;
        pMachine_->GetParamRange(format(c,cols,rows),min_v,max_v);
        cell->setRange(min_v,max_v);
        cell->setValueAsText(buffer);
     }
  }

  knobPanel->repaint();
}



void FrameMachine::loadPresets() {
  std::string filename(pMac()->GetDllName());

  std::string::size_type pos = filename.find('.')  ;
  if ( pos == std::string::npos ) {
    filename  = filename + '.' + "prs";
  } else {
    filename = filename.substr(0,pos)+".prs";
  }

  try {
     DeSerializer f(Global::pConfig()->prsPath+filename);

     int numpresets = f.getInt();
     int filenumpars = f.getInt();

     if (numpresets >= 0) {
       // old file format .. do not support so far ..
     } else {
       // new file format
       if (filenumpars == 1) {
         int filepresetsize;
         // new preset format version 1
         // new preset format version 1

         int numParameters = ((Plugin*) pMac())->GetInfo()->numParameters;
         int sizeDataStruct = ((Plugin *) pMac())->proxy().GetDataSize();

         numpresets = f.getInt();
         filenumpars = f.getInt();
         filepresetsize = f.getInt();

         if (( filenumpars != numParameters )  || (filepresetsize != sizeDataStruct)) return;

         while (!f.eof() ) {
           Preset newPreset(numParameters, sizeDataStruct);
           newPreset.loadFromFile(&f);
           NButton* prsBtn = new NButton(newPreset.name());
             prsBtn->setFlat(false);
             prsBtn->clicked.connect(this,&FrameMachine::onPrsClick);
           prsPanel->add(prsBtn);
           presetMap[prsBtn] = newPreset;
         }
       }
     }
  } catch (const char * e) {
     // couldn`t open presets
  }
}

void FrameMachine::onLeftBtn( NButtonEvent * ev )
{
  prsPanel->setScrollDx(std::max(0,prsPanel->scrollDx()-100));
  prsPanel->repaint();
}

void FrameMachine::onRightBtn( NButtonEvent * ev )
{
  prsPanel->setScrollDx(std::min(prsPanel->preferredWidth() - prsPanel->clientWidth(),prsPanel->scrollDx()+100));
  prsPanel->repaint();
}

void FrameMachine::onPrsClick( NButtonEvent * ev )
{
  std::map<NButton*,Preset>::iterator itr;
  if ( (itr = presetMap.find((NButton*)ev->sender())) != presetMap.end() ) {
        itr->second.tweakMachine(pMac() );
    }
  updateValues();
}

void FrameMachine::onAddPrs( NButtonEvent * ev )
{
  NewNameDlg* dlg = new NewNameDlg();
  add(dlg);
  dlg->execute();
  NApp::addRemovePipe(dlg);
}

void FrameMachine::onLoadPrs( NButtonEvent * ev )
{
  NFileDialog* dlg = new NFileDialog();
  add(dlg);
  dlg->setPosition(10,10,500,500);
  dlg->execute();
  NApp::addRemovePipe(dlg);
}

Preset FrameMachine::knobsPreset( )
{
  Preset prs;

  return prs;
}



