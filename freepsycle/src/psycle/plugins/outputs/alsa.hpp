// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2007 psycle development team http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\interface psycle::plugins::outputs::alsa
#pragma once
#include <psycle/detail/project.hpp>
#include "../resource.hpp"
#include <alsa/asoundlib.h>
#include <cstdint>

#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__PLUGINS__OUTPUTS__ALSA
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace psycle { namespace plugins { namespace outputs {

	/// outputs to a soundcard device via alsa output implementation.
	class UNIVERSALIS__COMPILER__DYNAMIC_LINK alsa : public resource {
		protected: friend class virtual_factory_access;
			alsa(engine::plugin_library_reference &, engine::graph &, const std::string & name) throw(engine::exception);
			virtual ~alsa() throw();
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
			/// pcm device name
			///\todo parametrable
			std::string pcm_device_name_;
			/// pcm device handle
			::snd_pcm_t * pcm_;
			/// number of frames (samples) per period
			///\todo parametrable
			::snd_pcm_uframes_t period_frames_;
			/// samples
			std::int16_t * samples_;
			/// aeras
			::snd_pcm_channel_area_t * areas_;
			/// attached to std output
			::snd_output_t * output_;
	};
}}}
#include <universalis/compiler/dynamic_link/end.hpp>
