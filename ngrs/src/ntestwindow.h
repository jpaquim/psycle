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
#ifndef NTESTWINDOW_H
#define NTESTWINDOW_H

#include <nwindow.h>
#include "nmenubar.h"
#include "nmenuitem.h"
#include "nscrollbox.h"
#include "nedit.h"
#include "nfile.h"
#include "nprogressbar.h"
#include "nslider.h"
#include "nimage.h"
#include "nslider.h"
#include "nnotebook.h"
#include "npage.h"
#include "ntabbook.h"
#include "ncheckbox.h"
#include "nfilelistbox.h"
#include "nfiledialog.h"

/**
@author Stefan
*/

class Tpnl : public NPanel {

   virtual void paint(NGraphics* g) {
      XPoint pol[5];

      pol[0].x = 0;
      pol[0].y = 0;
      pol[1].x = 10;
      pol[1].y = 0;
      pol[2].x = 10;
      pol[2].y = 10;
      pol[3].x = 0;
      pol[3].y = 10;

      g->drawPolygon(pol,4);

   }

};


class NTestWindow : public NWindow
{
public:
    NTestWindow();

    ~NTestWindow();

private:

    NProgressBar* pBar;
    NSlider* slider;
    NWindow* scrollWin;
    NFileListBox* fileBox;
    NFileDialog* fDialog;

    void testSpinButton();
    void testBorderLayout();
    void testFrameBorder();
    void testLine();
    void testMenu();
    void testEdit();

    void onOpen(NButtonEvent* ev);
    void onSliderPosChanged(double v);
    void onQuit(NObject* sender);
};

#endif
