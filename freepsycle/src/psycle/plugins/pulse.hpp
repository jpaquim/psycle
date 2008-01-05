// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2007 psycledelics http://psycle.pastnotecut.org ; johan boule <bohan@jabber.org>

///\interface psycle::plugins::pulse
#pragma once
#include "plugin.hpp"
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__PLUGINS__PULSE
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace psycle { namespace plugins {

class UNIVERSALIS__COMPILER__DYNAMIC_LINK pulse : public engine::node {
	protected: friend class virtual_factory_access;
		pulse(engine::plugin_library_reference &, engine::graph &, std::string const & name);
	protected:
		void do_process() throw(engine::exception) /*override*/;

	public:
		void operator()(real const & sample, std::size_t index = 0) throw() {
			sample_ = sample;
			index_ = index;
			sample_set_ = true;
		}
	private:
		bool sample_set_;
		real sample_;
		std::size_t index_;
};

}}
#include <universalis/compiler/dynamic_link/end.hpp>

