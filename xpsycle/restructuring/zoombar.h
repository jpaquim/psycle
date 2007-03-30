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
#ifndef ZOOMBAR_H
#define ZOOMBAR_H

#include <ngrs/panel.h>
#include <ngrs/bitmap.h>

/**
@author  Stefan Nattkemper
*/

namespace ngrs {
	class Button;
	class Slider;
	class Image;
}

class ZoomBar : public ngrs::Panel{
public:
		ZoomBar();

		~ZoomBar();

		sigslot::signal2<ZoomBar*,double> posChanged;

		void setOrientation(int orientation);
		int orientation() const;

		void setRange( double min, double max );
		void setPos( double pos );
		double pos() const;

		void customSliderPaint( ngrs::Slider* sl, ngrs::Graphics& g );

private:

		int orientation_;
		int increment_;

		ngrs::Button* decBtn;
		ngrs::Button* incBtn;
		ngrs::Slider* zoomSlider;

		ngrs::Bitmap zoomOutBpm;
		ngrs::Bitmap zoomInBpm;
		ngrs::Bitmap sliderBpm;

		void init();

		void onPosChanged( ngrs::Slider* slider );
		void onIncButton( ngrs::ButtonEvent* ev );
		void onDecButton( ngrs::ButtonEvent* ev );
};

#endif
