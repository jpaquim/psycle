// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2006 Johan Boule <bohan@jabber.org>
// Copyright (C) 2004-2006 Psycledelics http://psycle.pastnotecut.org

///\interface psycle::engine::ports::inputs::multiple
#pragma once
#include "../input.hpp"
#include <vector>
#if defined PSYCLE__EXPERIMENTAL
	#include <psycle/generic/generic.hpp>
#endif
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__ENGINE__PORTS__INPUTS__MULTIPLE
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
				typedef generic::ports::inputs::multiple<typenames::typenames> multiple_base;
				/// handles an input stream of signal coming to a node.
				class UNIVERSALIS__COMPILER__DYNAMIC_LINK multiple : public multiple_base
				{
					friend class node;
	
					protected:
						multiple(parent_type &, name_type const &, bool const & single_connection_is_identity_transform, int const & channels = 0); friend class generic_access;
					public:
						virtual ~multiple() throw();
		
					protected:
						void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES do_propagate_channels() throw(exception);
						void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES do_propagate_seconds_per_event();
		
					public:
						bool inline const & single_connection_is_identity_transform() const throw() { return single_connection_is_identity_transform_; }
					private:
						bool                single_connection_is_identity_transform_;
		
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
				class UNIVERSALIS__COMPILER__DYNAMIC_LINK multiple : public input
				{
					friend class node;
	
					public:
						multiple(engine::node &, name_type const &, bool const & single_connection_is_identity_transform, int const & channels = 0);
						virtual ~multiple() throw();
		
					public:	
						void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES disconnect_all();
	
					protected:
						void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES    connect_internal_side(output &);
						void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES disconnect_internal_side(output &);
	
					protected:
						void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES do_propagate_channels() throw(exception);
						void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES do_propagate_seconds_per_event();
		
					public:
						typedef std::vector<ports::output*> output_ports_type;
						output_ports_type inline const & output_ports() const throw() { return output_ports_; }
					private:
						output_ports_type                output_ports_;
						
					public:
						bool inline const & single_connection_is_identity_transform() const throw() { return single_connection_is_identity_transform_; }
					private:
						bool                single_connection_is_identity_transform_;
		
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
