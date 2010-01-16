// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2010 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

#ifndef PSYCLE__AUDIODRIVERS__GSTREAMER_OUT__INCLUDED
#define PSYCLE__AUDIODRIVERS__GSTREAMER_OUT__INCLUDED
#pragma once

#if defined PSYCLE__GSTREAMER_AVAILABLE

#include "audiodriver.h"
#include <gst/gstelement.h>
#include <universalis/stdlib/cstdint.hpp>

namespace psycle { namespace audiodrivers {

using namespace universalis::stdlib;

class GStreamerOut : public AudioDriver {
	public:
		GStreamerOut();
		~GStreamerOut() throw();
		/*override*/ AudioDriverInfo info() const;

	protected:
		/*override*/ void do_open();
		/*override*/ void do_start();
		/*override*/ void do_stop();
		/*override*/ void do_close();

	private:
		::GstElement * pipeline_, * source_, * caps_filter_, * sink_;
		::GstCaps * caps_;
		void static handoff_static(::GstElement *, ::GstBuffer *, ::GstPad *, GStreamerOut *);
		void handoff(::GstBuffer &, ::GstPad &);
		typedef int16_t output_sample_type;
};

}}

#endif // defined PSYCLE__GSTREAMER_AVAILABLE
#endif
