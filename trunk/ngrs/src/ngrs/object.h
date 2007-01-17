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
#ifndef OBJECT_H
#define OBJECT_H

#include "sigslot.h"
#include "keyaccelerator.h"
#include <string>
#include <vector>

namespace ngrs {

  class Event;
  class PropertyMap;

  using namespace sigslot;


  /**
  @author  Stefan
  */
  class Object : public sigslot::has_slots<> {
  public:
    Object();

    ~Object();

    void setName(const std::string & name);
    const std::string & name() const;

    void sendMessage(Event* ev);
    void addMessageListener(Object* obj);

    virtual void onKeyAcceleratorNotify(KeyAccelerator accelerator);
    virtual void onMessage(Event* event);

    PropertyMap* properties() const;

  private:

    std::string name_;
    std::vector<Object*> msgListener;

    PropertyMap* properties_;
    void getNameProperty(std::string const & name);

  };

  // the types of the class factories
  typedef Object* create_t();
  typedef void destroy_t(Object*);

}

#endif
