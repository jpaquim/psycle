// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2007 psycle development team http://psycle.sourceforge.net ; erodix ; johan boule <bohan@jabber.org>

///\interface psycle::plugins::outputs::jack
#pragma once
#include <psycle/detail/project.hpp>

#include "../resource.hpp"
#include <jack/jack.h>

#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__PLUGINS__OUTPUTS__JACK
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace psycle { namespace plugins { namespace outputs {

	/// outputs to a soundcard device via jackd
	class UNIVERSALIS__COMPILER__DYNAMIC_LINK jack : public resource {
		protected: friend class virtual_factory_access;
			jack(engine::plugin_library_reference &, engine::graph &, std::string const & name) throw(engine::exception);
		public:
			bool opened()  const /*override*/;
			bool started() const /*override*/;
		protected:
			void do_open()    throw(engine::exception) /*override*/;
			void do_start()   throw(engine::exception) /*override*/;
			void do_process() throw(engine::exception) /*override*/;
			void do_stop()    throw(engine::exception) /*override*/;
			void do_close()   throw(engine::exception) /*override*/;
		private:
			jack_client_t * client_;
			jack_port_t * output_port_;
	};
}}}
#include <universalis/compiler/dynamic_link/end.hpp>

