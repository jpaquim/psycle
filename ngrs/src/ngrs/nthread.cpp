/***************************************************************************
 *   Copyright (C) 2005, 2006 by Stefan Nattkemper   *
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
#include "nthread.h"
#include <algorithm>

std::vector<NThread*> NThread::threadList;

NThread::NThread() : terminated_(1) {
                   
}                   
   
NThread::~NThread() {
                    
}                    


void NThread::execute() {
  // override this virtual method
  // called by the OS Thread
}        

void NThread::start() {
  if ( terminated_ )
    terminated_ = !createOSThread();    
}     
     
void NThread::terminate() {
  terminated_ = true;     
}     

bool NThread::terminated() const {
  return terminated_;    
}     

// static callBack

void* NThread::callBack( void* ptr ) {
   std::vector<NThread*>::iterator itr = find( threadList.begin(), threadList.end(), static_cast<NThread*> (ptr) );
   if ( itr != threadList.end() ) {
     NThread* thread = *itr;
     thread->execute();     
     thread->killOSThread();
   }

   return 0;
}      

// creates an OS Thread

bool NThread::createOSThread() {
  
  addThreadToList( this );
       
  #ifdef __unix__
  if ( 0 == pthread_create( &threadid, NULL, (void*(*)(void*))NThread::callBack, (void*) this)  ) {
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

void NThread::killOSThread() {
  #ifdef __unix__
  pthread_exit( 0 ); // kills thread;
  #else
  #endif
}

void NThread::addThreadToList( NThread* thread ) {
  std::vector<NThread*>::iterator itr = find( threadList.begin(), threadList.end(), this );
  if ( itr != threadList.end() ) {
    // todo : enter critical section
    threadList.push_back( this );    
    // todo : end of critical section  
  }  
}     

void NThread::removeThreadFromList( NThread* thread ) {     
   std::vector<NThread*>::iterator itr = find( threadList.begin(), threadList.end(), thread );
   if ( itr != threadList.end() ) {
     // todo critical section begin
     threadList.erase( itr );
     // todo critical section end
   }      
}     
