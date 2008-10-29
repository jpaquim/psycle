// -*- mode:c++; indent-tabs-mode:t -*-
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2008-2008 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\interface psycle::plugins::outputs::dummy
#pragma once
#include "../resource.hpp"
#include <thread>
#include <mutex>
#include <condition>
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__PLUGINS__OUTPUTS__DUMMY
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace psycle { namespace plugins { namespace outputs {

	/// dummy, null silent output.
	class UNIVERSALIS__COMPILER__DYNAMIC_LINK dummy : public resource {
		protected: friend class virtual_factory_access;
			dummy(engine::plugin_library_reference &, engine::graph &, const std::string & name) throw(engine::exception);
			virtual ~dummy() throw();
		public:
			engine::ports::inputs::single &  in_port() { return *single_input_ports()[0]; }
			bool opened()  const /*override*/;
			bool started() const /*override*/;
		protected:
			void do_open()    throw(engine::exception) /*override*/;
			void do_start()   throw(engine::exception) /*override*/;
			void do_process() throw(engine::exception) /*override*/;
			void do_stop()    throw(engine::exception) /*override*/;
			void do_close()   throw(engine::exception) /*override*/;
		private:
			bool free_wheeling_;
			std::nanoseconds sleep_;
			
			bool opened_;
			
			std::thread * thread_;
			void thread_function();
			void thread_loop() throw(engine::exception);
			
			typedef std::scoped_lock<std::mutex> scoped_lock;
			std::mutex mutable mutex_;
			std::condition<scoped_lock> mutable condition_;

			bool stop_requested_;
	};
}}}
#include <universalis/compiler/dynamic_link/end.hpp>
