/***************************************************************************
 *   Copyright (C) 2005 by  Stefan   *
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

#include <ngrs/window.h>
#include <ngrs/menubar.h>
#include <ngrs/menuitem.h>
#include <ngrs/scrollbox.h>
#include <ngrs/edit.h>
#include <ngrs/file.h>
#include <ngrs/progressbar.h>
#include <ngrs/slider.h>
#include <ngrs/image.h>
#include <ngrs/slider.h>
#include <ngrs/notebook.h>
#include <ngrs/tabbook.h>
#include <ngrs/checkbox.h>
#include <ngrs/filelistbox.h>
#include <ngrs/filedialog.h>

/**
@author  Stefan
*/

class Tpnl : public ngrs::Panel {

   virtual void paint( ngrs::Graphics& g ) {
      ngrs::FntString str;
      str.setText("Hallo");
      str.setFont( ngrs::Font("Suse Sans",8,ngrs::nMedium | ngrs::nStraight | ngrs::nAntiAlias));
      str.append("Hier");
      g.drawText(10,10,str.substr(5,1));
   }

};


class NTestWindow : public ngrs::Window
{
public:
    NTestWindow();

    ~NTestWindow();

private:

    int counter; // this is for timer button test;

    ngrs::ProgressBar* pBar;
    ngrs::Slider* slider;
    ngrs::Window* scrollWin;
    ngrs::FileListBox* fileBox;
    ngrs::FileDialog* fDialog;
    ngrs::Label* info;
    ngrs::CustomItem* itemD;

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

    void onOpen(ngrs::ButtonEvent* ev);
    void onSliderPosChanged(double v);
    void onQuit(ngrs::Object* sender);
    void onBtnClick(ngrs::ButtonEvent* ev);

    void onColorSelected(const ngrs::Color & color);
    void onDelete(ngrs::ButtonEvent* ev);

    void onSelection();
    void onScrollPosChange( ngrs::ScrollBar* bar );



    virtual void onMousePress( int x, int y, int button);
};

#endif
