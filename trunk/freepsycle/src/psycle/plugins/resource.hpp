///\file
///\interface psycle::plugins::resource - an abstract plugin for resources.
#pragma once
#include <psycle/detail/project.hpp>
#include "plugin.hpp"
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__PLUGINS__RESOURCE
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace psycle
{
	namespace plugins
	{
		/// resource plugin
		class UNIVERSALIS__COMPILER__DYNAMIC_LINK resource : public engine::node
		{
			protected: friend class virtual_factory_access;
				resource(engine::plugin_library_reference &, engine::graph &, std::string const & name) throw(std::exception);
			protected:
				void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES do_open()  throw(std::exception);
				void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES do_start() throw(std::exception);
				void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES do_stop()  throw(std::exception);
				void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES do_close() throw(std::exception);
		};
	}
}
#include <universalis/compiler/dynamic_link/end.hpp>
