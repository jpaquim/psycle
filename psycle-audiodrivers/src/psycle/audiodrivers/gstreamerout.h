// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2009 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>

#pragma once

#if defined PSYCLE__GSTREAMER_AVAILABLE

#include "audiodriver.h"
#include <gst/gstelement.h>
#include <condition>
#include <mutex>

namespace psy { namespace core {

class GStreamerOut : public AudioDriver {
	public:
		GStreamerOut();
		~GStreamerOut();
	
	public:
		/*override*/ AudioDriverInfo info() const;
	private:
		std::string const & name() const { return name_; }
		std::string const name_;

	private:
		bool opened() const;
		void opened(bool value) { if(value) open(); else close(); }
		void open() { if(!opened()) try { do_open(); } catch(...) { do_close(); throw; } }
		void close() { stop(); if(opened()) do_close(); }
		
		bool started() const;
		void started(bool value) { if(value) start(); else stop(); }
		void start() { open(); if(!started()) try { do_start(); } catch(...) { do_stop(); throw; } }
		void stop() { if(started()) do_stop(); }
		
	private:
		void do_open();
		void do_start();
		void do_process();
		void do_stop();
		void do_close();
		
	private:
		void io_ready(bool b) { io_ready_ = b; }
		bool io_ready() const { return io_ready_; }
		bool io_ready_;

	private:
		::GstElement * pipeline_, * source_, * caps_filter_, * sink_;

		::GstCaps * caps_;

		void static handoff_static(::GstElement *, ::GstBuffer *, ::GstPad *, GStreamerOut *);
		void        handoff(::GstBuffer &, ::GstPad &);

		typedef std::scoped_lock<std::mutex> scoped_lock;
		std::mutex mutable mutex_;
		std::condition<scoped_lock> mutable condition_;

		bool wait_for_state_to_become_playing_;
		bool handoff_called_;
		bool stop_requested_;

		/// intermediate buffer between do_process() and the routine that writes to the gstreamer buffer
		char * intermediate_buffer_;
		
		/// pointer to current gstreamer read position in the intermediate buffer
		char * intermediate_buffer_current_read_pointer_;
		
		/// pointer to end of intermediate buffer
		char * intermediate_buffer_end_;
		
		typedef universalis::compiler::numeric</*bits_per_channel_sample*/16>::signed_int output_sample_type;
		std::vector<output_sample_type> last_samples_;
};

}}

#endif // defined PSYCLE__GSTREAMER_AVAILABLE
