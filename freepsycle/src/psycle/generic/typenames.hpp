// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2006 Johan Boule <bohan@jabber.org>
// copyright 2004-2006 Psycledelics http://psycle.pastnotecut.org

///\interface psycle::engine::generic::typenames
#pragma once
#include <psycle/detail/project.hpp>
#if 0 // checks disabled here because types are incomplete at the time template classes are instanciated.
	#include <boost/static_assert.hpp>
	#include <boost/type_traits/is_base_and_derived.hpp>
#endif
namespace psycle
{
	namespace generic
	{
		template
		<
			typename               Graph,
			typename                Node,
			typename                Port,
			typename         Output_Port,
			typename          Input_Port,
			typename   Single_Input_Port,
			typename Multiple_Input_Port,
			typename Underlying_Typenames = void
		>
		class typenames : public typenames<Graph, Node, Port, Output_Port, Input_Port, Single_Input_Port, Multiple_Input_Port>
		{
			public:
				typedef Underlying_Typenames underlying;
		};

		template
		<
			typename               Graph,
			typename                Node,
			typename                Port,
			typename         Output_Port,
			typename          Input_Port,
			typename   Single_Input_Port,
			typename Multiple_Input_Port
		>
		class typenames<Graph, Node, Port, Output_Port, Input_Port, Single_Input_Port, Multiple_Input_Port>
		{
			public:
				typedef Graph graph;
				typedef Node node;
				typedef Port port;
				class ports
				{
					public:
						typedef Output_Port output;
						typedef  Input_Port  input;
						class inputs
						{
							public:
								typedef   Single_Input_Port   single;
								typedef Multiple_Input_Port multiple;
						};
				};
			#if 0 // checks disabled here because types are incomplete at the time this template class is instanciated.
				private:
					BOOST_STATIC_ASSERT((boost::is_base_and_derived<child_of<graph>, node                   >::value));
					BOOST_STATIC_ASSERT((boost::is_base_and_derived<child_of< node>, port                   >::value));
					BOOST_STATIC_ASSERT((boost::is_base_and_derived<port           , ports::output          >::value));
					BOOST_STATIC_ASSERT((boost::is_base_and_derived<port           , ports::input           >::value));
					BOOST_STATIC_ASSERT((boost::is_base_and_derived<ports::input   , ports::inputs::single  >::value));
					BOOST_STATIC_ASSERT((boost::is_base_and_derived<ports::input   , ports::inputs::multiple>::value));
			#endif
		};
	}
}
