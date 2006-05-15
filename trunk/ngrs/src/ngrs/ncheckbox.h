/***************************************************************************
 *   Copyright (C) 2005 by Stefan   *
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
#ifndef NCHECKBOX_H
#define NCHECKBOX_H

#include <npanel.h>
#include <nfontmetrics.h>

class NLabel;

/**
@author Stefan
*/
class NCheckBox : public NPanel
{
public:
    NCheckBox();
    NCheckBox(const std::string & text);

    ~NCheckBox();

   void setText(const std::string & text);
   const std::string & text() const;

   virtual void paint(NGraphics* g);

   virtual void onMousePress(int x, int y, int button);
   virtual void onMousePressed (int x, int y, int button);

   void setCheck(bool on);
   bool checked() const;

   virtual int preferredWidth() const;
   virtual int preferredHeight() const;

   virtual void resize();

   signal1<NButtonEvent*> clicked;

private:

   NLabel* label_;
   int dx,dy;
   std::string text_;
   bool checked_;
   NFontMetrics metrics;

   void init();
   void drawCheck(NGraphics* g);
};

#endif
