// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2006 Johan Boule <bohan@jabber.org>
// Copyright (C) 2004-2006 Psycledelics http://psycle.pastnotecut.org

///\interface psycle::engine::ports::inputs::single
#pragma once
#include "../input.hpp"
#if defined PSYCLE__EXPERIMENTAL
	#include <psycle/generic/generic.hpp>
#endif
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__ENGINE__PORTS__INPUTS__SINGLE
#include <universalis/compiler/dynamic_link/begin.hpp>

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
#if defined PSYCLE__EXPERIMENTAL
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

namespace psycle
{
	namespace engine
	{
		namespace ports
		{
			namespace inputs
			{
				typedef generic::ports::inputs::single<typenames::typenames> single_base;
				/// handles an input stream of signal coming to a node.
				class UNIVERSALIS__COMPILER__DYNAMIC_LINK single : public single_base
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

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
#else // !defined PSYCLE__EXPERIMENTAL
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

namespace psycle
{
	namespace engine
	{
		namespace ports
		{
			namespace inputs
			{
				/// handles an input stream of signal coming to a node.
				class UNIVERSALIS__COMPILER__DYNAMIC_LINK single : public input
				{
					friend class node;
	
					public:
						single(engine::node &, name_type const &, int const & channels = 0);
						virtual ~single() throw();
		
					public:	
						void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES disconnect_all();
	
					protected:
						void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES    connect_internal_side(output &);
						void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES disconnect_internal_side(output &);
	
					protected:
						void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES do_propagate_channels() throw(exception);
						void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES do_propagate_seconds_per_event();
		
					public:
						ports::output inline * const & output_port() const throw() { return output_port_; }
					private:
						ports::output        *         output_port_;
		
					public:
						void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES dump(std::ostream &, int const & tabulations = 0) const;
				};
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
#endif // !defined PSYCLE__EXPERIMENTAL
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

#include <universalis/compiler/dynamic_link/end.hpp>
