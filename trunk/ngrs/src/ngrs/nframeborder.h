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
#ifndef NFRAMEBORDER_H
#define NFRAMEBORDER_H

#include <nborder.h>
#include <ncolor.h>

/**
@author Stefan
*/
class NFrameBorder : public NBorder
{
public:
    NFrameBorder();

    ~NFrameBorder();

   void setLineCount(int count, int dWidth = 3, int dHeight = 3);
   void setOval(bool on = true, int arcWidth = 5, int arcHeight = 5);
   bool oval() const;

   int lineCount() const;

   int arcWidth() const;
   int arcHeight() const;

   int dWidth() const;
   int dHeight() const;

   virtual void paint(NGraphics* g, const NShape & geometry);

   virtual NFrameBorder* clone()  const;   // Uses the copy constructor

private:

   int lcount_;  // number of frame lines
   int dx_;      // dx space between lines
   int dy_;      // dy space between lines

   int arcWidth_;
   int arcHeight_;

   bool oval_;
};

#endif
