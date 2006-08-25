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
#ifndef CUSTOMPATTERNVIEW_H
#define CUSTOMPATTERNVIEW_H

#include <ngrs/npanel.h>

/**
@author Stefan Nattkemper
*/

namespace psycle {
	namespace host	{	

		class CustomPatternView : public NPanel
		{
			public:
					CustomPatternView();

					~CustomPatternView();

					virtual int colWidth() const;
					virtual int rowHeight() const;
					virtual int lineNumber() const;
					virtual int trackNumber() const;

      		void setDx(int dx);
					int dx() const;
					void setDy(int dy);
					int dy() const;

					virtual void paint(NGraphics* g);

					virtual void onMousePress(int x, int y, int button);
					virtual void onMousePressed(int x, int y, int button);
					virtual void onMouseOver	(int x, int y);

					virtual void onKeyPress(const NKeyEvent & event);
					virtual void onKeyRelease(const NKeyEvent & event);

			protected:

					virtual void customPaint( NGraphics* g, int startLine, int endLine, int startTrack, int endTrack );

			private:

					int dx_, dy_;

					void init();

					NRect repaintTrackArea(int startLine,int endLine,int startTrack, int endTrack) const;
      		NPoint linesFromRepaint(const NRegion & repaintArea) const;
      		NPoint tracksFromRepaint(const NRegion & repaintArea) const;

		};
	}
}

#endif
