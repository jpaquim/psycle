///\file
///\brief \interface psycle::plugins::multiplier - * operation
#pragma once
#include "bipolar_filter.hpp"
namespace psycle
{
	namespace plugins
	{
		/// * operation
		class multiplier : public bipolar_filter
		{
		public:
			inline multiplier(engine::plugin_library_reference & plugin_library_reference, engine::graph & graph, const std::string & name)
				: bipolar_filter(plugin_library_reference, graph, name, 1) {}
		protected:
			virtual void do_process() throw(engine::exception);
		};
	}
}
