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

#include <ngrs/nwindow.h>
#include <ngrs/nmenubar.h>
#include <ngrs/nmenuitem.h>
#include <ngrs/nscrollbox.h>
#include <ngrs/nedit.h>
#include <ngrs/nfile.h>
#include <ngrs/nprogressbar.h>
#include <ngrs/nslider.h>
#include <ngrs/nimage.h>
#include <ngrs/nslider.h>
#include <ngrs/nnotebook.h>
#include <ngrs/ntabbook.h>
#include <ngrs/ncheckbox.h>
#include <ngrs/nfilelistbox.h>
#include <ngrs/nfiledialog.h>

/**
@author Stefan
*/

class Tpnl : public ngrs::NPanel {

   virtual void paint( ngrs::Graphics& g ) {
      ngrs::NFntString str;
      str.setText("Hallo");
      str.setFont( ngrs::NFont("Suse Sans",8,ngrs::nMedium | ngrs::nStraight | ngrs::nAntiAlias));
      str.append("Hier");
      g.drawText(10,10,str.substr(5,1));
   }

};


class NTestWindow : public ngrs::NWindow
{
public:
    NTestWindow();

    ~NTestWindow();

private:

    int counter; // this is for timer button test;

    ngrs::NProgressBar* pBar;
    ngrs::NSlider* slider;
    ngrs::NWindow* scrollWin;
    ngrs::NFileListBox* fileBox;
    ngrs::NFileDialog* fDialog;
    ngrs::NLabel* info;
    ngrs::NCustomItem* itemD;

    void testListBox();
    void testComboBox();
    void toolBarTest();
    void testSpinButton();
    void testBorderLayout();
    void testFlowLayout();
    void testFrameBorder();
	void testGroupBox();
    void testLine();
    void testMenu();
    void testEdit();
    void testSpinEdit();
    void testSegDisplay();
    void testTable();
    void testMemo();
    void testMsgBox();
    void testTimerButton();
    void testScrollBar();
	void testObjectInspector();
	void testCustomTreeView();

    void onOpen(ngrs::NButtonEvent* ev);
    void onSliderPosChanged(double v);
    void onQuit(ngrs::NObject* sender);
    void onBtnClick(ngrs::NButtonEvent* ev);

    void onColorSelected(const ngrs::NColor & color);
    void onDelete(ngrs::NButtonEvent* ev);

    void onSelection();
    void onScrollPosChange( ngrs::NScrollBar* bar );



    virtual void onMousePress( int x, int y, int button);
};

#endif
