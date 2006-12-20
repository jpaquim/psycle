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
"15 15 10 1",
" 	c None",
".	c #020204",
"+	c #826E54",
"@	c #9A8A76",
"#	c #FFFFFF",
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
  sharedDirIcon_.createFromXpmData( group_xpm );
  activeFilter = 0;
  showHiddenFiles_ = false;
}


NFileListBox::~NFileListBox()
{
 std::map<std::string,NRegExp*>::iterator itr;
 for (itr = filterMap.begin(); itr != filterMap.end(); itr++) {
    NRegExp* exp = itr->second;
    delete exp;
 }
}


void NFileListBox::setDirectory( const std::string & directory )
{
  dir_ = directory;
}

const std::string & NFileListBox::directory( ) const
{
  return dir_;
}

void NFileListBox::update( )
{
  std::string oldDir = NFile::workingDir();
  removeChilds();
  std::vector<std::string> list;

  if ( mode_ & nFiles ) {
	  list = NFile::fileList( directory(), nFiles );
      for ( std::vector<std::string>::iterator it = list.begin(); it < list.end(); it++ ) {
        std::string entry = *it;

        if (activeFilter==0  || (activeFilter && activeFilter->accept(entry)))
        {
          if (!showHiddenFiles_ && entry!="" && (entry[0]!='.') || showHiddenFiles_) {
            NItem* item = new NItem();
            item->setText(entry);
            item->mousePress.connect(this,&NFileListBox::onFileItemSelected);
            add (item, false);
          }
        }
      }
  }
  if ( mode_ & nDirs ) {
	  std::vector<std::string> list = NFile::fileList( directory(), nDirs );
      for ( std::vector<std::string>::iterator it = list.begin(); it < list.end(); it++ ) {
        std::string entry = *it;
        if ((showHiddenFiles_) || (!showHiddenFiles_ && entry!="" && ((entry[0]!='.') || entry.find("..")==0))) {
          NItem* item = new NItem();
            item->setText(entry);
            NImage* icon = new NImage();
               icon->setSharedBitmap(&sharedDirIcon_);
            item->add(icon);
            item->mousePress.connect(this,&NFileListBox::onDirItemSelected);
          add (item, false);
        }
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

const std::string & NFileListBox::fileName( ) const
{
  return fName_;
}

void NFileListBox::setMode( int mode )
{
  mode_ = mode;
}

bool NFileListBox::isDirItem( ) const
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

void NFileListBox::addFilter( const std::string & name, const std::string & regexp )
{
  NRegExp* exp = new NRegExp();
  exp->setRegExp(regexp);
  filterMap[name] = exp;
}

void NFileListBox::setActiveFilter( const std::string & name )
{
  std::map<std::string,NRegExp*>::iterator itr;
  if ( (itr = filterMap.find(name)) != filterMap.end())
  {
    activeFilter = itr->second;
  } else {
    activeFilter = 0;
  }
  setDirectory(dir_);
}

void NFileListBox::setShowHiddenFiles( bool on )
{
  showHiddenFiles_ = on;
  setDirectory(dir_);
}




