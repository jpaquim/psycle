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
#include "thread.h"
#include <algorithm>

namespace ngrs {

  std::vector<Thread*> Thread::threadList;

  Thread::Thread() : terminated_(1) {

  }                   

  Thread::~Thread() {

  }                    


  void Thread::execute() {
    // override this virtual method
    // called by the OS Thread
  }        

  void Thread::start() {
    if ( terminated_ )
      terminated_ = !createOSThread();    
  }     

  void Thread::terminate() {
    terminated_ = true;     
  }     

  bool Thread::terminated() const {
    return terminated_;    
  }     

  // static callBack

  void* Thread::callBack( void* ptr ) {
    std::vector<Thread*>::iterator itr = find( threadList.begin(), threadList.end(), static_cast<Thread*> (ptr) );
    if ( itr != threadList.end() ) {
      Thread* thread = *itr;
      thread->execute();     
      thread->killOSThread();
    }

    return 0;
  }      

  // creates an OS Thread

  bool Thread::createOSThread() {

    addThreadToList( this );

#ifdef __unix__
    if ( 0 == pthread_create( &threadid, NULL, (void*(*)(void*))Thread::callBack, (void*) this)  ) {
      // sth went wrong
      removeThreadFromList( this );   
      return false;
    }    
    else
      return true;
#else
    // todo win32 thread create
    // not yet implemented
    removeThreadFromList( this );
    return false;
#endif
  }

  void Thread::killOSThread() {
#ifdef __unix__
    pthread_exit( 0 ); // kills thread;
#else
#endif
  }

  void Thread::addThreadToList( Thread* thread ) {
    std::vector<Thread*>::iterator itr = find( threadList.begin(), threadList.end(), this );
    if ( itr != threadList.end() ) {
      // todo : enter critical section
      threadList.push_back( this );    
      // todo : end of critical section  
    }  
  }     

  void Thread::removeThreadFromList( Thread* thread ) {     
    std::vector<Thread*>::iterator itr = find( threadList.begin(), threadList.end(), thread );
    if ( itr != threadList.end() ) {
      // todo critical section begin
      threadList.erase( itr );
      // todo critical section end
    }      
  }     

}
