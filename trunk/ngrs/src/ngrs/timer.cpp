/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by  Stefan Nattkemper   *
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
#include "timer.h"
#include <algorithm>

using namespace std;

namespace ngrs {

  vector<Timer*> Timer::timerList;

  Timer::Timer()
  {
    ms_=1000;
    enabled_ = false;
    count_=0;
  }


  Timer::~Timer()
  {
    vector<Timer*>::iterator it = find(timerList.begin(),timerList.end(),this);
    if (it!=timerList.end()) timerList.erase(it);
  }

  void Timer::enableTimer( )
  {
    timerList.push_back(this);
    enabled_ = true;
  }

  void Timer::disableTimer( )
  {
    vector<Timer*>::iterator it = find(timerList.begin(),timerList.end(),this);
    if (it!=timerList.end()) timerList.erase(it);
    enabled_ = false;
  }

  void Timer::setIntervalTime( int ms )
  {
    ms_=ms;
    count_=ms;
  }

  int Timer::intervalTime( ) const
  {
    return ms_;
  }


  void Timer::selectTimeOut( int us )
  {
    for( vector<Timer*>::iterator itr = timerList.begin(); itr < timerList.end(); itr++) {
      Timer* ti = *itr;
      if (ti->count()==0) { 
        ti->setCount(ti->intervalTime()); 
        ti->timerEvent.emit();
        ti->onTimerEvent();
      } else ti->setCount(ti->count()-1);
    }
  }

  void Timer::setCount( int c )
  {
    count_ = c;
  }

  int Timer::count( )
  {
    return count_;
  }

  void Timer::onTimerEvent( )
  {
  }

  bool Timer::enabled( ) const
  {
    return enabled_;
  }

}
