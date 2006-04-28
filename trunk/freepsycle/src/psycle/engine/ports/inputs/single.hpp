// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2006 Johan Boule <bohan@jabber.org>
// Copyright (C) 2004-2006 Psycledelics http://psycle.pastnotecut.org

///\interface psycle::engine::ports::inputs::single
#pragma once
#include "../input.hpp"
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__ENGINE__PORTS__INPUTS__SINGLE
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace psycle
{
	namespace engine
	{
		namespace ports
		{
			namespace inputs
			{
				/// handles an input stream of signal coming to a node.
				class UNIVERSALIS__COMPILER__DYNAMIC_LINK single : public typenames::typenames::bases::ports::inputs::single
				{
					friend class node;
	
					protected:
						single(parent_type &, name_type const &, int const & channels = 0); friend class generic_access;
					public:
						virtual ~single() throw();
		
					protected:
						void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES do_propagate_channels() throw(exception);
						void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES do_propagate_seconds_per_event();
		
					public:
						void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES dump(std::ostream &, int const & tabulations = 0) const;
				};
			}
		}
	}
}
#include <universalis/compiler/dynamic_link/end.hpp>
