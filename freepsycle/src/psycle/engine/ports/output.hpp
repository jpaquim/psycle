// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2000-2007 psycledelics http://psycle.sourceforge.net

///\interface psycle::engine::ports::output
#pragma once
#include "../port.hpp"
#include <boost/signal.hpp>
#include <vector>
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__ENGINE__PORTS__OUTPUT
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace psycle
{
	namespace engine
	{
		namespace ports
		{
			/// handles an output stream of signal parting from a node
			class UNIVERSALIS__COMPILER__DYNAMIC_LINK output : public typenames::typenames::bases::ports::output
			{
				friend class graph;
				friend class node;
				friend class input;
				friend class inputs::single;
				friend class inputs::multiple;
					
				protected: friend class virtual_factory_access;
					output(parent_type &, name_type const &, int const & channels = 0);
					virtual ~output();
	
				protected:
					void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES do_buffer(engine::buffer * const);
					void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES do_propagate_channels() throw(exception);
					void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES do_propagate_seconds_per_event();
	
				public:
					void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES dump(std::ostream &, int const & tabulations = 0) const;
			};
		}
	}
}
#include <universalis/compiler/dynamic_link/end.hpp>
