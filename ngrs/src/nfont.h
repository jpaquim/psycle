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
#ifndef NFONT_H
#define NFONT_H

#include <string>
#include <ncolor.h>
#include <nruntime.h>
#include <nfontstructure.h>


/**
@author Stefan
*/

enum NFontStyleEnum { nBold=1,nMedium=2, nItalic=4, nStraight=8, nAntiAlias=16};

class NFont{
public:
    NFont();
    NFont(std::string name,int size);
    NFont::NFont( std::string name, int size=10,int style=nMedium | nStraight | nAntiAlias);

    ~NFont();

    void setName(std::string name);
    void setSize(int size);
    void setStyle(int style);

    std::string name() const;
    int size() const;
    int style() const;
    bool antialias() const;
    std::string fontString( ) const;
    bool operator==(const NFont & fnt) const;
    bool operator<(const NFont & fnt) const;

    void setTextColor(const NColor & color);
    const NColor & textColor() const;

    const NFontStructure & systemFont() const;



private:

    std::string name_;
    int size_, style_;
    NColor textColor_;

    NFontStructure systemFnt;

};

#endif
