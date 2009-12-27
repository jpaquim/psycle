// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2009 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>

#pragma once

#if defined PSYCLE__GSTREAMER_AVAILABLE

#include "audiodriver.h"
#include <gst/gstelement.h>
#include <universalis/stdlib/cstdint.hpp>

namespace psycle { namespace core {

using namespace universalis::stdlib;

class GStreamerOut : public AudioDriver {
	public:
		GStreamerOut();
		~GStreamerOut();

		/*override*/ AudioDriverInfo info() const {
			return AudioDriverInfo("gstreamer", "GStreamer Driver", "Output through the GStreamer infrastructure", true);
		}

	public:
		/*override*/ void Initialize(AUDIODRIVERWORKFN callback, void * context) { callback_ = callback; callback_context_ = context; }
		/*override*/ bool Initialized() { return callback_; }
	private:
		AUDIODRIVERWORKFN callback_;
		void * callback_context_;

	public:
		/*override*/ void Configure();
		/*override*/ bool Configured() const;
		/*override*/ bool Enable(bool e);
		/*override*/ bool Enabled() const;
	private:
		void do_open();
		void do_start();
		void do_stop();
		void do_close();
		
		::GstElement * pipeline_, * source_, * caps_filter_, * sink_;

		::GstCaps * caps_;

		void static handoff_static(::GstElement *, ::GstBuffer *, ::GstPad *, GStreamerOut *);
		void        handoff(::GstBuffer &, ::GstPad &);

		typedef int16_t output_sample_type;
		unsigned int channels_;
		unsigned int samples_per_second_;
		unsigned int periods_;
		unsigned int period_frames_;
};

}}

#endif // defined PSYCLE__GSTREAMER_AVAILABLE
