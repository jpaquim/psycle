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
#ifndef DEFAULTBITMAPS_H
#define DEFAULTBITMAPS_H

#include <ngrs/nbitmap.h>
#include <ngrs/npixmap.h>

/**
@author Stefan Nattkemper
*/

namespace psycle {
  namespace host {

    class DefaultBitmaps{
    public:
      DefaultBitmaps();
      DefaultBitmaps( class Configuration* pCfg );

      ~DefaultBitmaps();

      ngrs::NBitmap & logoSmall();
      ngrs::NBitmap & tweakHeader();
      ngrs::NBitmap & addTrack();
      ngrs::NBitmap & deleteTrack();
      ngrs::NBitmap & alert();
      ngrs::NBitmap & autoStop();
      ngrs::NBitmap & help();
      ngrs::NBitmap & less();
      ngrs::NBitmap & lessless();
      ngrs::NBitmap & littleleft();
      ngrs::NBitmap & littleright();
      ngrs::NBitmap & machines();
      ngrs::NPixmap & machine_skin();
      ngrs::NBitmap & masterbk();
      ngrs::NBitmap & minus1();
      ngrs::NBitmap & minus();
      ngrs::NBitmap & moremore();
      ngrs::NBitmap & more();
      ngrs::NBitmap & newmachine();
      ngrs::NBitmap & newfile();
      ngrs::NBitmap & numbers();
      ngrs::NBitmap & openeditor();
      ngrs::NBitmap & open();
      ngrs::NBitmap & pattern_header_skin();
      ngrs::NBitmap & pattern_new();
      ngrs::NBitmap & patterns();
      ngrs::NBitmap & delPattern();
      ngrs::NBitmap & playselpattern();
      ngrs::NBitmap & playstart();
      ngrs::NBitmap & play();
      ngrs::NBitmap & playstart_flat();
      ngrs::NBitmap & play_flat();
      ngrs::NBitmap & ffwd();
      ngrs::NBitmap & rwnd();
      ngrs::NBitmap & release();
      ngrs::NBitmap & plus1();
      ngrs::NBitmap & plus();
      ngrs::NBitmap & p();
      ngrs::NBitmap & recordnotes();
      ngrs::NBitmap & recordwav();
      ngrs::NBitmap & redo();
      ngrs::NBitmap & save_audio();
      ngrs::NBitmap & save();
      ngrs::NBitmap & sequencer();
      ngrs::NBitmap & splash();
      ngrs::NBitmap & stop();
      ngrs::NBitmap & stop_flat();
      ngrs::NBitmap & tbmainknob();
      ngrs::NBitmap & undo();
      ngrs::NBitmap & wood();
      ngrs::NBitmap & screw();
      ngrs::NBitmap & new_category();
      ngrs::NBitmap & moveDownTrack();
      ngrs::NBitmap & moveUpTrack();
      ngrs::NBitmap & patNav();
      ngrs::NBitmap & logoLeft();
      ngrs::NBitmap & logoMid();
      ngrs::NBitmap & logoRight();

    private:

      ngrs::NBitmap addTrack_;
      ngrs::NBitmap new_category_;
      ngrs::NBitmap alert_;
      ngrs::NBitmap screw_;
      ngrs::NBitmap wood_;
      ngrs::NBitmap autoStop_;
      ngrs::NBitmap help_;
      ngrs::NBitmap less_;
      ngrs::NBitmap lessless_;
      ngrs::NBitmap littleleft_;
      ngrs::NBitmap littleright_;
      ngrs::NBitmap machines_;
      ngrs::NPixmap machine_skin_;
      ngrs::NBitmap masterbk_;
      ngrs::NBitmap minus1_;
      ngrs::NBitmap minus_;
      ngrs::NBitmap moremore_;
      ngrs::NBitmap more_;
      ngrs::NBitmap newmachine_;
      ngrs::NBitmap newfile_;
      ngrs::NBitmap numbers_;
      ngrs::NBitmap openeditor_;
      ngrs::NBitmap open_;
      ngrs::NBitmap pattern_header_skin_;
      ngrs::NBitmap patterns_;
      ngrs::NBitmap pattern_new_;
      ngrs::NBitmap playselpattern_;
      ngrs::NBitmap playstart_;
      ngrs::NBitmap play_;
      ngrs::NBitmap playstart_flat_;
      ngrs::NBitmap play_flat_;
      ngrs::NBitmap ffwd_;
      ngrs::NBitmap rwnd_;
      ngrs::NBitmap release_;
      ngrs::NBitmap plus1_;
      ngrs::NBitmap plus_;
      ngrs::NBitmap p_;
      ngrs::NBitmap recordnotes_;
      ngrs::NBitmap recordwav_;
      ngrs::NBitmap redo_;
      ngrs::NBitmap save_audio_;
      ngrs::NBitmap save_;
      ngrs::NBitmap sequencer_;
      ngrs::NBitmap splash_;
      ngrs::NBitmap stop_;
      ngrs::NBitmap stop_flat_;
      ngrs::NBitmap tbmainknob_;
      ngrs::NBitmap undo_;
      ngrs::NBitmap delPattern_;
      ngrs::NBitmap deleteTrack_;
      ngrs::NBitmap moveDownTrack_;
      ngrs::NBitmap moveUpTrack_;
      ngrs::NBitmap tweakHeader_;
      ngrs::NBitmap patNav_;
      ngrs::NBitmap logoSmall_;
      ngrs::NBitmap logoLeft_;
      ngrs::NBitmap logoMid_;
      ngrs::NBitmap logoRight_;

    };

  }
}

#endif
