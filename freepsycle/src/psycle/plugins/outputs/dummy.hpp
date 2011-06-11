// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2008-2009 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>

///\interface psycle::plugins::outputs::dummy
#ifndef PSYCLE__PLUGINS__OUTPUTS__DUMMY__INCLUDED
#define PSYCLE__PLUGINS__OUTPUTS__DUMMY__INCLUDED
#pragma once
#include "../resource.hpp"
#include <universalis/stdlib/thread.hpp>
#include <universalis/stdlib/mutex.hpp>
#include <universalis/stdlib/condition.hpp>
#define PSYCLE__DECL  PSYCLE__PLUGINS__OUTPUTS__DUMMY
#include <psycle/detail/decl.hpp>
namespace psycle { namespace plugins { namespace outputs {

using namespace universalis::stdlib;

/// dummy, null silent output.
class PSYCLE__DECL dummy : public resource {
	public:
		dummy(class plugin_library_reference &, name_type const &);
		virtual ~dummy() throw();
		engine::ports::inputs::single &  in_port() { return *single_input_ports()[0]; }
		bool opened()  const /*override*/;
		bool started() const /*override*/;
	protected:
		void do_open() /*override*/;
		void do_start() /*override*/;
		void do_process() /*override*/;
		void do_stop() /*override*/;
		void do_close() /*override*/;
	private:
		bool free_wheeling_;
		nanoseconds sleep_;

		bool opened_;

		thread * thread_;
		void thread_function();
		void thread_loop();

		typedef std::scoped_lock<mutex> scoped_lock;
		mutex mutable mutex_;
		condition<scoped_lock> mutable condition_;

		bool stop_requested_;

		ports::inputs::single in_, amp_;
};

}}}
#include <psycle/detail/decl.hpp>
#endif
