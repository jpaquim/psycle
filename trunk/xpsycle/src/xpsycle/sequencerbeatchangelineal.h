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
#ifndef SEQUENCERBEATCHANGELINEAL_H
#define SEQUENCERBEATCHANGELINEAL_H

#include <ngrs/npanel.h>

/**
@author Stefan Nattkemper
*/

namespace ngrs{
  class NEdit;
}

namespace psycle {
  namespace host {

    class SequencerGUI;
    class PatternSequence;
    class GlobalEvent;

    class SequencerBeatChangeLineal : public ngrs::NPanel
    {

      class BeatChangeTriangle : public ngrs::NPanel {
      public:
        BeatChangeTriangle();
        BeatChangeTriangle(SequencerGUI * gui);
        ~BeatChangeTriangle();

        void setBpmChangeEvent(GlobalEvent* event);
        GlobalEvent* bpmChangeEvent();

        virtual void onMove(const ngrs::NMoveEvent & moveEvent);
        virtual void paint( ngrs::Graphics& g);
        virtual int preferredHeight() const;
        virtual void resize();

      private:

        SequencerGUI* sView;
        ngrs::NEdit* bpmEdt_;

        int tWidth;
        int tHeight;

        GlobalEvent* bpmChangeEvent_;

        void onKeyPress(const ngrs::NKeyEvent& event);

      };

    public:
      SequencerBeatChangeLineal();
      SequencerBeatChangeLineal( SequencerGUI* seqGui );

      ~SequencerBeatChangeLineal();

      virtual void paint( ngrs::Graphics& g );

      virtual int preferredHeight() const;

      virtual void onMouseDoublePress( int x, int y, int button );

      virtual void resize();

    private:

      SequencerGUI* sView;
      std::list<BeatChangeTriangle*> beatChanges;

    };


  }
}
#endif
