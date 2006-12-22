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
#ifndef NFLIPBAR_H
#define NFLIPBAR_H

#include "npanel.h"

/**
@author Stefan Nattkemper
*/

class NImage;
class NBitmap;

class NFlipBar : public NPanel
{

  class NFlipper : public NPanel {
    public:

       friend class NFlipBar;

       NFlipper( NFlipBar* flipBar );

       ~NFlipper( );

       void setExpanded( bool on );
	   bool expanded( ) const;

       virtual void onMousePress( int x, int y, int button );

       virtual int preferredWidth() const;
       virtual int preferredHeight() const;

    private:

       bool expanded_;

       NImage* expandImg_;

       NBitmap expandBmp;
       NBitmap expandedBmp;

       NFlipBar* flipBar_;


  };

public:
    NFlipBar();

    ~NFlipBar();

    void setExpanded(bool on);
    bool expanded() const;

    signal1<NFlipBar*> change;

    NPanel* header();

    int flipperWidth() const;
	int flipperHeight() const;

private:

    NFlipper* flipper_;
    NPanel* header_;

    void onFlipClick();

};

#endif
