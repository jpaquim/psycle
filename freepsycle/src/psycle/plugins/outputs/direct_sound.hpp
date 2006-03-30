///\file
///\brief \interface psycle::plugins::devices::outputs::direct_sound
#pragma once
#include <psycle/detail/project.hpp>
#include <universalis/operating_system/exception.hpp>
#include <universalis/compiler/numeric.hpp>

#if !defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
	#error "this plugin is specific to microsoft's operating system"
#endif

#include <windows.h>

#if defined DIVERSALIS__COMPILER__FEATURE__AUTOLINK
	#pragma comment(lib, "user32") // for ::GetDesktopWindow(), see implementation file
#endif

#if defined DIVERSALIS__COMPILER__MICROSOFT
	#pragma warning(push)
	#pragma warning(disable:4201) // nonstandard extension used : nameless struct/union
#endif

#include <mmsystem.h>

#if defined DIVERSALIS__COMPILER__MICROSOFT
	#pragma warning(pop)
#endif

#include <dsound.h> // dsound lib
#if defined DIVERSALIS__COMPILER__FEATURE__AUTOLINK
	#pragma comment(lib, "dsound")
#endif

#include "../resource.hpp"
namespace psycle
{
	namespace plugins
	{
		namespace outputs
		{
			/// outputs to a soundcard device via direct sound output implementation.
			class direct_sound : public resource
			{
				public:
					direct_sound(engine::plugin_library_reference &, engine::graph &, const std::string & name) throw(universalis::operating_system::exception);
					bool UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES opened() const;
					bool UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES started() const;
				protected:
					void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES do_open() throw(universalis::operating_system::exception);
					void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES do_start() throw(universalis::operating_system::exception);
					void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES do_process() throw(universalis::operating_system::exception);
					void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES do_stop() throw(universalis::operating_system::exception);
					void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES do_close() throw(universalis::operating_system::exception);
				private:
					::IDirectSound * direct_sound_;
					::IDirectSound inline & direct_sound_instance() throw() { assert(direct_sound_); return *direct_sound_; }
					::IDirectSoundBuffer mutable * buffer_;
					::IDirectSoundBuffer inline & buffer() throw() { assert(buffer_); return *buffer_; }
					::IDirectSoundBuffer inline & buffer() const throw() { assert(buffer_); return *buffer_; }
					int bits_per_sample_;
					bool allocate_buffer() throw(universalis::operating_system::exception);
					bool write_primary_;
					int samples_per_buffer_;
					int buffers_;
					unsigned long int total_buffering_size_;
					unsigned long int bytes_per_buffer_;
					int current_buffer_;
					universalis::compiler::numeric<16>::signed_int last_sample_;
			};
		}
	}
}
