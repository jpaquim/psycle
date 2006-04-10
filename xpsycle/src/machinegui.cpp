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
#include "machinegui.h"
#include "machine.h"
#include "framemachine.h"
#include "masterdlg.h"
#include "global.h"
#include "configuration.h"
#include "defaultbitmaps.h"
#include <nframeborder.h>
#include <nlabel.h>
#include <nline.h>
#include <nwindow.h>
#include <nslider.h>

NBitmap MachineGUI::bitmap;
int MachineGUI::c = 0;

MachineGUI::MachineGUI(Machine* mac)
 : NPanel()
{
  line = 0;
  mac_ = mac;
  setMoveable(NMoveable(nMvHorizontal | nMvVertical | nMvNoneRepaint));
  setPosition(mac->_x,mac_->_y,200,30);

  if (c==0) {
    if (Global::pConfig()->iconPath=="") 
       bitmap = Global::pBitmaps()->machine_skin(); 
    else
       bitmap.loadFromFile(Global::pConfig()->iconPath+ "machine_skin.xpm");
  }
  c++;
  setFont(NFont("Suse sans",6, nMedium | nStraight | nAntiAlias));
}


MachineGUI::~MachineGUI()
{

}

Machine * MachineGUI::pMac( )
{
  return mac_;
}

void MachineGUI::attachLine( NLine * line, int point )
{
  attachedLines.push_back(LineAttachment(line,point));
  int midW = clientWidth()  / 2;
  int midH = clientHeight() / 2;
   if (point == 1) {
        line->setPoints(NPoint(left()+midW,top()+midH),line->p2());
    } else {
        line->setPoints(line->p1(),NPoint(left()+midW,top()+midH));
    }

}

NRegion MachineGUI::linesClipBox( )
{
  NRegion region(geometry()->rectArea());

  for (std::vector<LineAttachment>::iterator itr = attachedLines.begin(); itr < attachedLines.end(); itr++) {
    LineAttachment lineAttach = *itr;
    region = region | lineAttach.line->geometry()->region();
  }
  return region;
}

void MachineGUI::onMoveStart( const NMoveEvent & moveEvent )
{
  oldDrag = linesClipBox();
}

void MachineGUI::onMove( const NMoveEvent & moveEvent )
{
  NRegion newDrag = linesClipBox();
  NRegion repaintArea = newDrag | oldDrag;

  int parentAbsLeft = ((NVisualComponent*) parent())->absoluteLeft() - ((NVisualComponent*) parent())->scrollDx();
  int parentAbsTop  = ((NVisualComponent*) parent())->absoluteTop() - ((NVisualComponent*) parent())->scrollDy();;

  repaintArea.move(parentAbsLeft, parentAbsTop);

  window()->repaint((NVisualComponent*) parent(),repaintArea);

  oldDrag = newDrag;

  if (pMac()) {
    pMac()->_x = left();
    pMac()->_y = top();
  }
}

void MachineGUI::resize( )
{
  for (std::vector<LineAttachment>::iterator itr = attachedLines.begin(); itr < attachedLines.end(); itr++) {
    LineAttachment lineAttach = *itr;
    int midW = clientWidth() / 2;
    int midH = clientHeight() / 2;
    if (lineAttach.point == 1) {
        lineAttach.line->setPoints(NPoint(left() + midW,top()+midH),lineAttach.line->p2());
    } else {
        lineAttach.line->setPoints(lineAttach.line->p1(),NPoint(left()+midW,top()+midH));
    }
  }
}


MasterGUI::MasterGUI(Machine* mac) : MachineGUI(mac)
{
  setSkin();
  masterDlg = new MasterDlg(mac);
  setBackground(NColor(0,0,200));
}

MasterGUI::~ MasterGUI( )
{
}

void MasterGUI::setSkin( )
{
  bgCoords.setPosition(0,0,148,48);
  setTransparent(true);
  setHeight(bgCoords.height());
  setWidth(bgCoords.width());
  setBackground(NColor(0,0,200));
}

void MasterGUI::paint( NGraphics * g )
{
  g->putBitmap(0,0,bgCoords.width(),bgCoords.height(), bitmap, bgCoords.left(), bgCoords.top());
}

GeneratorGUI::GeneratorGUI(Machine* mac) : MachineGUI(mac)
{
  vuSlider_ = new NSlider();
    vuSlider_->posChanged.connect(this,&GeneratorGUI::onPosChanged);
  add(vuSlider_);

  setSkin();
  frameMachine = new FrameMachine(pMac());
}

GeneratorGUI::~ GeneratorGUI( )
{
}

void GeneratorGUI::paint( NGraphics * g )
{
  g->putBitmap(0,0,bgCoords.width(),bgCoords.height(), bitmap, bgCoords.left(), bgCoords.top());
  g->drawText(dNameCoords.x(),dNameCoords.y()+g->textAscent(), stringify(pMac()->_macIndex)+":"+pMac()->_editName);
}

void GeneratorGUI::setSkin( )
{
  bgCoords.setPosition(0,47,148,47);
  dNameCoords.setXY(49,7);
  setHeight(bgCoords.height());
  setWidth(bgCoords.width());

  vuSlider_->setPosition(45,23,96,10);
  vuSlider_->setOrientation(nHorizontal);
  vuSlider_->setRange(0,127);
  vuSlider_->setPos( pMac()->_panning );

}

void GeneratorGUI::onPosChanged(NSlider* sender, double value )
{
  if (pMac()) {
    pMac()->SetPan( (int) vuSlider_->pos());
  }
}



EffektGUI::EffektGUI(Machine* mac ) : MachineGUI(mac)
{
  vuSlider_ = new NSlider();
    vuSlider_->posChanged.connect(this,&EffektGUI::onPosChanged);
  add(vuSlider_);

  setSkin();
  frameMachine = new FrameMachine(pMac());
}

EffektGUI::~ EffektGUI( )
{
}

void EffektGUI::paint( NGraphics * g )
{
  g->putBitmap(0,0,bgCoords.width(),bgCoords.height(), bitmap, bgCoords.left(), bgCoords.top());
  g->drawText(dNameCoords.x(),dNameCoords.y()+g->textAscent(), pMac()->_editName);
}

void EffektGUI::setSkin( )
{
  bgCoords.setPosition(0,94,148,47);
  dNameCoords.setXY(49,7);
  setHeight(bgCoords.height());
  setWidth(bgCoords.width());
  setTransparent(true);

  vuSlider_->setPosition(45,23,96,10);
  vuSlider_->setOrientation(nHorizontal);
  vuSlider_->setRange(0,127);
  vuSlider_->setPos( pMac()->_panning );

}


void EffektGUI::onPosChanged(NSlider* sender, double value )
{
  if (pMac()) {
    std::cout << vuSlider_->pos() << std::endl;
    pMac()->SetPan( (int) vuSlider_->pos());
  }
}


void MachineGUI::onMousePress( int x, int y, int button )
{
  if (button==3) newConnection.emit(this);
}

void MachineGUI::detachLine( NLine * line )
{
  std::vector<LineAttachment>::iterator it = attachedLines.begin();
  for (;it <  attachedLines.end(); it++) {
    LineAttachment lineAttachment = *it;

     if (lineAttachment.line == line) {
       attachedLines.erase(it); 
       break;
     }
  }
}

void MachineGUI::onMouseDoublePress( int x, int y, int button )
{

}

void GeneratorGUI::onMouseDoublePress( int x, int y, int button )
{
  frameMachine->setVisible(true);
}

void MasterGUI::onMouseDoublePress( int x, int y, int button )
{
  masterDlg->setVisible(true);
}

void EffektGUI::onMouseDoublePress( int x, int y, int button )
{
  frameMachine->setVisible(true);
}










