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
#ifndef PROTOTYPEMACHINEFACTORY_H
#define PROTOTYPEMACHINEFACTORY_H

#include "abstractmachinefactory.h"
#include <map>

namespace psy {
  namespace core {

    class PrototypeMachineFactory : public AbstractMachineFactory {
    public:

      PrototypeMachineFactory();

      ~PrototypeMachineFactory();

      virtual void registerMachine( int key, Machine& prototype );
      virtual Machine* createMachine( int key ) const;

      void setDeleteFlag( bool deletePrototypes );

    private:

      bool deleteProtoypes_;
      std::map<int,Machine*> prototypeMap_;
      
    };

  }
}

#endif