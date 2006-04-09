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
#ifndef NCHECKMENUITEM_H
#define NCHECKMENUITEM_H

#include <ncustommenuitem.h>


class NCheckBox;
class NLabel;
class NMenu;
class NButton;


/**
@author Stefan
*/
class NCheckMenuItem : public NCustomMenuItem
{
public:
    NCheckMenuItem();
    NCheckMenuItem(const std::string & text);

    ~NCheckMenuItem();

    virtual int maxIconSize();
    virtual int maxCaptionSize();
    virtual int maxShortCutSize();
    virtual int maxHintSize();

    virtual void setIconSize(int size);
    virtual void setCaptionSize(int size);
    virtual void setShortCutSize(int size);
    virtual void setHintSize(int size);

    virtual int preferredWidth() const;
    virtual int preferredHeight() const;

    virtual void setHeight( int height );

    virtual void onMouseEnter();
    virtual void onMouseExit();

private:

    NCheckBox* checkBox_;
    NLabel*    caption_;

    NSkin itemNone;
    NSkin itemOver;

    void init();
    void onCheckBoxClicked(NButtonEvent* ev);

    void onCheckBoxMouseEnter(NEvent* ev);
    void onCheckBoxMouseExit(NEvent* ev);

};

#endif
