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

#include "skinreader.h"

#include <ngrs/npanel.h>

class NLine;
class NSlider;

namespace psycle { namespace host {

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

				MachineGUI* in;

  };

public:
    MachineGUI(Machine* mac);

    ~MachineGUI();

		signal1<MachineGUI*> deleteRequest;

    Machine* pMac();
    void attachLine(NLine* line, int point);
    void detachLine(NLine* line);

    signal1<MachineGUI*> newConnection;
    signal3<Machine*,int,int> moved;
    signal3<int,int,int> patternTweakSlide;
    signal1<MachineGUI*> selected;

    int ident();

    virtual void onMouseDoublePress(int x, int y, int button);
    virtual void onMousePress(int x, int y, int button);
    virtual void onMoveStart(const NMoveEvent & moveEvent);
    virtual void onMove(const NMoveEvent & moveEvent);
    virtual void onMoveEnd(const NMoveEvent & moveEvent);
    virtual void resize();

    virtual void paint(NGraphics* g);

    virtual void repaintVUMeter();

    void setSelected(bool on);

		std::vector<LineAttachment> attachedLines;

		void setCoordInfo( const MachineCoordInfo &  coords );
		const MachineCoordInfo & coords() const;

		virtual void updateSkin();

private:

    NBorder* myBorder_;

    NRegion oldDrag;
    NRegion linesRegion();

    Machine* mac_;
    NLine* line;

    bool selected_;

		MachineCoordInfo coords_;
};


class MasterGUI : public MachineGUI
{
public:
    MasterGUI(Machine* mac);

    ~MasterGUI();


    virtual void onMousePress(int x, int y, int button);
    virtual void onMouseDoublePress(int x, int y, int button);

    virtual void paint(NGraphics* g);

		virtual void updateSkin();

private:

    MasterDlg* masterDlg;
/*    NRect bgCoords;
    NRect muteCoords;
    NRect soloCoords;
    NRect dSoloCoords;
    NRect dMuteCoords;*/

    void setSkin();
};


class GeneratorGUI : public MachineGUI
{
public:

    class VUPanel : public NPanel {

      friend class GeneratorGUI;

      public:
          VUPanel(GeneratorGUI* pGui) {
            pGui_ = pGui;
          };

        virtual void paint(NGraphics* g);

      private:

				GeneratorGUI* pGui_;

    };


    GeneratorGUI(Machine* mac);

    ~GeneratorGUI();

    FrameMachine* frameMachine;

    virtual void onMousePress(int x, int y, int button);

    virtual void repaintVUMeter();

    virtual void onMouseDoublePress(int x, int y, int button);

    virtual void paint(NGraphics* g);

    virtual void onKeyPress( const NKeyEvent & event);

		virtual void updateSkin();

private:

    NSlider* panSlider_;
    VUPanel* vuPanel_;

    /*NRect bgCoords;
    NRect muteCoords;
    NRect soloCoords;
    NRect dSoloCoords;
    NRect dMuteCoords;
    NPoint dNameCoords;

    NRect sGeneratorVuPeak;
    NRect sGeneratorVu0;
    NRect dGeneratorVu;
    NRect sGenerator;

    NRect sGenPan;*/

    void customSliderPaint(NSlider* sl, NGraphics* g);

    void setSkin();
    void onPosChanged( NSlider* sender );
    void onTweakSlide( int machine, int command, int value );
};


class EffektGUI : public MachineGUI
{
public:

    class VUPanel : public NPanel {

      friend class EffectGUI;

      public:
          VUPanel(EffektGUI* pGui) {
            pGui_ = pGui;
          };

        virtual void paint(NGraphics* g);

      private:

        EffektGUI* pGui_;

    };


    EffektGUI(Machine* mac);

    ~EffektGUI();

    FrameMachine* frameMachine;

    virtual void onMousePress(int x, int y, int button);
    virtual void onMouseDoublePress(int x, int y, int button);

    virtual void paint(NGraphics* g);

    virtual void repaintVUMeter();

		virtual void onKeyPress( const NKeyEvent & event );

		virtual void updateSkin();

private:

    //NRect bgCoords;
    //NRect sEffectPan;
    //NPoint dNameCoords;
    NSlider* panSlider_;

    /*NRect muteCoords;
    NRect soloCoords;
    NRect dSoloCoords;
    NRect dMuteCoords;

    NRect sGeneratorVuPeak;
    NRect sGeneratorVu0;
    NRect dGeneratorVu;
    NRect sGenerator;*/


    VUPanel* vuPanel_;

    void customSliderPaint(NSlider* sl, NGraphics* g);

    void setSkin();
    void onPosChanged( NSlider* sender );
    void onTweakSlide( int machine, int command, int value );
};

}
}

#endif
