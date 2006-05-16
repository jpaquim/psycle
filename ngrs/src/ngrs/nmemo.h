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
#ifndef NMEMO_H
#define NMEMO_H

#include <ntextbase.h>

/**
@author Stefan Nattkemper
*/


class NMemo : public NTextBase
{
public:
    NMemo();

    ~NMemo();

    virtual std::string text() const;
    void setText(const std::string & text);

    void loadFromFile(const std::string & fileName);
    void setReadOnly(bool on);

private:

    int oldPos_;

    NPanel* scrollPane_;
    void clear();

    void onKeyPress(const NKeyEvent & event);

    std::vector<class NEdit*> edits;

};

#endif
