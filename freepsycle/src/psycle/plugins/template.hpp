///\file
///\brief \interface psycle::plugins::template_plugin
#pragma once
#include "plugin.hpp"
namespace psycle
{
	namespace plugins
	{
		class template_plugin : public engine::node
		{
		public:
			template_plugin(engine::plugin_library_reference &, engine::graph &, const std::string & name);
		protected:
			virtual void do_process() throw(engine::exception);
		};
	}
}
