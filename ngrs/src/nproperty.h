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
#ifndef NPROPERTY_H
#define NPROPERTY_H

#include <map>
#include <string>
#include "sigslot.h"
#include <vector>

/**
@author Stefan Nattkemper
*/



template<class SetType, class GetType> 
class SetGet {
  public:
    sigslot::signal1<SetType>  set;
    sigslot::signal1<GetType>  get;
    std::string methodName;

    SetGet<SetType,GetType>* clone() const {
      return new SetGet<SetType,GetType>(*this);
    }
};

class NProperty{
public:
    NProperty();

    ~NProperty();

    template<class SetType, class GetType> inline void registrate(const std::string &  methodName, const SetGet<SetType, GetType> & setGet) {
        setGetMap[methodName] = (void*) setGet.clone();
    }

    void write(const std::string & name, const void* value);
    void read(const std::string & name, const void* value);

    //SetGet* registerProperty(std::string propertyName, RuntimeType setType, RuntimeType getType);

private:

   std::map<std::string,void*> setGetMap;
    //std::map<std::string,SetGet>  setGetMap;
    //SetGetSignal setGet;
};

#endif
