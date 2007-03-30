/***************************************************************************
*   Copyright (C) 2006 by  Stefan Nattkemper   *
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

#include <ngrs/panel.h>

/**
@author  Stefan Nattkemper
*/

namespace ngrs{
	class Edit;
}

namespace psy {
	namespace core {
		class PatternSequence;
		class GlobalEvent;
	}
	namespace host {

		class SequencerGUI;

		class SequencerBeatChangeLineal : public ngrs::Panel
		{

			class BeatChangeTriangle : public ngrs::Panel {
			public:
				BeatChangeTriangle();
				BeatChangeTriangle( SequencerGUI * gui );
				~BeatChangeTriangle();

				void setBpmChangeEvent( psy::core::GlobalEvent* event );
				psy::core::GlobalEvent* bpmChangeEvent();

				virtual void onMove(const ngrs::MoveEvent & moveEvent);
				virtual void paint( ngrs::Graphics& g);
				virtual int preferredHeight() const;
				virtual void resize();

			private:

				SequencerGUI* sView;
				ngrs::Edit* bpmEdt_;

				int tWidth;
				int tHeight;

				psy::core::GlobalEvent* bpmChangeEvent_;

				void onKeyPress( const ngrs::KeyEvent& event );

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
