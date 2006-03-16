// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2006 Johan Boule <bohan@jabber.org>
// Copyright (C) 2004-2006 Psycledelics http://psycle.pastnotecut.org

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
							graph g;
							node n(g);
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
				graph::graph(graph::underlying_type & underlying) : graph_base(underlying)
				{
					underlying.new_node_signal().connect(boost::bind(&graph::on_new_node, this, _1));
				}

				void graph::on_new_node(typenames::underlying::node & underlying_node)
				{
					this->new_node_signal()(*new typenames::node(*this, underlying_node));
				}
				
				node::node(node::parent_type & parent, node::underlying_type & underlying) : node_base(parent, underlying)
				{
					underlying.        new_output_port_signal().connect(boost::bind(&node::on_new_output_port        , this, _1));
					underlying.  new_single_input_port_signal().connect(boost::bind(&node::on_new_single_input_port  , this, _1));
					underlying.new_multiple_input_port_signal().connect(boost::bind(&node::on_new_multiple_input_port, this, _1));
				}
						
				void node::on_new_output_port(typenames::underlying::ports::output & underlying_output_port)
				{
					// ...
				}
				void node::on_new_single_input_port(typenames::underlying::ports::inputs::single & underlying_single_input_port)
				{
					// ...
				}
				void node::on_new_multiple_input_port(typenames::underlying::ports::inputs::multiple & underlying_multiple_input_port)
				{
					// ...
				}
				
				port::port(port::parent_type & parent, port::underlying_type & underlying) : port_base(parent, underlying) {}
				
				namespace ports
				{
					output::output(output::parent_type & parent, output::underlying_type & underlying) : output_base(parent, underlying)
					{
						// ...
					}
					
					input::input(input::parent_type & parent, input::underlying_type & underlying) : input_base(parent, underlying) {}
					
					namespace inputs
					{
						single::single(single::parent_type & parent, single::underlying_type & underlying) : single_base(parent, underlying)
						{
							// ...
						}
						
						multiple::multiple(multiple::parent_type & parent, multiple::underlying_type & underlying) : multiple_base(parent, underlying)
						{
							// ...
						}
					}
				}
				
				namespace
				{
					class test
					{
						test()
						{
							graph::underlying_type g;
							graph gg(g);
							node::underlying_type n(g);
							node nn(gg, n);
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
					underlying.new_node_signal().connect(boost::bind(&graph::on_new_node, this, _1));
				}

				void graph::on_new_node(typenames::underlying::node & underlying_node)
				{
					this->new_node_signal()(*new typenames::node(*this, underlying_node));
				}
				
				node::node(node::parent_type & parent, node::underlying_type & underlying, node::name_type const & name) : node_base(parent, underlying), named(name)
				{
					underlying.        new_output_port_signal().connect(boost::bind(&node::on_new_output_port        , this, _1));
					underlying.  new_single_input_port_signal().connect(boost::bind(&node::on_new_single_input_port  , this, _1));
					underlying.new_multiple_input_port_signal().connect(boost::bind(&node::on_new_multiple_input_port, this, _1));
				}
				
				void node::on_new_output_port(typenames::underlying::ports::output & underlying_output_port)
				{
					// ...
				}
				void node::on_new_single_input_port(typenames::underlying::ports::inputs::single & underlying_single_input_port)
				{
					// ...
				}
				void node::on_new_multiple_input_port(typenames::underlying::ports::inputs::multiple & underlying_multiple_input_port)
				{
					// ...
				}
				
				port::port(port::parent_type & parent, port::underlying_type & underlying, port::name_type const & name) : port_base(parent, underlying), named(name) {}
				
				namespace ports
				{
					input::input(input::parent_type & parent, input::underlying_type & underlying, input::name_type const & name) : input_base(parent, underlying, name) {}

					output::output(output::parent_type & parent, output::underlying_type & underlying, output::name_type const & name) : output_base(parent, underlying, name)
					{
						// ...
					}
					
					namespace inputs
					{
						single::single(single::parent_type & parent, single::underlying_type & underlying, single::name_type const & name) : single_base(parent, underlying, name)
						{
							// ...
						}
						
						multiple::multiple(multiple::parent_type & parent, multiple::underlying_type & underlying, multiple::name_type const & name) : multiple_base(parent, underlying, name)
						{
							// ...
						}
					}
				}
				
				namespace
				{
					class test
					{
						test()
						{
							typenames::underlying::underlying::graph g;
							typenames::underlying::            graph gg(g);
							typenames::                        graph ggg(gg, "g");
							node::underlying_type::underlying_type n(g);
							node::underlying_type                  nn(gg, n);
							node                                   nnn(ggg, nn, "n");
							
							typedef node::underlying_type xnode_base;
							class xnode : public xnode_base
							{
								public:
									xnode(parent_type & parent, underlying_type & underlying) : xnode_base(parent, underlying)
									{
										new typenames::underlying::underlying::ports::output(*this);
										new typenames::underlying::underlying::ports::inputs::single(*this);
										new typenames::underlying::underlying::ports::inputs::multiple(*this);
									}
							};
							
							xnode xx(ggg.underlying(), n);
							node xxx(ggg, xx, "x");
						}
					};
				}
			}
		}
	}
}

#endif // defined PSYCLE__EXPERIMENTAL__TEST
