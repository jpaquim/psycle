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
#include "machinedata.h"

namespace psy {
  namespace core {


    MachineSlot::MachineSlot( int number, const std::string& name ) 
      : number_(number),
      name_(name) 
    {
    }
       
    MachineSlot::~MachineSlot() {
    }
      
    int MachineSlot::number() const {
      return number_;
    }

    void MachineSlot::setName( const std::string& name ) {
      name_ = name;
    }

    const std::string& MachineSlot::name() const {
      return name_;
    }

    bool MachineSlot::operator<( const MachineSlot& rhs ) const
    {
      return number() < rhs.number();
    }



    MachineData::MachineData() 
      : delFlag_(1)
    {
    }

    MachineData::~MachineData()
    {
      if ( delFlag_ ) {
        std::map<MachineSlot, Machine*>::iterator it;
        for ( ; it != machines_.end(); ++it ) {
          delete it->second;
        }
      }
    }

    void MachineData::add( const MachineSlot& slot, Machine* machine ) {
       machines_[slot] = machine;
    }

  }
}
