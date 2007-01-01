/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by Stefan Nattkemper   *
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
#ifndef NIMAGE_H
#define NIMAGE_H

#include "npanel.h"
#include "nbitmap.h"
#include "nimgfilter.h"

/**
@author Stefan
*/
class NImage : public NPanel
{
public:
    NImage();
    NImage(const std::string & fileName);
    NImage(const NBitmap & bitmap);

    ~NImage();

    virtual void paint(NGraphics* g);

    void loadFromFile( const std::string & filename);
    void createFromXpmData(const char** data);

    void setBitmap(const NBitmap & bitmap);

    void setSharedBitmap(NBitmap* bitmap);

    virtual void setHAlign(int align);
    virtual void setVAlign(int align);

    virtual int preferredWidth()  const;
    virtual int preferredHeight() const;


private:

    NBitmap bitmap24bpp_;
    NBitmap* pBitmap_;

    int halign_;
    int valign_;

};

#endif
