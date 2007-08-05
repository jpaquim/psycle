///\file
///\brief \interface psycle::plugins::additioner - + operation
#pragma once
#include "bipolar_filter.hpp"
namespace psycle
{
	namespace plugins
	{
		/// + operation
		class additioner : public bipolar_filter
		{
		public:
			inline additioner(engine::plugin_library_reference & plugin_library_reference, engine::graph & graph, const std::string & name)
				: bipolar_filter(plugin_library_reference, graph, name, 0) {}
		protected:
			virtual void do_process() throw(engine::exception);
		};
	}
}
