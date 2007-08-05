///\file
///\brief \interface psycle::plugins::outputs::jack
///\author erodix
#pragma once
#include <psycle/detail/project.hpp>

#if defined OPERATING_SYSTEM__MICROSOFT
	#error "this plugin is specific to *nix operating system"
#endif

#include "../resource.hpp"
#include <jack/jack.h>
namespace psycle
{
	namespace plugins
	{
		namespace outputs
		{
			// outputs to a soundcard device via jackd
			class jack : public resource
			{
			public:
				jack(engine::plugin_library_reference &, engine::graph &, const std::string & name) throw(engine::exception);
				bool override opened() const;
				bool override started() const;
			protected:
				void override do_open() throw(engine::exception);
				void override do_start() throw(engine::exception);
				void override do_process() throw(engine::exception);
				void override do_stop() throw(engine::exception);
				void override do_close() throw(engine::exception);
			private:
				jack_client_t * client_;
				jack_port_t * output_port_;
			};
		}
	}
}
