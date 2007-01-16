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
#include "masterdlg.h"
#include "global.h"
#include "defaultbitmaps.h"
#include "skinreader.h"
#include "machine.h"
#include "dsp.h"
#include <algorithm>

namespace psycle { 
  namespace host {

    MasterDlg::MasterDlg(Machine* master)
      : ngrs::Window()
    {
      pMaster = master;

      const ngrs::Bitmap & bg = SkinReader::Instance()->bitmaps().masterbk();

      pane()->skin_.setBitmap( bg , 1);

      setMinimumHeight( bg.height() );
      setMinimumWidth( bg. width() );
      setPosition(0,0, bg.width(),bg.height());

      init();
    }


    MasterDlg::~MasterDlg()
    {
    }

    void MasterDlg::init( )
    {
      for (int i = 0; i < 13 ; i++) {
        Slider* slider = new Slider();
        slider->setTrackLine(false);
        slider->setRange(0,208);
        slider->change.connect(this,&MasterDlg::onSliderPosChanged);
        slider->setIndex(i);
        pane()->add(slider);

        sliders.push_back(slider);

        Led* led = new Led();
        pane()->add(led);

        slider->setLed(led);

        if (i==0) {
          // Master of Desaster
          slider->setPosition(30,40,20,100);
          led->setPosition(30,140,20,10);
        } else {
          slider->setPosition(70+i*20,40,15,100);
          led->setPosition(70+i*20,140,20,10);
        }
      }
    }

    int MasterDlg::onClose( )
    {
      setVisible(false);
      return ngrs::nHideWindow;
    }

    void MasterDlg::setVisible( bool on )
    {
      int index = 0;
      for (std::vector<Slider*>::iterator it = sliders.begin(); it < sliders.end(); it++) {
        Slider* sl = *it;

        if ( index == 0 ) {
          float db = dsp::dB(pMaster->_outDry/256.0f);
          sl->setPos(208-(int)((db+40.0f)*4.0f));
          sl->led()->setNumber( static_cast<int>( sl->pos() ) );
          sl->led()->repaint();
          sl->repaint();
        } else {
          if (pMaster->_inputCon[index-1]) {
            float val;
            pMaster->GetWireVolume(index-1,val);
            sl->setPos(208-(int)((dsp::dB(val)+40.0f)*4.0f));
            sl->led()->setNumber( static_cast<int>( sl->pos() ) );
            sl->led()->repaint();
            sl->repaint();
          } else {
            sl->setPos(208);
            sl->led()->setNumber(-99);
            sl->led()->repaint();
            sl->repaint();
          }
        }
        index++;
      }
      ngrs::Window::setVisible(on);
    }

    void MasterDlg::onSliderPosChanged( ngrs::Slider * sender  )
    {
      std::vector<Slider*>::iterator it = find(sliders.begin(),sliders.end(),sender);
      if (it != sliders.end()) {
        Slider* slider = *it;
        if (slider->led()) {
          if (slider->index() > 0) {
            float db = ((208 - sender->pos() )/4.0f)-40.0f;
            pMaster->SetWireVolume(slider->index()-1,dsp::dB2Amp(db));
            slider->led()->setNumber( static_cast<int>( db ) );
            slider->led()->repaint();
          } else {
            // slidermaster

            float db = ((208- sender->pos() )/4.0f)-40.0f;
            pMaster->_outDry = int(dsp::dB2Amp(db)*256.0f);
            slider->led()->setNumber( static_cast<int>( db ) );
            slider->led()->repaint();
          }
        }
      }
    }

    void MasterDlg::Led::setNumber( int number )
    {
      number_ = number;
    }

    int MasterDlg::Led::number( )
    {
      return number_;
    }

    void MasterDlg::Led::paint( ngrs::Graphics& g )
    {
      /// todo replace with bitmap numbers and add in number.xpm a minus
      g.drawText(0,clientHeight(),stringify(number_));
    }

    MasterDlg::Led::Led( ) : number_(0)
    {
      ngrs::NFont font("Suse sans",6,ngrs::nStraight | ngrs::nMedium | ngrs::nAntiAlias);
      font.setTextColor( ngrs::Color( 255, 255, 255) );
      setFont(font);
    }

  }
}
