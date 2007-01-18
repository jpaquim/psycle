/***************************************************************************
*   Copyright (C) 2007 by  Stefan Nattkemper   *
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
#include "PrototypeMachineFactory.h"
#include "machine.h"

namespace psycle{
  namespace host{

  PrototypeMachineFactory::PrototypeMachineFactory( )
    : deleteProtoypes_( 0 )
  {
   
  }

  PrototypeMachineFactory::~PrototypeMachineFactory()
  {
     if ( deleteProtoypes_ ) {
      std::map<int,Machine*>::const_iterator it = prototypeMap_.begin();
      for ( ; it != prototypeMap_.end(); it++ ) {
        Machine* machine = it->second;
        delete machine;
      }
    }
  }

  void PrototypeMachineFactory::registerMachine( int key, Machine& prototype ) {
    prototypeMap_[key] = &prototype;
  }

  Machine* PrototypeMachineFactory::createMachine( int key ) const {
    std::map<int,Machine*>::const_iterator it = prototypeMap_.find( key );
    if ( it != prototypeMap_.end() )
      return (it->second)->clone();
    else
      return 0;
  }

  void PrototypeMachineFactory::setDeleteFlag( bool deletePrototypes ) {
    deleteProtoypes_ = deletePrototypes;
  }

  }
}