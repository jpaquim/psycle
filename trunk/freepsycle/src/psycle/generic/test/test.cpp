// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2006 Johan Boule <bohan@jabber.org>
// copyright 2004-2006 Psycledelics http://psycle.pastnotecut.org

///\implementation psycle::engine::generic::test
#include PACKAGENERIC__PRE_COMPILED
#include PACKAGENERIC
#include <psycle/detail/project.private.hpp>
#include "test.hpp"

#if defined PSYCLE__EXPERIMENTAL__TEST

namespace psycle
{
	namespace generic
	{
		namespace test
		{
			namespace layer0
			{
				graph::graph() : graph_base() {}
				node::node(node::parent_type & parent) : node_base(parent) {}
				port::port(port::parent_type & parent) : port_base(parent) {}
				namespace ports
				{
					output::output(output::parent_type & parent) : output_base(parent) {}
					input::input(input::parent_type & parent) : input_base(parent) {}
					namespace inputs
					{
						single::single(single::parent_type & parent) : single_base(parent) {}
						multiple::multiple(multiple::parent_type & parent) : multiple_base(parent) {}
					}
				}

				namespace
				{
					class test
					{
						test()
						{
							//graph g;
							graph & g(graph::create());
							//node n(g);
							node & n(node::create(g));
							//ports::output op(n);
							ports::output & op(ports::output::create(n));
							//ports::inputs::single sip(n);
							ports::inputs::single & sip(ports::inputs::single::create(n));
							//ports::inputs::multiple mip(n);
							ports::inputs::multiple & mip(ports::inputs::multiple::create(n));
						}
					};
				}
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
				graph::graph(graph::underlying_type & underlying) : graph_base(underlying) {}
				node::node(node::parent_type & parent, node::underlying_type & underlying) : node_base(parent, underlying) {}
				port::port(port::parent_type & parent, port::underlying_type & underlying) : port_base(parent, underlying) {}
				namespace ports
				{
					output::output(output::parent_type & parent, output::underlying_type & underlying) : output_base(parent, underlying) {}
					input::input(input::parent_type & parent, input::underlying_type & underlying) : input_base(parent, underlying) {}
					namespace inputs
					{
						single::single(single::parent_type & parent, single::underlying_type & underlying) : single_base(parent, underlying) {}
						multiple::multiple(multiple::parent_type & parent, multiple::underlying_type & underlying) : multiple_base(parent, underlying) {}
					}
				}
				
				namespace
				{
					class test
					{
						test()
						{
							//graph::underlying_type g;
							graph::underlying_type & g(graph::underlying_type::create());
							//graph gg(g);
							graph & gg(graph::create(g));
							//node::underlying_type n(g);
							node::underlying_type & n(node::underlying_type::create(g));
							//node nn(gg, n); // implicit!
							node & nn(node::create(gg, n)); // implicit!
						}
					};
				}
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
				graph::graph(graph::underlying_type & underlying, graph::name_type const & name) : graph_base(underlying), named(name)
				{
					new_node_signal().connect(boost::bind(&graph::on_new_node, this, _1));
				}

				void graph::on_new_node(typenames::node & node)
				{
					node.name("unnamed (graph::on_new_node)");
				}
				
				node::node(node::parent_type & parent, node::underlying_type & underlying, node::name_type const & name) : node_base(parent, underlying), named(name)
				{
					new_output_port_signal()        .connect(boost::bind(&node::on_new_output_port        , this, _1));
					new_single_input_port_signal()  .connect(boost::bind(&node::on_new_single_input_port  , this, _1));
					new_multiple_input_port_signal().connect(boost::bind(&node::on_new_multiple_input_port, this, _1));
				}
				
				void node::on_new_output_port(typenames::ports::output & output_port)
				{
					output_port.name("unnamed (node::on_new_output_port)");
				}
				void node::on_new_single_input_port(typenames::ports::inputs::single & single_input_port)
				{
					single_input_port.name("unnamed (node::on_new_single_input_port)");
				}
				void node::on_new_multiple_input_port(typenames::ports::inputs::multiple & multiple_input_port)
				{
					multiple_input_port.name("unamed (node::on_new_multiple_input_port)");
				}
				
				port::port(port::parent_type & parent, port::underlying_type & underlying, port::name_type const & name) : port_base(parent, underlying), named(name) {}
				
				namespace ports
				{
					input::input(input::parent_type & parent, input::underlying_type & underlying, input::name_type const & name) : input_base(parent, underlying, name) {}
					output::output(output::parent_type & parent, output::underlying_type & underlying, output::name_type const & name) : output_base(parent, underlying, name) {}
					namespace inputs
					{
						single::single(single::parent_type & parent, single::underlying_type & underlying, single::name_type const & name) : single_base(parent, underlying, name) {}
						multiple::multiple(multiple::parent_type & parent, multiple::underlying_type & underlying, multiple::name_type const & name) : multiple_base(parent, underlying, name) {}
					}
				}
				
				namespace
				{
					class test
					{
						test()
						{
						//	typenames::underlying::underlying::graph g;
							typenames::underlying::underlying::graph & g(typenames::underlying::underlying::graph::create());
						//	typenames::underlying::            graph   gg(g);
							typenames::underlying::            graph & gg(typenames::underlying::            graph::create(g));
						//	typenames::                        graph   ggg(gg, "g");
							typenames::                        graph & ggg(typenames::                        graph::create(gg, "g"));
						//	node::underlying_type::underlying_type   n(g);
							node::underlying_type::underlying_type & n(node::underlying_type::underlying_type::create(g));
						//	node::underlying_type                    nn(gg, n); // implicit!
							node::underlying_type                  & nn(node::underlying_type::create(gg, n)); // implicit!
						//	node                                     nnn(ggg, nn, "n"); // implicit!
							node                                   & nnn(node::create(ggg, nn, "n")); // implicit!
							
							typedef node::underlying_type xnode_base;
							class xnode : public xnode_base
							{
								protected:
									xnode(parent_type & parent, underlying_type & underlying) : xnode_base(parent, underlying)
									{
										typenames::underlying::underlying::ports::output::create(*this);
										typenames::underlying::underlying::ports::inputs::single::create(*this);
										typenames::underlying::underlying::ports::inputs::multiple::create(*this);
									}
							};
							
						//	xnode   xx(ggg.underlying(), n);
							xnode & xx(xnode::create_<xnode>(ggg.underlying(), n));
						//	node   xxx(ggg, xx, "x"); // implicit!
							node & xxx(node::create(ggg, xx, "x")); // implicit!
						}
					};
				}
			}
		}
	}
}

#endif // defined PSYCLE__EXPERIMENTAL__TEST
