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
      NFntString str;
      str.setText("Hallo");
      str.setFont(NFont("Suse Sans",8,nMedium | nStraight | nAntiAlias));
      str.append("Hier");
      g->drawText(10,10,str.substr(5,1));
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

    void testListBox();
    void toolBarTest();
    void testSpinButton();
    void testBorderLayout();
    void testFrameBorder();
    void testLine();
    void testMenu();
    void testEdit();
    void testSpinEdit();

    void onOpen(NButtonEvent* ev);
    void onSliderPosChanged(double v);
    void onQuit(NObject* sender);
};

#endif
