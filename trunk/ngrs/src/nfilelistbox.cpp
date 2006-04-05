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

/* XPM */
const char * group_xpm[] = {
"16 16 10 1",
" 	c gray",
".	c #020204",
"+	c #826E54",
"@	c #9A8A76",
"#	c #AD9E89",
"$	c #BAAE98",
"%	c #E0D9CF",
"&	c #D2CAC0",
"*	c #C6BEB0",
"=	c #EDE9DF",
"                ",
" ....           ",
".++@#.          ",
".+##$.......    ",
".....%&&**$#.   ",
".==%%&&**$$#.   ",
".=%%&&*$$$##.   ",
".%%&&**$$##@.   ",
".%&&*$$$##@@.   ",
".&&**$$##@@+.   ",
"..*$$$##@@++.   ",
" ...........    ",
"                ",
"                ",
"                ",
"                "};


#include "nfilelistbox.h"
#include "nlabel.h"
#include "nflowlayout.h"
#include "nitem.h"

NFileListBox::NFileListBox()
 :  NListBox()
{
  isDirItem_ = false;
  mode_ = nFiles | nDirs;
  setTransparent(false);
  setBackground(NColor(255,255,255));
  sharedDirIcon_.createFromXpmData(group_xpm);
}


NFileListBox::~NFileListBox()
{
}


void NFileListBox::setDirectory( std::string directory )
{
  std::string oldDir = NFile::workingDir();

  removeChilds();

  std::vector<std::string> list;
  if (mode_ & nFiles) {
      list = fSystem.fileList(directory);
      for (std::vector<std::string>::iterator it = list.begin(); it < list.end(); it++) {
        std::string entry = *it;
        NItem* item = new NItem();
        item->setText(entry);
        item->mousePress.connect(this,&NFileListBox::onFileItemSelected);
        add (item, false);
      }
  }
  if (mode_ & nDirs) {
      std::vector<std::string> list = fSystem.dirList(directory);
      for (std::vector<std::string>::iterator it = list.begin(); it < list.end(); it++) {
        std::string entry = *it;
        NItem* item = new NItem();
          item->setText(entry);
          NImage* icon = new NImage();
            icon->setSharedBitmap(&sharedDirIcon_);
          item->add(icon);
          item->mousePress.connect(this,&NFileListBox::onDirItemSelected);
        add (item, false);
      }
  }

  NFile::cd(oldDir);
  resize();
}

void NFileListBox::onItemSelected( NCustomItem * item )
{
  fName_ = item->text();
  NListBox::onItemSelected(item);
}

std::string NFileListBox::fileName( )
{
  return fName_;
}

void NFileListBox::setMode( int mode )
{
  mode_ = mode;
}

bool NFileListBox::isDirItem( )
{
  return isDirItem_;
}


// signal connections

void NFileListBox::onDirItemSelected( NButtonEvent * ev )
{
  isDirItem_ = true;
}

void NFileListBox::onFileItemSelected( NButtonEvent * ev )
{
  isDirItem_ = false;
}





