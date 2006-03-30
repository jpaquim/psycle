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
#ifndef MASTERDLG_H
#define MASTERDLG_H

#include <nwindow.h>
#include <nslider.h>



class Machine;

/**
@author Stefan
*/

class MasterDlg : public NWindow
{
     class Led : public NPanel {
     public :

        Led();

        void setNumber(int number);
        int number();

        virtual void paint(NGraphics* g);

     private:

        int number_;

        static NBitmap numberBitmap;
        static int c;

   };



   class Slider : public NSlider {
     public:

       Slider() : NSlider(), index_(-1), led_(0) {}

       void setIndex(int index) { index_ = index; }
       int index() { return index_; }

       void setLed(Led* led) { led_ = led; }
       Led* led() { return led_; }

     private: 

        int index_;
        Led* led_;
   };



public:
    MasterDlg(Machine* master);

    ~MasterDlg();

    virtual void setVisible(bool on);
    virtual int onClose();

private:

   std::vector<Slider*> sliders;

   Machine* pMaster;

   void init();
   void onSliderPosChanged(NSlider* sender, double pos);
};

#endif
