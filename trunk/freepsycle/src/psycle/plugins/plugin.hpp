///\file
///\brief interface file containing the declarations needed by psycle::plugins.
#pragma once
#include <psycle/engine/engine.hpp>
#include <boost/ref.hpp>
#include <cmath>

///\internal
/// extensible modular audio frawework.
namespace psycle
{
	///\internal
	/// functionalities used by the plugin side only, not by the host.
	/// Place your plugins in this namespace.
	namespace plugins
	{
		typedef engine::real real;
		
		///\internal
		/// plugins driving an underlying output device.
		namespace outputs
		{
		}

		#define PSYCLE__PLUGINS__NODE_INSTANCIATOR(typename) \
			extern "C" \
			{ \
				psycle::engine::node & PSYCLE__ENGINE__NODE_INSTANCIATOR__SYMBOL(new) \
				( \
					psycle::engine::plugin_library_reference & plugin_library_reference, \
					psycle::engine::graph & graph, \
					std::string const & name \
				) \
				throw(psycle::engine::exception) \
				{ \
					return psycle::engine::node::create<typename>(plugin_library_reference, graph, name); \
				} \
				\
				void PSYCLE__ENGINE__NODE_INSTANCIATOR__SYMBOL(delete)(psycle::engine::node * const node) \
				{ \
					delete node; \
				} \
			}
	}
}
