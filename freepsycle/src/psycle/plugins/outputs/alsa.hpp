// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 psycledelics http://psycle.pastnotecut.org : johan boule

///\interface psycle::plugins::outputs::alsa
#pragma once
#include <psycle/detail/project.hpp>

#include "../resource.hpp"
#include <alsa/asoundlib.h>

#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__PLUGINS__OUTPUTS__ALSA
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace psycle { namespace plugins { namespace outputs {

	/// outputs to a soundcard device via alsa output implementation.
	class UNIVERSALIS__COMPILER__DYNAMIC_LINK alsa : public resource {
		protected: friend class virtual_factory_access;
			alsa(engine::plugin_library_reference &, engine::graph &, const std::string & name) throw(engine::exception);
			virtual ~alsa() throw();
		public:
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
}}}
#include <universalis/compiler/dynamic_link/end.hpp>

