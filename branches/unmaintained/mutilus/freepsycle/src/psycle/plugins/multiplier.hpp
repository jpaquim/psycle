///\file
///\brief \interface psycle::plugins::multiplier - * operation
#pragma once
#include "bipolar_filter.hpp"
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__PLUGINS__MULTIPLIER
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace psycle
{
	namespace plugins
	{
		/// * operation
		class UNIVERSALIS__COMPILER__DYNAMIC_LINK multiplier : public bipolar_filter
		{
			protected: friend class factory;
				inline multiplier(engine::plugin_library_reference & plugin_library_reference, engine::graph & graph, std::string const & name)
					: bipolar_filter(plugin_library_reference, graph, name, 1) {}
			protected: 
				virtual void do_process() throw(engine::exception);
		};
	}
}
#include <universalis/compiler/dynamic_link/end.hpp>
