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
#ifndef NLABEL_H
#define NLABEL_H

#include <nvisualcomponent.h>
#include <nrectshape.h>
#include <string>
#include <nfontmetrics.h>

/**
@author Stefan
*/
class NLabel : public NVisualComponent
{
public:
    NLabel();
    NLabel(std::string text);

    ~NLabel();

   virtual void paint(NGraphics* g);
   void setText(const std::string & text);
   const std::string & text() const;

   void setMnemonic(char c);
   char mnemonic();

   virtual int preferredWidth() const;
   virtual int preferredHeight() const;

   void setVAlign(int align);
   void setHAlign(int align);

   void setTextOrientation(int orientation);

   int vAlign();
   int hAlign();

   void resize();

   void setWordbreak(bool on);
   bool wordBreak() const;

private:

   int valign_, halign_;
   int orientation_;

   bool wbreak_;

   NBitmap rotateBmp;
   std::string text_;
   NFontMetrics metrics;

   std::vector<int> breakPoints;

   void init();
   char mnemonic_;

   void computeBreakPoints();
   int findWidthMax(long width, const std::string & data, bool wbreak);
};

#endif
