///\file
///\interface psycle::plugins::resource - an abstract plugin for resources.
#pragma once
#include <psycle/detail/project.hpp>
#include "plugin.hpp"
namespace psycle
{
	namespace plugins
	{
		/// resource plugin
		class resource : public engine::node
		{
			protected:
				resource(engine::plugin_library_reference &, engine::graph &, std::string const & name) throw(std::exception);
				void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES do_open()  throw(std::exception);
				void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES do_start() throw(std::exception);
				void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES do_stop()  throw(std::exception);
				void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES do_close() throw(std::exception);
		};
	}
}
