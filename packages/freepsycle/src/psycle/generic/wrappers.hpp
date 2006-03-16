// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2006 Johan Boule <bohan@jabber.org>
// Copyright (C) 2004-2006 Psycledelics http://psycle.pastnotecut.org

///\file
///\interface psycle::generic::wrappers
#pragma once
#include "generic.hpp"
#include <algorithm>
#include <functional>
namespace psycle
{
	namespace generic
	{
		namespace wrappers
		{
			///\internal
			namespace detail
			{
				template<typename Underlying_Wrapper>
				class underlying_wrapper_predicate
				{
					public:
						inline underlying_wrapper_predicate(typename Underlying_Wrapper::underlying_type & underlying) : underlying(underlying) {}
						inline bool operator()(Underlying_Wrapper const * const underlying_wrapper) const { return &underlying_wrapper->underlying() == &underlying; }
					private:
						typename Underlying_Wrapper::underlying_type & underlying;
				};
			}

			template<typename Typenames>
			class graph
			:
				public generic::graph<Typenames>,
				public universalis::compiler::cast::underlying_wrapper<typename Typenames::underlying::graph>
			{
				public:
					graph(typename graph::underlying_type & underlying) : graph::underlying_wrapper_type(underlying)
					{
						for(typename graph::underlying_type::const_iterator i(underlying.begin()) ; i != underlying.end() ; ++i) on_new_node(**i);
						for(typename graph::const_iterator i(this->begin()) ; i != this->end() ; ++i)
						{
							; ///\todo node connections
						}
						on_new_node_signal_connection          = underlying.         new_node_signal().connect(boost::bind(&graph::on_new_node         , this, _1    ));
					//	on_delete_node_signal_connection       = underlying.      delete_node_signal().connect(boost::bind(&graph::on_delete_node      , this, _1    ));
						on_new_connection_signal_connection    = underlying.   new_connection_signal().connect(boost::bind(&graph::on_new_connection   , this, _1, _2));
						on_delete_connection_signal_connection = underlying.delete_connection_signal().connect(boost::bind(&graph::on_delete_connection, this, _1, _2));
					}
					virtual ~graph() throw()
					{
						on_new_node_signal_connection.disconnect();
					//	on_delete_node_signal_connection.disconnect();
						on_new_connection_signal_connection.disconnect();
						on_delete_connection_signal_connection.disconnect();
					}
					
				///\name signal slots and connections
				///\{
					private:
						boost::signals::connection on_new_node_signal_connection;
						void on_new_node(typename Typenames::underlying::node & underlying_node)
						{
							new typename Typenames::node(*this, underlying_node);
						}

						//boost::signals::connection on_delete_node_signal_connection;
						//void on_delete_node(typename Typenames::underlying::node & underlying_node)
						//{
						//	this->delete_node_signal()(underlying_wrapper(underlying_node));
						//}

						boost::signals::connection    on_new_connection_signal_connection;
						void    on_new_connection(typename Typenames::underlying::ports::input & underlying_input_port, typename Typenames::underlying::ports::output & underlying_output_port)
						{
							this->new_connection_signal()(underlying_wrapper(underlying_input_port), underlying_wrapper(underlying_output_port));
						}

						boost::signals::connection on_delete_connection_signal_connection;
						void on_delete_connection(typename Typenames::underlying::ports::input & underlying_input_port, typename Typenames::underlying::ports::output & underlying_output_port)
						{
							this->delete_connection_signal()(underlying_wrapper(underlying_input_port), underlying_wrapper(underlying_output_port));
						}
				///\}

				private:
					///\return the the node wrapper corresponding to the given underlying node
					typename Typenames::node & underlying_wrapper(typename Typenames::underlying::node & underlying_node)
					{
						assert("the underlying node must belong to the underlying graph" && &underlying_node.parent() == &this->underlying());
						typename graph::const_iterator i(std::find_if(this->begin(), this->end(), detail::underlying_wrapper_predicate<typename Typenames::node>(underlying_node)));
						assert(i != this->end());
						return **i;
					}

					///\return the the output port wrapper corresponding to the given underlying output port
					typename Typenames::ports::output & underlying_wrapper(typename Typenames::underlying::ports::output & underlying_output_port)
					{
						return underlying_wrapper(underlying_output_port.parent()).underlying_wrapper(underlying_output_port);
					}

					///\return the the input port wrapper corresponding to the given underlying input port
					typename Typenames::ports::input & underlying_wrapper(typename Typenames::underlying::ports::input & underlying_input_port)
					{
						return underlying_wrapper(underlying_input_port.parent()).underlying_wrapper(underlying_input_port);
					}
			};
			
			template<typename Typenames>
			class node
			:
				public generic::node<Typenames>,
				public universalis::compiler::cast::underlying_wrapper<typename Typenames::underlying::node>
			{
				public:
					node(typename node::parent_type & parent, typename node::underlying_type & underlying)
					:
						generic::node<Typenames>(parent),
						node::underlying_wrapper_type(underlying)
					{
						if(loggers::trace()())
						{
							loggers::trace()("new node wrapper", UNIVERSALIS__COMPILER__LOCATION);
						}
						for(typename node::underlying_type::      output_ports_type::const_iterator i(underlying.      output_ports().begin()) ; i != underlying.      output_ports().end() ; ++i) on_new_output_port      (**i);
						for(typename node::underlying_type::single_input_ports_type::const_iterator i(underlying.single_input_ports().begin()) ; i != underlying.single_input_ports().end() ; ++i) on_new_single_input_port(**i);
						if(underlying.multiple_input_port()) on_new_multiple_input_port(*underlying.multiple_input_port());
						on_new_output_port_signal_connection         = underlying.        new_output_port_signal().connect(boost::bind(&node::on_new_output_port        , this, _1));
						on_new_single_input_port_signal_connection   = underlying.  new_single_input_port_signal().connect(boost::bind(&node::on_new_single_input_port  , this, _1));
						on_new_multiple_input_port_signal_connection = underlying.new_multiple_input_port_signal().connect(boost::bind(&node::on_new_multiple_input_port, this, _1));
						on_delete_signal_connection                  = underlying.                 delete_signal().connect(boost::bind(&node::on_delete                 , this, _1));
					}

					virtual ~node() throw()
					{
						on_new_output_port_signal_connection.disconnect();
						on_new_single_input_port_signal_connection.disconnect();
						on_new_multiple_input_port_signal_connection.disconnect();
						on_delete_signal_connection.disconnect();
					}
					
				///\name signal slots and connections
				///\{
					private:
						boost::signals::connection on_delete_signal_connection;
						void on_delete(typename node::underlying_type & underlying)
						{
							assert(&underlying == &this->underlying());
							delete this;
						}
						void static on_delete_(node & wrapper, typename node::underlying_type & underlying)
						{
							assert(&underlying == &wrapper.underlying());
							delete &wrapper;
						}

						boost::signals::connection on_new_output_port_signal_connection;
						void on_new_output_port(typename Typenames::underlying::ports::output & underlying_output_port)
						{
							if(loggers::trace()())
							{
								loggers::trace()("node wrapper on new output port underlying", UNIVERSALIS__COMPILER__LOCATION);
							}
							new typename Typenames::ports::output(*this, underlying_output_port);
						}

						boost::signals::connection on_new_single_input_port_signal_connection;
						void on_new_single_input_port(typename Typenames::underlying::ports::inputs::single & underlying_single_input_port)
						{
							if(loggers::trace()())
							{
								loggers::trace()("node wrapper on new single input port underlying", UNIVERSALIS__COMPILER__LOCATION);
							}
							new typename Typenames::ports::inputs::single(*this, underlying_single_input_port);
						}

						boost::signals::connection on_new_multiple_input_port_signal_connection;
						void on_new_multiple_input_port(typename Typenames::underlying::ports::inputs::multiple & underlying_multiple_input_port)
						{
							if(loggers::trace()())
							{
								loggers::trace()("node wrapper on new multiple input port underlying", UNIVERSALIS__COMPILER__LOCATION);
							}
							new typename Typenames::ports::inputs::multiple(*this, underlying_multiple_input_port);
						}
				///\}

				public://private:
					///\return the the output port wrapper corresponding to the given underlying output port
					typename Typenames::ports::output & underlying_wrapper(typename Typenames::underlying::ports::output & underlying_output_port)
					{
						assert("the underlying port must belong to the underlying node" && &underlying_output_port.parent() == &this->underlying());
						typename node::output_ports_type::const_iterator i(std::find_if(this->output_ports().begin(), this->output_ports().end(), detail::underlying_wrapper_predicate<typename Typenames::ports::output>(underlying_output_port)));
						assert(i != this->output_ports().end());
						return **i;
					}

					///\return the the input port wrapper corresponding to the given underlying input port
					typename Typenames::ports::input & underlying_wrapper(typename Typenames::underlying::ports::input & underlying_input_port)
					{
						assert("the underlying port must belong to the underlying node" && &underlying_input_port.parent() == &this->underlying());
						if(this->multiple_input_port() && &this->multiple_input_port()->underlying() == &underlying_input_port) return *this->multiple_input_port();
						typename node::single_input_ports_type::const_iterator i(std::find_if(this->single_input_ports().begin(), this->single_input_ports().end(), detail::underlying_wrapper_predicate<typename Typenames::ports::input>(underlying_input_port)));
						assert(i != this->single_input_ports().end());
						return **i;
					}
			};

			template<typename Typenames>
			class port
			:
				public generic::port<Typenames>,
				public universalis::compiler::cast::underlying_wrapper<typename Typenames::underlying::port>
			{
				public:
					port(typename port::parent_type & parent, typename port::underlying_type & underlying)
					:
						generic::port<Typenames>(parent),
						port::underlying_wrapper_type(underlying)
					{
					}
			};
			
			///\internal
			namespace detail
			{
				namespace cast
				{			
					template<typename Typenames, typename Derived_Underlying, typename Derived, template<typename = Typenames> class Base>
					class port
					:
						public Base<>,
						public universalis::compiler::cast::underlying_wrapper<Derived_Underlying, Derived, Base<> >
						//public universalis::compiler::cast::derived<Derived>
							// already derived from via Base<>
					{
						UNIVERSALIS__COMPILER__CAST__UNDERLYING_WRAPPER__DISAMBIGUATES(Derived_Underlying, port)
						protected:
							typedef port cast_type;
							#define constructor(_, count, __) \
								BOOST_PP_EXPR_IF(count, template<) BOOST_PP_ENUM_PARAMS(count, typename Xtra) BOOST_PP_EXPR_IF(count, >) \
								port(typename Base<>::parent_type & parent, Derived_Underlying & underlying BOOST_PP_ENUM_TRAILING_BINARY_PARAMS(count, Xtra, & xtra)) \
								: \
									Base<>(parent, underlying BOOST_PP_ENUM_TRAILING_PARAMS(count, xtra)) \
								{}
								BOOST_PP_REPEAT(PSYCLE__GENERIC__TEMPLATE_CONSTRUCTORS_LIMIT, constructor, ~)
							#undef constructor
					};
				}
			}
			
			namespace ports
			{
				template<typename Typenames>
				class output
				:
					public detail::cast::port
					<
						Typenames,
						typename Typenames::underlying::ports::output,
						typename Typenames::ports::output,
						generic::ports::output
					>
				{
					private:
						void init()
						{
							if(loggers::trace()())
							{
								loggers::trace()("new output port wrapper", UNIVERSALIS__COMPILER__LOCATION);
							}
						}
					public:
						#define constructor(_, count, __) \
							BOOST_PP_EXPR_IF(count, template<) BOOST_PP_ENUM_PARAMS(count, typename Xtra) BOOST_PP_EXPR_IF(count, >) \
							output(typename output::parent_type & parent, typename output::underlying_type & underlying BOOST_PP_ENUM_TRAILING_BINARY_PARAMS(count, Xtra, & xtra)) \
							: \
								output::cast_type(parent, underlying BOOST_PP_ENUM_TRAILING_PARAMS(count, xtra)) \
							{ \
								init(); \
							}
							BOOST_PP_REPEAT(PSYCLE__GENERIC__TEMPLATE_CONSTRUCTORS_LIMIT, constructor, ~)
						#undef constructor
				};

				template<typename Typenames>
				class input
				:
					public detail::cast::port
					<
						Typenames,
						typename Typenames::underlying::ports::input,
						typename Typenames::ports::input,
						generic::ports::input
					>
				{
					private:
						void init()
						{
							if(loggers::trace()())
							{
								loggers::trace()("new input port wrapper", UNIVERSALIS__COMPILER__LOCATION);
							}
						}
					public:
						#define constructor(_, count, __) \
							BOOST_PP_EXPR_IF(count, template<) BOOST_PP_ENUM_PARAMS(count, typename Xtra) BOOST_PP_EXPR_IF(count, >) \
							input(typename input::parent_type & parent, typename input::underlying_type & underlying BOOST_PP_ENUM_TRAILING_BINARY_PARAMS(count, Xtra, & xtra)) \
							: \
								input::cast_type(parent, underlying BOOST_PP_ENUM_TRAILING_PARAMS(count, xtra)) \
							{ \
								init(); \
							}
							BOOST_PP_REPEAT(PSYCLE__GENERIC__TEMPLATE_CONSTRUCTORS_LIMIT, constructor, ~)
						#undef constructor
				};
				
				namespace inputs
				{
					template<typename Typenames>
					class single
					:
						public detail::cast::port
						<
							Typenames,
							typename Typenames::underlying::ports::inputs::single,
							typename Typenames::ports::inputs::single,
							generic::ports::inputs::single
						>
					{
						private:
							void init()
							{
								if(loggers::trace()())
								{
									loggers::trace()("new single input port wrapper", UNIVERSALIS__COMPILER__LOCATION);
								}
							}
						public:
							#define constructor(_, count, __) \
								BOOST_PP_EXPR_IF(count, template<) BOOST_PP_ENUM_PARAMS(count, typename Xtra) BOOST_PP_EXPR_IF(count, >) \
								single(typename single::parent_type & parent, typename single::underlying_type & underlying BOOST_PP_ENUM_TRAILING_BINARY_PARAMS(count, Xtra, & xtra)) \
								: \
									single::cast_type(parent, underlying BOOST_PP_ENUM_TRAILING_PARAMS(count, xtra)) \
								{ \
									init(); \
								}
								BOOST_PP_REPEAT(PSYCLE__GENERIC__TEMPLATE_CONSTRUCTORS_LIMIT, constructor, ~)
							#undef constructor
					};

					template<typename Typenames>
					class multiple
					:
						public detail::cast::port
						<
							Typenames,
							typename Typenames::underlying::ports::inputs::multiple,
							typename Typenames::ports::inputs::multiple,
							generic::ports::inputs::multiple
						>
					{
						private:
							void init()
							{
								if(loggers::trace()())
								{
									loggers::trace()("new multiple input port wrapper", UNIVERSALIS__COMPILER__LOCATION);
								}
							}
						public:
							#define constructor(_, count, __) \
								BOOST_PP_EXPR_IF(count, template<) BOOST_PP_ENUM_PARAMS(count, typename Xtra) BOOST_PP_EXPR_IF(count, >) \
								multiple(typename multiple::parent_type & parent, typename multiple::underlying_type & underlying BOOST_PP_ENUM_TRAILING_BINARY_PARAMS(count, Xtra, & xtra)) \
								: \
									multiple::cast_type(parent, underlying BOOST_PP_ENUM_TRAILING_PARAMS(count, xtra)) \
								{ \
									init(); \
								}
								BOOST_PP_REPEAT(PSYCLE__GENERIC__TEMPLATE_CONSTRUCTORS_LIMIT, constructor, ~)
							#undef constructor
					};
				}
			}
		}
	}
}
