/***************************************************************************
 *   Copyright (C) 2006 by Stefan   *
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
#ifndef MACHINEGUI_H
#define MACHINEGUI_H

#include <npanel.h>

class NLine;
class Machine;
class FrameMachine;
class MasterDlg;

/**
@author Stefan
*/

class MachineGUI : public NPanel
{
  class LineAttachment {
    public:
       LineAttachment(NLine* l, int p) : line(l),point(p) {}

       NLine* line;
       int point;
  };

public:
    MachineGUI(Machine* mac);

    ~MachineGUI();

    static NBitmap bitmap;

    Machine* pMac();
    void attachLine(NLine* line, int point);
    void detachLine(NLine* line);

    signal1<MachineGUI*> newConnection;

    virtual void onMouseDoublePress(int x, int y, int button);
    virtual void onMousePress(int x, int y, int button);
    virtual void onMoveStart(const NMoveEvent & moveEvent);
    virtual void onMove(const NMoveEvent & moveEvent);
    virtual void resize();

private:


    static int c;
    NBorder* myBorder_;

    NSize oldDrag;
    NSize linesClipBox();

    Machine* mac_;
    std::vector<LineAttachment> attachedLines;
    NLine* line;
};


class MasterGUI : public MachineGUI
{
public:
    MasterGUI(Machine* mac);

    ~MasterGUI();


private:

   MasterDlg* masterDlg;
   NRect bgCoords;

   virtual void onMouseDoublePress(int x, int y, int button);
   virtual void paint(NGraphics* g);

   void setSkin();
};


class GeneratorGUI : public MachineGUI
{
public:
    GeneratorGUI(Machine* mac);

    ~GeneratorGUI();

    FrameMachine* frameMachine;

private:


   NRect bgCoords;
   NPoint dNameCoords;

   virtual void onMouseDoublePress(int x, int y, int button);
   virtual void paint(NGraphics* g);

   void setSkin();
};


class EffektGUI : public MachineGUI
{
public:
    EffektGUI(Machine* mac);

    ~EffektGUI();

    FrameMachine* frameMachine;

private:

   NRect bgCoords;
   NPoint dNameCoords;

   virtual void onMouseDoublePress(int x, int y, int button);
   virtual void paint(NGraphics* g);

   void setSkin();
};



#endif
