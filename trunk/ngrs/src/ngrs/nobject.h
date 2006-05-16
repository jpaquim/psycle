/***************************************************************************
 *   Copyright (C) 2005 by Stefan   *
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
#ifndef NOBJECT_H
#define NOBJECT_H

#include "ngrs/sigslot.h"
#include "ngrs/nkeyaccelerator.h"
#include <string>
#include <vector>


class NEvent;
class NPropertyMap;

using namespace sigslot;


/**
@author Stefan
*/
class NObject : public sigslot::has_slots<> {
public:
    NObject();

    ~NObject();

    void setName(const std::string & name);
    const std::string & name() const;

    void sendMessage(NEvent* ev);
    void addMessageListener(NObject* obj);

    virtual void onKeyAcceleratorNotify(NKeyAccelerator accelerator);
    virtual void onMessage(NEvent* event);

    NPropertyMap* properties() const;

private:

   std::string name_;
   std::vector<NObject*> msgListener;

   NPropertyMap* properties_;
   void getNameProperty(std::string const & name);

};

// the types of the class factories
typedef NObject* create_t();
typedef void destroy_t(NObject*);

#endif
