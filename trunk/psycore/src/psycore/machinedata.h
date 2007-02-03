/***************************************************************************
*   Copyright (C) 2007 by  Stefan Nattkemper  *
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
#ifndef MACHINEDATA_H
#define MACHINEDATA_H

#include "machine.h"
#include "wire.h"
#include <map>
#include <string>

namespace psy {
  namespace core {

    class MachineSlot {
    public:

      MachineSlot( int number, const std::string& name = "" );
      ~MachineSlot();
      
      int number() const;
      void setName( const std::string& name );
      const std::string& name() const;

      bool operator<( const MachineSlot& rhs ) const;

    private:

      int number_;
      std::string name_;

    };

    class MachineData {
    public:

      MachineData();

      ~MachineData();

      void add( const MachineSlot& slot, Machine* machine );

    private:

      bool delFlag_;
      std::map<MachineSlot, Machine*> machines_;
      std::vector<Wire*> wires_;

    };

  }
}

#endif
