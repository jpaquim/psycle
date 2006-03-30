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


/**
@author Stefan
*/
class NCheckMenuItem : public NCustomMenuItem
{
public:
    NCheckMenuItem();
    NCheckMenuItem(std::string text);
    NCheckMenuItem(std::string text, int keyMask, char c, std::string description );


    ~NCheckMenuItem();
    void setText(std::string text);
    std::string text();
    void setShortCut(int keyMask, char c, std::string description );


    void onMouseEnter();
    void onMouseExit();
    virtual void add(NRuntime* comp);
    void add(NMenu* menu);

    virtual int preferredWidth() const;
    virtual int preferredHeight() const;

    virtual int maxIconSize();
    virtual int maxCaptionSize();
    virtual int maxShortCutSize();
    virtual int maxHintSize();

    virtual void setIconSize(int size);
    virtual void setCaptionSize(int size);
    void setShortCutSize(int size);
    virtual void setMnemonic(char c);
    virtual void setHintSize(int size);

    virtual void setHeight(int height);
    virtual void onKeyAcceleratorNotify(NKeyAccelerator accelerator);

    void setGap(int gap);

    virtual void onMousePress(int x, int y, int button);


private:

    int gap_;

    NBorder* border_;
    NPanel* iconField_;
    NCheckBox* caption_;
    NLabel* shortCut_;
    NLabel* subMenuHint_;
    NMenu* menu_;

    void init();

};

#endif
