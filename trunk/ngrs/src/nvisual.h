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
#ifndef NVISUAL_H
#define NVISUAL_H

#include <nruntime.h>
#include <nshape.h>
#include <nkeyevent.h>
#include <nbuttonevent.h>

/**
@author Stefan
*/
class NVisual : public NRuntime
{
public:
    NVisual();

    ~NVisual();

    virtual void setVisible(bool on);
    int visible();

    virtual void setPosition(int x, int y, int width, int height);
    virtual void setPreferredSize(int width, int height);
    virtual void setPosition(const NRect & pos);
    virtual void setLeft(int left);
    virtual void setTop(int top);
    virtual void setWidth(int width);
    virtual void setHeight(int height);

    virtual void setMinimumWidth (int minWidth);
    virtual void setMinimumHeight(int minHeight);

    virtual long top()  const;
    virtual long left() const;

    // events ..

    signal1<NButtonEvent* > mousePress;
    signal1<NButtonEvent* > mousePressed;
    signal1<NEvent* > mouseEnter;
    signal1<NEvent* > mouseExit;

    virtual void onMousePress  (int x, int y, int button);
    virtual void onMousePressed(int x, int y, int button);
    virtual void onMouseDoublePress(int x, int y, int button);
    virtual void onMouseOver(int x, int y);
    virtual void onMouseExit();
    virtual void onMouseEnter();

    virtual void setFocus();
    virtual bool focus() const;
    virtual void onFocus();

    virtual void onKeyPress(const NKeyEvent & event);

    NShape* geometry();
    NShape* geometry() const;
    void setGeometry(NShape* geometry);

    virtual int preferredWidth() const;
    virtual int preferredHeight() const;

    virtual int minimumWidth() const;
    virtual int minimumHeight() const;

    virtual int width() const;
    virtual int height() const;

    virtual void resize();

    virtual int absoluteLeft() const;
    virtual int absoluteTop() const;

    virtual NRect clipBox() const;


private:

    NShape* geometry_;
    bool visible_;
    int minWidth_, minHeight_;

};

#endif
