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
#ifndef NEDIT_H
#define NEDIT_H

#include <npanel.h>
#include <string>
#include <nfontmetrics.h>

/**
@author Stefan
*/
class NEdit : public NPanel
{
public:
    NEdit();

    ~NEdit();

    void setText(std::string text);
    std::string text();

    virtual void paint(NGraphics* g);
    virtual void onKeyPress(const NKeyEvent & keyevent);

    virtual int preferredWidth() const;
    virtual int preferredHeight() const;

    virtual void onFocus();

    void setVAlign(int align);
    void setHAlign(int align);

    int vAlign() const;
    int hAlign() const;

    int pos() const;

private:

    int valign_, halign_;
    int dx;
    int pos_;
    int selStartIdx_;
    int selEndIdx_;

    std::string text_;
    NFontMetrics metrics;

    NPoint getScreenPos(NGraphics* g, const std::string & text );
    int computeDx( NGraphics* g, const std::string & text );

    void drawCursor(NGraphics* g, const std::string & text );

    void init();

};

#endif
