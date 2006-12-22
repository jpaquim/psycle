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
#ifndef NFLIPBOX_H
#define NFLIPBOX_H

#include "npanel.h"

class NFlipBar;

// style flags

const int nFlipBoxNone    = 0;
const int nFlipBoxTwister = 1;
const int nFlipBoxLine    = 2;

/**
@author Stefan Nattkemper
*/
class NFlipBox : public NPanel
{
public:
    NFlipBox();

    ~NFlipBox();

    NPanel* pane();
    NPanel* header();
	
    void setExpanded( bool on );
    bool expanded() const;

	void setStyle( int style );
	int style() const;

    int flipperWidth() const;

    virtual int preferredHeight() const;
    virtual void paint( NGraphics* g );

private:

   int style_;
   NFlipBar* flipBar_;
   NPanel*   pane_;

   void onFlipChange( NFlipBar* sender );

};

#endif
