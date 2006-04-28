// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2006 Johan Boule <bohan@jabber.org>
// Copyright (C) 2004-2006 Psycledelics http://psycle.pastnotecut.org

///\interface psycle::engine::ports::input
#pragma once
#include "../port.hpp"
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__ENGINE__PORTS__INPUT
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace psycle
{
	namespace engine
	{
		namespace ports
		{
			/// handles an input stream of signal coming to a node.
			class UNIVERSALIS__COMPILER__DYNAMIC_LINK input : public typenames::typenames::bases::ports::input
			{
				friend class node;
				
				protected:
					input(parent_type &, name_type const &, int const & channels = 0); friend class generic_access;
				public:
					virtual ~input() throw();
	
				public:
					void    connect(typenames::ports::output &) throw(exception);
					void disconnect(typenames::ports::output &);
	
				public:
					void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES dump(std::ostream &, int const & tabulations = 0) const = 0;
			};
		}
	}
}
#include <universalis/compiler/dynamic_link/end.hpp>
