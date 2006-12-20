/***************************************************************************
 *   Copyright (C) 2005, 2006 by Stefan Nattkemper  *
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
#ifndef NFILELISTBOX_H
#define NFILELISTBOX_H

#include "nlistbox.h"
#include "nfile.h"
#include "nregexp.h"

/**
@author Stefan
*/

class NFileListBox : public NListBox
{
public:
    NFileListBox();

    ~NFileListBox();

    void setDirectory( const std::string & directory );
    const std::string & directory() const;

    void update();

    virtual void onItemSelected( NCustomItem * item );

    const std::string & fileName() const;

    void setMode( int mode );

    bool isDirItem() const;

    void setActiveFilter( const std::string & name );
    void addFilter( const std::string & name, const std::string & regexp );

    void setShowHiddenFiles( bool on );


private:

    bool isDirItem_;
    bool showHiddenFiles_;
    int mode_;    
    std::string fName_;
    NBitmap sharedDirIcon_;
    NRegExp* activeFilter;
    std::map<std::string,NRegExp*> filterMap;
    std::string dir_;

    void onDirItemSelected( NButtonEvent* ev );
    void onFileItemSelected( NButtonEvent* ev );

};

#endif
