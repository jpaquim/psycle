// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2009 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>

///\interface psycle::plugins::outputs::asio
#pragma once
#include <psycle/detail/project.hpp>

#if !defined DIVERSALIS__OS__MICROSOFT
	#error "this plugin is specific to microsoft's operating system"
#endif

#include "../resource.hpp"

#define PSYCLE__DECL  PSYCLE__PLUGINS__OUTPUTS__ASIO
#include <psycle/detail/decl.hpp>
namespace psycle { namespace plugins { namespace outputs {

	/// outputs to a soundcard device via steinberg's asio output implementation.
	class PSYCLE__DECL asio : public resource {
		protected: friend class virtual_factory_access;
			asio(engine::plugin_library_reference &, engine::graph &, const std::string & name) throw(engine::exception);
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
	};
}}}
#include <psycle/detail/decl.hpp>
