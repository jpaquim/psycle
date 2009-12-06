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
		~GStreamerOut() { close(); }

		/*override*/ AudioDriverInfo info() const {
			return AudioDriverInfo("gstreamer", "GStreamer Driver", "Output through the GStreamer infrastructure", true);
		}

		/*override*/ void Configure() {}
	
	public:
		/*override*/ void Initialize(AUDIODRIVERWORKFN callback, void * context) { callback_ = callback; callback_context_ = context; }
		/*override*/ bool Initialized() { return callback_; }
	private:
		AUDIODRIVERWORKFN callback_;
		void * callback_context_;

	public:
		/*override*/ bool Enable(bool e) { if(e) start(); else stop(); return true; }
	private:
		bool opened() const;
		void open() { if(!opened()) try { do_open(); } catch(...) { do_close(); throw; } }
		void close() { stop(); if(opened()) do_close(); }
		
		bool started() const;
		void start() { open(); if(!started()) try { do_start(); } catch(...) { do_stop(); throw; } }
		void stop() { if(started()) do_stop(); }
		
		void do_open();
		void do_start();
		void do_process();
		void do_stop();
		void do_close();
		
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

		typedef float output_sample_type;
		int channels_;
		int samples_per_second_;
};

}}

#endif // defined PSYCLE__GSTREAMER_AVAILABLE
