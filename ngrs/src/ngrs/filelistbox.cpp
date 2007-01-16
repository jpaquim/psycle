/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by Stefan Nattkemper  *
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
#include "filelistbox.h"
#include "label.h"
#include "flowlayout.h"
#include "item.h"

namespace ngrs {

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
    "                "}
  ;


  FileListBox::FileListBox()
    :  ListBox()
  {
    isDirItem_ = false;
    mode_ = nFiles | nDirs;
    setTransparent(false);
    setBackground(Color(255,255,255));
    sharedDirIcon_.createFromXpmData( group_xpm );
    activeFilter = 0;
    showHiddenFiles_ = false;
  }


  FileListBox::~FileListBox()
  {
    std::map<std::string,RegExp*>::iterator itr;
    for (itr = filterMap.begin(); itr != filterMap.end(); itr++) {
      RegExp* exp = itr->second;
      delete exp;
    }
  }


  void FileListBox::setDirectory( const std::string & directory )
  {
    dir_ = directory;
  }

  const std::string & FileListBox::directory( ) const
  {
    return dir_;
  }

  void FileListBox::update( )
  {
    std::string oldDir = ngrs::File::workingDir();
    removeChilds();
    std::vector<std::string> list;

    if ( mode_ & nFiles ) {
      list = ngrs::File::fileList( directory(), nFiles );
      for ( std::vector<std::string>::iterator it = list.begin(); it < list.end(); it++ ) {
        std::string entry = *it;

        if (activeFilter==0  || (activeFilter && activeFilter->accept(entry)))
        {
          if (!showHiddenFiles_ && entry!="" && (entry[0]!='.') || showHiddenFiles_) {
            Item* item = new Item();
            item->setText(entry);
            item->mousePress.connect(this,&FileListBox::onFileItemSelected);
            add (item, false);
          }
        }
      }
    }
    if ( mode_ & nDirs ) {
      std::vector<std::string> list = ngrs::File::fileList( directory(), nDirs );
      for ( std::vector<std::string>::iterator it = list.begin(); it < list.end(); it++ ) {
        std::string entry = *it;
        if ((showHiddenFiles_) || (!showHiddenFiles_ && entry!="" && ((entry[0]!='.') || entry.find("..")==0))) {
          Item* item = new Item();
          item->setText(entry);
          Image* icon = new Image();
          icon->setSharedBitmap(&sharedDirIcon_);
          item->add(icon);
          item->mousePress.connect(this,&FileListBox::onDirItemSelected);
          add (item, false);
        }
      }
    }

    ngrs::File::cd(oldDir);
    resize();
  }


  void FileListBox::onItemSelected( CustomItem * item )
  {
    fName_ = item->text();
    ListBox::onItemSelected(item);
  }

  const std::string & FileListBox::fileName( ) const
  {
    return fName_;
  }

  void FileListBox::setMode( int mode )
  {
    mode_ = mode;
  }

  bool FileListBox::isDirItem( ) const
  {
    return isDirItem_;
  }


  // signal connections

  void FileListBox::onDirItemSelected( ButtonEvent * ev )
  {
    isDirItem_ = true;
  }

  void FileListBox::onFileItemSelected( ButtonEvent * ev )
  {
    isDirItem_ = false;
  }

  void FileListBox::addFilter( const std::string & name, const std::string & regexp )
  {
    RegExp* exp = new RegExp();
    exp->setRegExp(regexp);
    filterMap[name] = exp;
  }

  void FileListBox::setActiveFilter( const std::string & name )
  {
    std::map<std::string,RegExp*>::iterator itr;
    if ( (itr = filterMap.find(name)) != filterMap.end())
    {
      activeFilter = itr->second;
    } else {
      activeFilter = 0;
    }
    setDirectory(dir_);
  }

  void FileListBox::setShowHiddenFiles( bool on )
  {
    showHiddenFiles_ = on;
    setDirectory(dir_);
  }

}
