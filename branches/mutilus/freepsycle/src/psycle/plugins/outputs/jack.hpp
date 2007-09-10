///\file
///\brief \interface psycle::plugins::outputs::jack
///\author erodix
#pragma once
#include <psycle/detail/project.hpp>

#include "../resource.hpp"
#include <jack/jack.h>

#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__PLUGINS__OUTPUTS__JACK
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace psycle
{
	namespace plugins
	{
		namespace outputs
		{
			// outputs to a soundcard device via jackd
			class UNIVERSALIS__COMPILER__DYNAMIC_LINK jack : public resource
			{
				protected: friend class factory;
					jack(engine::plugin_library_reference &, engine::graph &, std::string const & name) throw(engine::exception);
				public:
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
#include <universalis/compiler/dynamic_link/end.hpp>
