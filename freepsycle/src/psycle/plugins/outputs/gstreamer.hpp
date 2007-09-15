// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycle development team http://psycle.sourceforge.net

///\interface psycle::plugins::outputs::gstreamer
#pragma once
#include <psycle/detail/project.hpp>
#include "../resource.hpp"
#include <gst/gstelement.h>
#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__PLUGINS__OUTPUTS__GSTREAMER
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace psycle
{
	namespace plugins
	{
		namespace outputs
		{
			/// outputs to a soundcard device via gstreamer output implementation.
			class UNIVERSALIS__COMPILER__DYNAMIC_LINK gstreamer : public resource
			{
				protected: friend class virtual_factory_access;
					gstreamer(engine::plugin_library_reference &, engine::graph &, std::string const & name) throw(engine::exception);
				public:
					void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES do_name(std::string const &);
					bool UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES opened()  const;
					bool UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES started() const;
				protected:
					void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES do_open()    throw(engine::exception);
					void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES do_start()   throw(engine::exception);
					void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES do_process() throw(engine::exception);
					void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES do_stop()    throw(engine::exception);
					void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES do_close()   throw(engine::exception);
				private:
					::GstElement * pipeline_, * source_, * sink_;

					unsigned int  samples_per_second_;
					unsigned char channels_;
					unsigned char significant_bits_per_channel_sample_;

					::GstCaps * caps_;
					bool caps_set_;

					void static handoff_static(::GstElement *, ::GstBuffer *, ::GstPad *, gstreamer *);
					void        handoff(::GstBuffer &, ::GstPad &);

					boost::condition condition_;
					boost::mutex mutex_;
					bool waiting_for_state_to_become_playing_;

					char * buffer_;
					unsigned int buffers_, buffer_size_, total_buffer_size_, current_read_position_, current_write_position_;
					unsigned int samples_per_buffer_;
			};
		}
	}
}
#include <universalis/compiler/dynamic_link/end.hpp>
