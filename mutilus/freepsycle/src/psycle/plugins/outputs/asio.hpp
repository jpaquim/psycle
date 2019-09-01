///\file
///\brief \interface psycle::plugins::outputs::asio
#pragma once
#include <psycle/detail/project.hpp>

#if !defined OPERATING_SYSTEM__MICROSOFT
	#error "this plugin is specific to microsoft's operating system"
#endif

#include "../resource.hpp"

#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__PLUGINS__OUTPUTS__ASIO
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace psycle
{
	namespace plugins
	{
		namespace outputs
		{
			// outputs to a soundcard device via steinberg's asio output implementation.
			class UNIVERSALIS__COMPILER__DYNAMIC_LINK asio : public resource
			{
				protected: friend class factory;
					asio(engine::plugin_library_reference &, engine::graph &, const std::string & name) throw(engine::exception);
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
			};
		}
	}
}
#include <universalis/compiler/dynamic_link/end.hpp>
