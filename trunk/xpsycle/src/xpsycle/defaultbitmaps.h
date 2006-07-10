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
    DefaultBitmaps( class Configuration* pCfg);

    ~DefaultBitmaps();

    NBitmap & alert();
    NBitmap & autoStop();
    NBitmap & help();
    NBitmap & less();
    NBitmap & lessless();
    NBitmap & littleleft();
    NBitmap & littleright();
    NBitmap & machines();
    NPixmap & machine_skin();
    NBitmap & masterbk();
    NBitmap & minus1();
    NBitmap & minus();
    NBitmap & moremore();
    NBitmap & more();
    NBitmap & newmachine();
    NBitmap & newfile();
    NBitmap & numbers();
    NBitmap & openeditor();
    NBitmap & open();
    NBitmap & pattern_header_skin();
    NBitmap & patterns();
    NBitmap & playselpattern();
    NBitmap & playstart();
    NBitmap & play();
    NBitmap & playstart_flat();
    NBitmap & play_flat();
    NBitmap & ffwd();
    NBitmap & rwnd();
    NBitmap & release();
    NBitmap & plus1();
    NBitmap & plus();
    NBitmap & p();
    NBitmap & recordnotes();
    NBitmap & recordwav();
    NBitmap & redo();
    NBitmap & save_audio();
    NBitmap & save();
    NBitmap & sequencer();
    NBitmap & splash();
    NBitmap & stop();
    NBitmap & stop_flat();
    NBitmap & tbmainknob();
    NBitmap & undo();
    NBitmap & wood();
    NBitmap & screw();

private:

    NBitmap alert_;
    NBitmap screw_;
    NBitmap wood_;
    NBitmap autoStop_;
    NBitmap help_;
    NBitmap less_;
    NBitmap lessless_;
    NBitmap littleleft_;
    NBitmap littleright_;
    NBitmap machines_;
    NPixmap machine_skin_;
    NBitmap masterbk_;
    NBitmap minus1_;
    NBitmap minus_;
    NBitmap moremore_;
    NBitmap more_;
    NBitmap newmachine_;
    NBitmap newfile_;
    NBitmap numbers_;
    NBitmap openeditor_;
    NBitmap open_;
    NBitmap pattern_header_skin_;
    NBitmap patterns_;
    NBitmap playselpattern_;
    NBitmap playstart_;
    NBitmap play_;
    NBitmap playstart_flat_;
    NBitmap play_flat_;
    NBitmap ffwd_;
    NBitmap rwnd_;
    NBitmap release_;
    NBitmap plus1_;
    NBitmap plus_;
    NBitmap p_;
    NBitmap recordnotes_;
    NBitmap recordwav_;
    NBitmap redo_;
    NBitmap save_audio_;
    NBitmap save_;
    NBitmap sequencer_;
    NBitmap splash_;
    NBitmap stop_;
    NBitmap stop_flat_;
    NBitmap tbmainknob_;
    NBitmap undo_;

};

}
}

#endif
