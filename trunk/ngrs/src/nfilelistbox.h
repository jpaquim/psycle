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
#ifndef NFILELISTBOX_H
#define NFILELISTBOX_H

#include <nlistbox.h>
#include <nfile.h>

const int nFiles = 1;
const int nDirs  = 2;

/**
@author Stefan
*/
class NFileListBox : public NListBox
{
public:
    NFileListBox();

    ~NFileListBox();

    void setDirectory(std::string directory);

    virtual void onItemSelected(NCustomItem * item);

    std::string fileName();

    void setMode(int mode);

    bool isDirItem();

private:

    bool isDirItem_;
    int mode_;
    NFile fSystem;
    std::string fName_;
    NBitmap sharedDirIcon_;

    void onDirItemSelected(NButtonEvent* ev);
    void onFileItemSelected(NButtonEvent* ev);
};

#endif
