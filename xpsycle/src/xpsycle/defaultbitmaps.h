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
#ifndef DEFAULTBITMAPS_H
#define DEFAULTBITMAPS_H

#include <ngrs/bitmap.h>
#include <ngrs/pixmap.h>

/**
@author  Stefan Nattkemper
*/

namespace psycle {
	namespace host {

		class DefaultBitmaps{
		public:
			DefaultBitmaps();
			DefaultBitmaps( class Configuration* pCfg );

			~DefaultBitmaps();

			ngrs::Bitmap & logoSmall();
			ngrs::Bitmap & tweakHeader();
			ngrs::Bitmap & addTrack();
			ngrs::Bitmap & deleteTrack();
			ngrs::Bitmap & alert();
			ngrs::Bitmap & autoStop();
			ngrs::Bitmap & help();
			ngrs::Bitmap & less();
			ngrs::Bitmap & lessless();
			ngrs::Bitmap & littleleft();
			ngrs::Bitmap & littleright();
			ngrs::Bitmap & machines();
			ngrs::Pixmap & machine_skin();
			ngrs::Bitmap & masterbk();
			ngrs::Bitmap & minus1();
			ngrs::Bitmap & minus();
			ngrs::Bitmap & moremore();
			ngrs::Bitmap & more();
			ngrs::Bitmap & newmachine();
			ngrs::Bitmap & newfile();
			ngrs::Bitmap & numbers();
			ngrs::Bitmap & openeditor();
			ngrs::Bitmap & open();
			ngrs::Bitmap & pattern_header_skin();
			ngrs::Bitmap & pattern_new();
			ngrs::Bitmap & patterns();
			ngrs::Bitmap & delPattern();
			ngrs::Bitmap & playselpattern();
			ngrs::Bitmap & playstart();
			ngrs::Bitmap & play();
			ngrs::Bitmap & playstart_flat();
			ngrs::Bitmap & play_flat();
			ngrs::Bitmap & ffwd();
			ngrs::Bitmap & rwnd();
			ngrs::Bitmap & release();
			ngrs::Bitmap & plus1();
			ngrs::Bitmap & plus();
			ngrs::Bitmap & p();
			ngrs::Bitmap & recordnotes();
			ngrs::Bitmap & recordwav();
			ngrs::Bitmap & redo();
			ngrs::Bitmap & save_audio();
			ngrs::Bitmap & save();
			ngrs::Bitmap & sequencer();
			ngrs::Bitmap & splash();
			ngrs::Bitmap & stop();
			ngrs::Bitmap & stop_flat();
			ngrs::Bitmap & tbmainknob();
			ngrs::Bitmap & undo();
			ngrs::Bitmap & wood();
			ngrs::Bitmap & screw();
			ngrs::Bitmap & new_category();
			ngrs::Bitmap & moveDownTrack();
			ngrs::Bitmap & moveUpTrack();
			ngrs::Bitmap & patNav();
			ngrs::Bitmap & logoLeft();
			ngrs::Bitmap & logoMid();
			ngrs::Bitmap & logoRight();
			ngrs::Bitmap & patternbox_clonepattern();

		private:

			ngrs::Bitmap addTrack_;
			ngrs::Bitmap new_category_;
			ngrs::Bitmap alert_;
			ngrs::Bitmap screw_;
			ngrs::Bitmap wood_;
			ngrs::Bitmap autoStop_;
			ngrs::Bitmap help_;
			ngrs::Bitmap less_;
			ngrs::Bitmap lessless_;
			ngrs::Bitmap littleleft_;
			ngrs::Bitmap littleright_;
			ngrs::Bitmap machines_;
			ngrs::Pixmap machine_skin_;
			ngrs::Bitmap masterbk_;
			ngrs::Bitmap minus1_;
			ngrs::Bitmap minus_;
			ngrs::Bitmap moremore_;
			ngrs::Bitmap more_;
			ngrs::Bitmap newmachine_;
			ngrs::Bitmap newfile_;
			ngrs::Bitmap numbers_;
			ngrs::Bitmap openeditor_;
			ngrs::Bitmap open_;
			ngrs::Bitmap pattern_header_skin_;
			ngrs::Bitmap patterns_;
			ngrs::Bitmap pattern_new_;
			ngrs::Bitmap playselpattern_;
			ngrs::Bitmap playstart_;
			ngrs::Bitmap play_;
			ngrs::Bitmap playstart_flat_;
			ngrs::Bitmap play_flat_;
			ngrs::Bitmap ffwd_;
			ngrs::Bitmap rwnd_;
			ngrs::Bitmap release_;
			ngrs::Bitmap plus1_;
			ngrs::Bitmap plus_;
			ngrs::Bitmap p_;
			ngrs::Bitmap recordnotes_;
			ngrs::Bitmap recordwav_;
			ngrs::Bitmap redo_;
			ngrs::Bitmap save_audio_;
			ngrs::Bitmap save_;
			ngrs::Bitmap sequencer_;
			ngrs::Bitmap splash_;
			ngrs::Bitmap stop_;
			ngrs::Bitmap stop_flat_;
			ngrs::Bitmap tbmainknob_;
			ngrs::Bitmap undo_;
			ngrs::Bitmap delPattern_;
			ngrs::Bitmap deleteTrack_;
			ngrs::Bitmap moveDownTrack_;
			ngrs::Bitmap moveUpTrack_;
			ngrs::Bitmap tweakHeader_;
			ngrs::Bitmap patNav_;
			ngrs::Bitmap logoSmall_;
			ngrs::Bitmap logoLeft_;
			ngrs::Bitmap logoMid_;
			ngrs::Bitmap logoRight_;
			ngrs::Bitmap patternbox_clonepattern_;

		};

	}
}

#endif
