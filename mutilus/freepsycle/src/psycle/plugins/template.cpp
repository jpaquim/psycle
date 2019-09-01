///\file
///\brief \implementation psycle::plugins::template_plugin
#include <psycle/detail/project.private.hpp>
#include "template.hpp"
#include <limits> // std::numeric_limits<real>::min_denorm() infinity()
#include <iostream>
namespace psycle
{
	namespace plugins
	{
		PSYCLE__PLUGINS__NODE_INSTANCIATOR(template_plugin)

		template_plugin::template_plugin(engine::plugin_library_reference & plugin_library_reference, engine::graph & graph, const std::string & name)
		:
			node(plugin_library_reference, graph, name)
		{
			if(loggers::trace()())
			{
				std::ostringstream s; s << qualified_name() << " new template plugin";
				loggers::trace()(s.str());
			}
			new engine::ports::inputs::single(*this, "in");
			new engine::ports::output(*this, "out")
		}

		void template_plugin::do_process() throw(engine::exception)
		{
			// template
			real sample(0);
			for(real i = 4 ; i >= 0.25 ; i /= 2) sample += i;
			std::cout << qualified_name() << ' ' << sample << endl;

			// c++ exception:
			//throw exception("template!", UNIVERSALIS__COMPILER__LOCATION);

			// division by 0:
			//volatile int i(0); i = 0 / i; // trick so that the compiler does not remove the code when optimizing

			// infinite loop so that we can test interruption signal:
			//while(true);
		}
	}
}
