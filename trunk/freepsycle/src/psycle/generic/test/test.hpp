// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2006 Johan Boule <bohan@jabber.org>
// copyright 2004-2006 Psycledelics http://psycle.pastnotecut.org

///\interface psycle::engine::generic::test
#pragma once

#if defined PSYCLE__EXPERIMENTAL__TEST

#include "../generic.hpp" // test::layer0
#include "../wrappers.hpp" // test::layer1
#include <psycle/engine/named.hpp> // test::layer2

#define UNIVERSALIS__COMPILER__DYNAMIC_LINK PSYCLE__GENERIC__TEST
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace psycle
{
	namespace generic
	{
		namespace test
		{
			namespace layer0
			{
				class graph;
				class node;
				class port;
				namespace ports
				{
					class output;
					class input;
					namespace inputs
					{
						class single;
						class multiple;
					}
				}

				class typenames : public generic::typenames<graph, node, port, ports::output, ports::input, ports::inputs::single, ports::inputs::multiple> {};

				typedef generic::graph<typenames> graph_base;
				class UNIVERSALIS__COMPILER__DYNAMIC_LINK graph : public graph_base { protected: graph(); friend class generic_access; };
				
				typedef generic::port<typenames> port_base;
				class UNIVERSALIS__COMPILER__DYNAMIC_LINK port : public port_base { protected: port(parent_type & parent); };
				
				namespace ports
				{
					typedef generic::ports::output<typenames> output_base;
					class UNIVERSALIS__COMPILER__DYNAMIC_LINK output : public output_base { protected: output(parent_type & parent); friend class generic_access; };
					
					typedef generic::ports::input<typenames> input_base;
					class UNIVERSALIS__COMPILER__DYNAMIC_LINK input : public input_base { protected: input(parent_type & parent); friend class generic_access; };
					
					namespace inputs
					{
						typedef generic::ports::inputs::single<typenames> single_base;
						class UNIVERSALIS__COMPILER__DYNAMIC_LINK single : public single_base { protected: single(parent_type & parent); friend class generic_access; };
						
						typedef generic::ports::inputs::multiple<typenames> multiple_base;
						class UNIVERSALIS__COMPILER__DYNAMIC_LINK multiple : public multiple_base { protected: multiple(parent_type & parent); friend class generic_access; };
					}
				}

				typedef generic::node<typenames> node_base;
				class UNIVERSALIS__COMPILER__DYNAMIC_LINK node : public node_base { protected: node(parent_type & parent); friend class generic_access; };
			}
		}
	}
}

namespace psycle
{
	namespace generic
	{
		namespace test
		{
			namespace layer1
			{
				namespace underlying = layer0;
				
				class graph;
				class node;
				class port;
				namespace ports
				{
					class output;
					class input;
					namespace inputs
					{
						class single;
						class multiple;
					}
				}

				class typenames : public generic::typenames<graph, node, port, ports::output, ports::input, ports::inputs::single, ports::inputs::multiple, underlying::typenames> {};

				typedef generic::wrappers::graph<typenames> graph_base;
				class UNIVERSALIS__COMPILER__DYNAMIC_LINK graph : public graph_base { protected: graph(underlying_type &); friend class generic_access; };

				typedef generic::wrappers::port<typenames> port_base;
				class UNIVERSALIS__COMPILER__DYNAMIC_LINK port : public port_base { protected: port(parent_type &, underlying_type &); };
				
				namespace ports
				{
					typedef generic::wrappers::ports::output<typenames> output_base;
					class UNIVERSALIS__COMPILER__DYNAMIC_LINK output : public output_base { protected: output(parent_type &, underlying_type &); friend class generic_access; };

					typedef generic::wrappers::ports::input<typenames> input_base;
					class UNIVERSALIS__COMPILER__DYNAMIC_LINK input : public input_base { protected: input(parent_type &, underlying_type &); friend class generic_access; };

					namespace inputs
					{
						typedef generic::wrappers::ports::inputs::single<typenames> single_base;
						class UNIVERSALIS__COMPILER__DYNAMIC_LINK single : public single_base { protected: single(parent_type &, underlying_type &); friend class generic_access; };

						typedef generic::wrappers::ports::inputs::multiple<typenames> multiple_base;
						class UNIVERSALIS__COMPILER__DYNAMIC_LINK multiple : public multiple_base { protected: multiple(parent_type &, underlying_type &); friend class generic_access; };
					}
				}

				typedef generic::wrappers::node<typenames> node_base;
				class UNIVERSALIS__COMPILER__DYNAMIC_LINK node : public node_base { protected: node(parent_type &, underlying_type &); friend class generic_access; };
			}
		}
	}
}

namespace psycle
{
	namespace generic
	{
		namespace test
		{
			namespace layer2
			{
				using engine::named;
				namespace underlying = layer1;

				class graph;
				class node;
				class port;
				namespace ports
				{
					class output;
					class input;
					namespace inputs
					{
						class single;
						class multiple;
					}
				}
				
				class typenames : public generic::typenames<graph, node, port, ports::output, ports::input, ports::inputs::single, ports::inputs::multiple, underlying::typenames> {};

				typedef generic::wrappers::graph<typenames> graph_base;
				class UNIVERSALIS__COMPILER__DYNAMIC_LINK graph : public graph_base, public named
				{
					protected:
						graph(underlying_type &, name_type const & = "unnamed"); friend class generic_access;

					///\name signal slots
					///\{
						private:
							void on_new_node(typenames::node &);
					///\}
				};

				typedef generic::wrappers::port<typenames> port_base;
				class UNIVERSALIS__COMPILER__DYNAMIC_LINK port : public port_base, public named
				{
					protected:
						port(parent_type &, underlying_type &, name_type const & = "unnamed");
				};

				namespace ports
				{
					typedef generic::wrappers::ports::output<typenames> output_base;
					class UNIVERSALIS__COMPILER__DYNAMIC_LINK output : public output_base
					{
						protected:
							output(parent_type &, underlying_type &, name_type const & = "unnamed"); friend class generic_access;
					};

					typedef generic::wrappers::ports::input<typenames> input_base;
					class UNIVERSALIS__COMPILER__DYNAMIC_LINK input : public input_base
					{
						protected:
							input(parent_type &, underlying_type &, name_type const & = "unnamed"); friend class generic_access;
					};

					namespace inputs
					{
						typedef generic::wrappers::ports::inputs::single<typenames> single_base;
						class UNIVERSALIS__COMPILER__DYNAMIC_LINK single : public single_base
						{
							protected:
								single(parent_type &, underlying_type &, name_type const & = "unnamed"); friend class generic_access;
						};

						typedef generic::wrappers::ports::inputs::multiple<typenames> multiple_base;
						class UNIVERSALIS__COMPILER__DYNAMIC_LINK multiple : public multiple_base
						{
							protected:
								multiple(parent_type &, underlying_type &, name_type const & = "unnamed"); friend class generic_access;
						};
					}
				}

				typedef generic::wrappers::node<typenames> node_base;
				class UNIVERSALIS__COMPILER__DYNAMIC_LINK node : public node_base, public named
				{
					protected:
						node(parent_type &, underlying_type &, name_type const & = "unnamed"); friend class generic_access;
						
					///\name signal slots
					///\{					
						private:
							void on_new_output_port(typenames::ports::output &);
							void on_new_single_input_port(typenames::ports::inputs::single &);
							void on_new_multiple_input_port(typenames::ports::inputs::multiple &);
					///\}
				};
			}
		}
	}
}
#include <universalis/compiler/dynamic_link/end.hpp>
#endif // defined PSYCLE__EXPERIMENTAL__TEST
