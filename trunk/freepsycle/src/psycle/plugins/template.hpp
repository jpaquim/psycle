///\file
///\brief \interface psycle::plugins::template_plugin
#pragma once
#include "plugin.hpp"
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__PLUGINS__TEMPLATE
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace psycle
{
	namespace plugins
	{
		class UNIVERSALIS__COMPILER__DYNAMIC_LINK template_plugin : public engine::node
		{
			protected: friend class virtual_factory_access;
				template_plugin(engine::plugin_library_reference &, engine::graph &, std::string const & name);
			protected:
				virtual void do_process() throw(engine::exception);
		};
	}
}
#include <universalis/compiler/dynamic_link/end.hpp>
