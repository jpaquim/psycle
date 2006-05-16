/***************************************************************************
 *   Copyright (C) 2006 by Stefan Nattkemper   *
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
#include "nmemo.h"
#include "nalignlayout.h"
#include "nlistlayout.h"
#include "nscrollbox.h"
#include "nedit.h"
#include "nwindow.h"
#include <fstream>
#include <sstream>

NMemo::NMemo()
 : NTextBase()
{
  setLayout(NAlignLayout());
  NScrollBox* box = new NScrollBox();
    scrollPane_ = new NPanel();
       scrollPane_->setClientSizePolicy(nVertical | nHorizontal);
       scrollPane_->setLayout(NListLayout());
    box->setScrollPane(scrollPane_);
  add(box, nAlClient);

  clear();
}


NMemo::~NMemo()
{
}

std::string NMemo::text( ) const
{
  return "";
}

void NMemo::clear( )
{
  NEdit* edt = new NEdit();
    edt->setAutoSize(true);
    edt->keyPress.connect(this, &NMemo::onKeyPress);
  scrollPane_->add(edt);
  edits.push_back(edt);
}

void NMemo::onKeyPress( const NKeyEvent & event )
{
  if (event.scancode() == XK_Return) {
   int index = window()->selectedBase()->zOrder();
   NEdit* edt = new NEdit();
    edt->setAutoSize(true);
    edt->keyPress.connect(this, &NMemo::onKeyPress);
   scrollPane_->insert(edt,index+1);
   edits.insert(edits.begin()+index+1,edt);
   edt->setFocus();
   scrollPane_->resize();
   scrollPane_->repaint();
  }
}

void NMemo::loadFromFile( const std::string & fileName )
{
  std::fstream file(fileName.c_str());
  if (!file.is_open ()) throw "couldn't open file";

  edits.clear();
  scrollPane_->removeChilds();

  std::string line;

  while(!file.eof())
  {
    getline(file, line, '\n');
    //append data
    NEdit* edt = new NEdit(line);
      edt->setAutoSize(true);
      edt->keyPress.connect(this, &NMemo::onKeyPress);
     scrollPane_->add(edt,nAlNone,false);
     edits.push_back(edt);
  }
  scrollPane_->resize();
  scrollPane_->repaint();
}

void NMemo::setText( const std::string & text )
{
  edits.clear();
  scrollPane_->removeChilds();

  std::string delimiters = "\n";
  // Skip delimiters at beginning.
  std::string::size_type lastPos = text.find_first_not_of(delimiters, 0);
  // Find first "non-delimiter".
  std::string::size_type pos     = text.find_first_of(delimiters, lastPos);

  while (std::string::npos != pos || std::string::npos != lastPos)
  {
     // Found a token, add it to the vector.
     NEdit* edt = new NEdit(text.substr(lastPos, pos - lastPos));
      edt->setAutoSize(true);
      edt->keyPress.connect(this, &NMemo::onKeyPress);
     scrollPane_->add(edt,nAlNone,false);
     edits.push_back(edt);
     // Skip delimiters.  Note the "not_of"
     lastPos = text.find_first_not_of(delimiters, pos);
     // Find next "non-delimiter"
     pos = text.find_first_of(delimiters, lastPos);
  }
  scrollPane_->resize();
  scrollPane_->repaint();
}

void NMemo::setReadOnly( bool on )
{
  std::vector<NEdit*>::iterator it = edits.begin();
  for (;it < edits.end(); it++) {
    NEdit* edt = *it;
    edt->setReadOnly(on);
  }
}




