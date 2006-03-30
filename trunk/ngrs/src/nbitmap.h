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
#ifndef NBITMAP_H
#define NBITMAP_H

#include <nobject.h>
#include <nsystem.h>

/**
@author Stefan
*/

typedef unsigned char* pRGBA;

class NBitmap : public NObject
{
public:
    NBitmap();
    NBitmap(const std::string & filename);
    NBitmap(const NBitmap & src);
    const NBitmap & operator= (const NBitmap & rhs);

    ~NBitmap();

    unsigned char const * dataPtr() const;

    int depth()     const;
    int pixelsize() const;
    int width()     const;
    int height()    const;

    void setDepth(int depth);
    void setSize(int width, int height);

    XImage* X11data() const;
    XImage* X11ClpData() const;

    void setX11Data(XImage* ximage, XImage* clp_);

    void loadFromFile(std::string filename);
    void createFromXpmData(const char** data);


private:

    int depth_;
    int width_;
    int height_;
    pRGBA data_;

    XImage* xi;
    XImage* clp;
};

#endif
