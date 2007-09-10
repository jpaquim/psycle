// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2000-2007 psycledelics http://psycle.sourceforge.net

///\interface psycle::engine::ports::inputs::multiple
#pragma once
#include "../input.hpp"
#include <vector>
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__ENGINE__PORTS__INPUTS__MULTIPLE
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
				class UNIVERSALIS__COMPILER__DYNAMIC_LINK multiple : public typenames::typenames::bases::ports::inputs::multiple
				{
					friend class node;
	
					protected: friend class factory;
						multiple(parent_type &, name_type const &, bool const & single_connection_is_identity_transform, int const & channels = 0);
						virtual ~multiple();
		
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
#include <universalis/compiler/dynamic_link/end.hpp>
