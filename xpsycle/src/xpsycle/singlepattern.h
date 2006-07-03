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
#ifndef SINGLEPATTERN_H
#define SINGLEPATTERN_H

#include "patternline.h"
#include <list>

/**
@author Stefan Nattkemper
*/

class SinglePattern : public std::list<PatternLine> {
public:
    SinglePattern();

    ~SinglePattern();

    std::list<PatternLine>::iterator startItr(float position);

    void setData(float position, int track, const PatternEvent & data);
    const PatternEvent & dataAt(float position, int track);

    void setBeatZoom(int zoom);
    int beatZoom() const;

    void setBeats(int beats);
    int beats() const;

private:

    int beats_;
    int beatZoom_;

    PatternLine* lastLine;
    PatternEvent zeroTrack;
};

#endif
