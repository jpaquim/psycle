// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2009 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>

///\interface psycle::plugins::devices::outputs::direct_sound
#ifndef PSYCLE__PLUGINS__OUTPUTS__DIRECT_SOUND__INCLUDED
#define PSYCLE__PLUGINS__OUTPUTS__DIRECT_SOUND__INCLUDED
#pragma once
#include "../resource.hpp"

#if !defined DIVERSALIS__OS__MICROSOFT
	#error "this plugin is specific to microsoft's operating system"
#endif

#include <windows.h>

#if defined DIVERSALIS__COMPILER__FEATURE__AUTO_LINK
	#pragma comment(lib, "user32") // for ::GetDesktopWindow(), see implementation file
#endif

#if defined DIVERSALIS__COMPILER__MICROSOFT
	#pragma warning(push)
	#pragma warning(disable:4201) // nonstandard extension used : nameless struct/union
#endif

#include <mmsystem.h> // winmm lib
#if defined DIVERSALIS__COMPILER__FEATURE__AUTO_LINK
	// unused #pragma comment(lib, "winmm")
#endif

#if defined DIVERSALIS__COMPILER__MICROSOFT
	#pragma warning(pop)
#endif

#include <dsound.h> // dsound lib
#if defined DIVERSALIS__COMPILER__FEATURE__AUTO_LINK
	#pragma comment(lib, "dsound")
#endif

#define PSYCLE__DECL  PSYCLE__PLUGINS__OUTPUTS__DIRECT_SOUND
#include <psycle/detail/decl.hpp>
namespace psycle { namespace plugins { namespace outputs {

	/// outputs to a soundcard device via microsoft's direct sound output implementation.
	class PSYCLE__DECL direct_sound : public resource {
		public:
			direct_sound(class plugin_library_reference &, name_type const &);
			ports::inputs::single & in_port() { return *single_input_ports()[0]; }
			bool opened()  const /*override*/;
			bool started() const /*override*/;
		protected:
			void do_open() /*override*/;
			void do_start() /*override*/;
			void do_process() /*override*/;
			void do_stop() /*override*/;
			void do_close() /*override*/;
			void channel_change_notification_from_port(engine::port const &) /*override*/;
		private:
			::IDirectSound * direct_sound_;
			::IDirectSound & direct_sound_implementation() { assert(direct_sound_); return *direct_sound_; }

			typedef universalis::compiler::numeric</*bits_per_channel_sample*/16>::signed_int output_sample_type;
			std::vector<output_sample_type> last_samples_;

			::IDirectSoundBuffer mutable * buffer_;
			::IDirectSoundBuffer inline  & buffer()       { assert(buffer_); return *buffer_; }
			::IDirectSoundBuffer inline  & buffer() const { assert(buffer_); return *buffer_; }

			bool write_primary_, started_;
			unsigned int buffers_, buffer_size_, total_buffer_size_;
			/// position in byte offset
			unsigned int current_position_;
			unsigned int samples_per_buffer_;
	};
}}}
#include <psycle/detail/decl.hpp>
#endif
