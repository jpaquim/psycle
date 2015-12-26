// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2015 members of the psycle project http://psycle.sourceforge.net

#pragma once
#include <psycle/host/detail/project.hpp>
#include "Psycle.hpp"

namespace psycle {
namespace host {

template <class AdapterImp, class Target, class Adaptee>
class AdapterWithCreate : public Target {
 public:
  AdapterWithCreate<AdapterImp, Target, Adaptee>() 
    : adaptee_(Adaptee::CreateInstance(reinterpret_cast<AdapterImp*>(this))) {}
  virtual ~AdapterWithCreate<AdapterImp, Target, Adaptee>() { Adaptee::DestroyInstance(adaptee_); }
  Adaptee& adaptee() { return *adaptee_; }
  const Adaptee& adaptee() const { return *adaptee_; }
 protected:
  Adaptee* adaptee_;
};

template <class AdapterImp, class Target, class Adaptee>
class AdapterWithNew : public Target {
 public:
  AdapterWithNew<AdapterImp, Target, Adaptee>() : adaptee_(reinterpret_cast<AdapterImp*>(new Adaptee(this))) {}
  Adaptee& adaptee() { return *adaptee_; }
  const Adaptee& adaptee() const { return *adaptee_; }
 protected:
  std::auto_ptr<Adaptee> adaptee_;
};

template <class AdapterImp, class Target, class Adaptee>
class Adapter : public Target {
 public:
  Adapter<AdapterImp, Target, Adaptee>() : adaptee_(reinterpret_cast<AdapterImp*>(new Adaptee())) {}
  Adaptee& adaptee() { return *adaptee_; }
  const Adaptee& adaptee() const { return *adaptee_; }
 protected:
  std::auto_ptr<Adaptee> adaptee_;
};


} // host
} // psycle