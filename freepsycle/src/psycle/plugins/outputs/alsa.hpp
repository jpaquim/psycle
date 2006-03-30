///\file
///\brief \interface psycle::plugins::outputs::alsa
#pragma once
#include <psycle/detail/project.hpp>

#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
	#error "ain't for microsoft"
#endif

#include "../resource.hpp"
#include <alsa/asoundlib.h>
namespace psycle
{
	namespace plugins
	{
		namespace outputs
		{
			/// outputs to a soundcard device via alsa output implementation.
			class alsa : public resource
			{
				public:
					alsa(engine::plugin_library_reference &, engine::graph &, const std::string & name) throw(engine::exception);
					virtual ~alsa() throw();
					bool UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES opened() const;
					bool UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES started() const;
				protected:
					void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES do_open() throw(engine::exception);
					void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES do_start() throw(engine::exception);
					void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES do_process() throw(engine::exception);
					void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES do_stop() throw(engine::exception);
					void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES do_close() throw(engine::exception);
				private:
					std::string pcm_name_;
					::snd_pcm_hw_params_t * pcm_hw_params_;
					::snd_pcm_t * pcm_;
			};
		}
	}
}
