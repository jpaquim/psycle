// -*- mode:c++; indent-tabs-mode:t -*-
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2008 psycle development team http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\interface psycle::plugins::outputs::gstreamer
#pragma once
#include <psycle/detail/project.hpp>
#include "../resource.hpp"
#include <universalis/compiler/numeric.hpp>
#include <gst/gstelement.h>
#include <condition>
#include <mutex>
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__PLUGINS__OUTPUTS__GSTREAMER
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace psycle { namespace plugins { namespace outputs {

	/// outputs to a soundcard device via gstreamer output implementation.
	class UNIVERSALIS__COMPILER__DYNAMIC_LINK gstreamer : public resource {
		protected: friend class virtual_factory_access;
			gstreamer(engine::plugin_library_reference &, engine::graph &, std::string const & name) throw(engine::exception);
		public:
			engine::ports::inputs::single &  in_port() { return *single_input_ports()[0]; }
			void do_name(std::string const &) /*override*/;
			bool opened()  const /*override*/;
			bool started() const /*override*/;
		protected:
			void do_open()    throw(engine::exception) /*override*/;
			void do_start()   throw(engine::exception) /*override*/;
			void do_process() throw(engine::exception) /*override*/;
			void do_stop()    throw(engine::exception) /*override*/;
			void do_close()   throw(engine::exception) /*override*/;
			void channel_change_notification_from_port(engine::port const &) throw(engine::exception) /*override*/;
		private:
			::GstElement * pipeline_, * source_, /** queue_,*/ * caps_filter_, * sink_;

			::GstCaps * caps_;

			void static handoff_static(::GstElement *, ::GstBuffer *, ::GstPad *, gstreamer *);
			void        handoff(::GstBuffer &, ::GstPad &);

			typedef std::scoped_lock<std::mutex> scoped_lock;
			std::mutex mutable mutex_;
			std::condition<scoped_lock> mutable condition_;

			bool wait_for_state_to_become_playing_;
			bool handoff_called_;
			bool stop_requested_;

			char * buffer_;
			unsigned int buffers_, buffer_size_, current_read_position_, current_write_position_;

			typedef universalis::compiler::numeric</*bits_per_channel_sample*/16>::signed_int output_sample_type;
			std::vector<output_sample_type> last_samples_;
	};
}}}
#include <universalis/compiler/dynamic_link/end.hpp>

