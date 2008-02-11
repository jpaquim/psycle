// -*- mode:c++; indent-tabs-mode:t -*-
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2005-2007 johan boule <bohan@jabber.org>
// copyright 2005-2007 psycledelics http://psycle.sourceforge.net

///\interface psycle::engine::generic::test
#pragma once
#if defined PSYCLE__TEST
#include "../generic.hpp" // test::layer0
#include "../wrappers.hpp" // test::layer1
#include <psycle/engine/named.hpp> // test::layer2
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK PSYCLE__GENERIC__TEST
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace psycle { namespace generic { namespace test {
	namespace layer0 {
		class graph;
		class node;
		class port;
		namespace ports {
			class output;
			class input;
			namespace inputs {
				class single;
				class multiple;
			}
		}
		class typenames : public generic::typenames<graph, node, port, ports::output, ports::input, ports::inputs::single, ports::inputs::multiple> {};
		class UNIVERSALIS__COMPILER__DYNAMIC_LINK graph : public typenames::bases::graph { protected: graph(); friend class generic_access; };
		class UNIVERSALIS__COMPILER__DYNAMIC_LINK port  : public typenames::bases::port  { protected: port(parent_type & parent); };
		namespace ports {
			class UNIVERSALIS__COMPILER__DYNAMIC_LINK output : public typenames::bases::ports::output { protected: output(parent_type & parent); friend class generic_access; };
			class UNIVERSALIS__COMPILER__DYNAMIC_LINK input  : public typenames::bases::ports:: input { protected: input(parent_type & parent); friend class generic_access; };
			namespace inputs {
				class UNIVERSALIS__COMPILER__DYNAMIC_LINK   single : public typenames::bases::ports::inputs::  single { protected: single(parent_type & parent); friend class generic_access; };
				class UNIVERSALIS__COMPILER__DYNAMIC_LINK multiple : public typenames::bases::ports::inputs::multiple { protected: multiple(parent_type & parent); friend class generic_access; };
			}
		}
		class UNIVERSALIS__COMPILER__DYNAMIC_LINK node : public typenames::bases::node { protected: node(parent_type & parent); friend class generic_access; };
	}
	namespace layer1 {
		namespace underlying = layer0;
		class graph;
		class node;
		class port;
		namespace ports {
			class output;
			class input;
			namespace inputs {
				class single;
				class multiple;
			}
		}
		class typenames : public generic::typenames<graph, node, port, ports::output, ports::input, ports::inputs::single, ports::inputs::multiple, underlying::typenames> {};
		class UNIVERSALIS__COMPILER__DYNAMIC_LINK graph : public typenames::bases::graph { protected: graph(underlying_type &); friend class generic_access; };
		class UNIVERSALIS__COMPILER__DYNAMIC_LINK port  : public typenames::bases::port  { protected: port(parent_type &, underlying_type &); };
		namespace ports {
			class UNIVERSALIS__COMPILER__DYNAMIC_LINK output : public typenames::bases::ports::output { protected: output(parent_type &, underlying_type &); friend class generic_access; };
			class UNIVERSALIS__COMPILER__DYNAMIC_LINK  input : public typenames::bases::ports:: input { protected:  input(parent_type &, underlying_type &); friend class generic_access; };
			namespace inputs {
				class UNIVERSALIS__COMPILER__DYNAMIC_LINK   single : public typenames::bases::ports::inputs::  single { protected:   single(parent_type &, underlying_type &); friend class generic_access; };
				class UNIVERSALIS__COMPILER__DYNAMIC_LINK multiple : public typenames::bases::ports::inputs::multiple { protected: multiple(parent_type &, underlying_type &); friend class generic_access; };
			}
		}
		class UNIVERSALIS__COMPILER__DYNAMIC_LINK node : public typenames::bases::node { protected: node(parent_type &, underlying_type &); friend class generic_access; };
	}
	namespace layer2 {
		using engine::named;
		namespace underlying = layer1;
		class graph;
		class node;
		class port;
		namespace ports {
			class output;
			class input;
			namespace inputs {
				class single;
				class multiple;
			}
		}
		class typenames : public generic::typenames<graph, node, port, ports::output, ports::input, ports::inputs::single, ports::inputs::multiple, underlying::typenames> {};
		class UNIVERSALIS__COMPILER__DYNAMIC_LINK graph : public typenames::bases::graph, public named {
			protected:
				graph(underlying_type &, name_type const & = "unnamed"); friend class generic_access;

			///\name signal slots
			///\{
				private:
					void on_new_node(typenames::node &);
			///\}
		};
		class UNIVERSALIS__COMPILER__DYNAMIC_LINK port : public typenames::bases::port, public named {
			protected:
				port(parent_type &, underlying_type &, name_type const & = "unnamed");
		};
		namespace ports {
			class UNIVERSALIS__COMPILER__DYNAMIC_LINK output : public typenames::bases::ports::output {
				protected:
					output(parent_type &, underlying_type &, name_type const & = "unnamed"); friend class generic_access;
			};
			class UNIVERSALIS__COMPILER__DYNAMIC_LINK input : public typenames::bases::ports::input {
				protected:
					input(parent_type &, underlying_type &, name_type const & = "unnamed"); friend class generic_access;
			};
			namespace inputs {
				class UNIVERSALIS__COMPILER__DYNAMIC_LINK single : public typenames::bases::ports::inputs::single {
					protected:
						single(parent_type &, underlying_type &, name_type const & = "unnamed"); friend class generic_access;
				};
				class UNIVERSALIS__COMPILER__DYNAMIC_LINK multiple : public typenames::bases::ports::inputs::multiple {
					protected:
						multiple(parent_type &, underlying_type &, name_type const & = "unnamed"); friend class generic_access;
				};
			}
		}
		class UNIVERSALIS__COMPILER__DYNAMIC_LINK node : public typenames::bases::node, public named {
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
}}}
#include <universalis/compiler/dynamic_link/end.hpp>
#endif // defined PSYCLE__TEST
