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
#ifndef NTIMER_H
#define NTIMER_H

#include "nobject.h"
#include <vector>

/**
@author Stefan Nattkemper
*/

class NTimer : public NObject
{
public:
    NTimer();

    ~NTimer();

    sigslot::signal0<> timerEvent;

    void enableTimer();
    void disableTimer();
    void setIntervalTime(int ms);
    int intervalTime();

    static std::vector<NTimer*> timerList;
    static void selectTimeOut(int us);
    int count();
    void setCount(int c);

    virtual void onTimerEvent();

private:

    int ms_;
    bool enabled;
    int count_;

};

#endif
