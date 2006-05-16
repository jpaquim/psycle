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
#include "ngrs/ntimer.h"

using namespace std;

vector<NTimer*> NTimer::timerList;

NTimer::NTimer()
{
 ms_=1000;
 enabled=false;
 count_=0;
}


NTimer::~NTimer()
{
 vector<NTimer*>::iterator it = find(timerList.begin(),timerList.end(),this);
 if (it!=timerList.end()) timerList.erase(it);
}

void NTimer::enableTimer( )
{
 timerList.push_back(this);
 enabled=true;
}

void NTimer::disableTimer( )
{
 vector<NTimer*>::iterator it = find(timerList.begin(),timerList.end(),this);
 if (it!=timerList.end()) timerList.erase(it);
 enabled=false;
}

void NTimer::setIntervalTime( int ms )
{
 ms_=ms;
 count_=ms;
}

int NTimer::intervalTime( )
{
 return ms_;
}


void NTimer::selectTimeOut( int us )
{
 for( vector<NTimer*>::iterator itr = timerList.begin(); itr < timerList.end(); itr++) {
    NTimer* ti = *itr;
    if (ti->count()==0) { 
        ti->setCount(ti->intervalTime()); 
        ti->timerEvent.emit();
        ti->onTimerEvent();
    } else ti->setCount(ti->count()-1);
  }
}

void NTimer::setCount( int c )
{
  count_ = c;
}

int NTimer::count( )
{
  return count_;
}

void NTimer::onTimerEvent( )
{
}
