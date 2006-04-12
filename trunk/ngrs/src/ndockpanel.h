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
#ifndef NDOCKPANEL_H
#define NDOCKPANEL_H

#include <npanel.h>

class NImage;

/**
@author Stefan Nattkemper
*/
class NDockPanel : public NPanel
{
public:
    NDockPanel();

    ~NDockPanel();

    virtual void add(NVisualComponent* comp);
    virtual void setLayout(NLayout* layout);
    virtual void setFont(const NFont & font);
    virtual void setBackground(const NColor & background);
    virtual void setTransparent(bool on);

    void onDockWindow();

private:

    NBitmap undockBmp;
    NBitmap dockBmp;

    NPanel* dockBar_;
    NPanel* area_;

    NImage* dockImg;

    NLayout* alignLayout;
    NLayout* flowLayout;

    NLayout* oldAreaLayout_;

    NWindow* undockedWindow;

    void onUndockWindow(NButtonEvent* ev);
};

#endif
