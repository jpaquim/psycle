/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by Stefan Nattkemper   *
 *   Germany   *
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
#ifndef NTHREAD_H
#define NTHREAD_H

#include <vector>

#ifdef __unix__
#include <pthread.h>
#else

#endif

namespace ngrs {

  class NThread {
  public:

    NThread();

    virtual ~NThread();

    // this ask's the system to start the thread
    void start();   

    // this sets terminated to true and kills the OS thread
    void terminate();

    // use this method in execute, to end your thread while loop
    bool terminated() const;


  protected:

    // override here your special implementation 
    // do not try to start the thread here
    // start the thread with resume.
    virtual void execute();

  private:

    bool terminated_;           

    static void* callBack( void* ptr ); 

    // pthread posix

#ifdef __unix__
    int iret1;
    pthread_t threadid;
#else

#endif

    static std::vector<NThread*> threadList;

    bool createOSThread();
    void killOSThread();

    void addThreadToList( NThread* thread );
    void removeThreadFromList( NThread* thread );
  };

}

#endif
