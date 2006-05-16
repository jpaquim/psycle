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
#ifndef NFNTSTRING_H
#define NFNTSTRING_H

#include <vector>
#include <string>

#include "ngrs/nfont.h"


/**
@author Stefan Nattkemper
*/

class NFntString : public std::string {
public:
    NFntString();

    ~NFntString();

    void setText(const std::string & text);
    void append(const std::string & text);
    void append(const NFntString & text);

    NFntString substr(int pos, int size) const;
    NFntString substr(int last) const;

    std::string textsubstr(int pos, int size) const;
    std::string textsubstr(int last) const;

    void setFont(const NFont & font);

    const std::vector<int> & positions() const;
    const std::vector<NFont> & fonts() const;

private:

   std::vector<int>   positions_;
   std::vector<NFont> fonts_;

};

#endif
