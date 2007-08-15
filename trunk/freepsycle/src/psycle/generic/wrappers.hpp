// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2002-2007 johan boule <bohan@jabber.org>
// copyright 2002-2007 psycledelics http://psycle.sourceforge.net

///\interface psycle::generic::wrappers
#pragma once
#include "generic.hpp"
#include <algorithm>
namespace psycle { namespace generic { namespace wrappers {

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

	/***********************************************************************/

	template<typename Typenames>
	class graph
	:
		public basic::graph<Typenames>,
		public universalis::compiler::cast::underlying_wrapper<typename Typenames::underlying::graph>
	{
		protected:
			typedef graph graph_type;

			graph(typename graph::underlying_type & underlying) : graph::underlying_wrapper_type(underlying)
			{
				on_new_node_signal_connection          = underlying.         new_node_signal().connect(boost::bind(&graph::on_new_node         , this, _1    ));
				on_new_connection_signal_connection    = underlying.   new_connection_signal().connect(boost::bind(&graph::on_new_connection   , this, _1, _2));
				on_delete_connection_signal_connection = underlying.delete_connection_signal().connect(boost::bind(&graph::on_delete_connection, this, _1, _2));
			}

			void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES after_construction()
			{
				basic::graph<Typenames>::after_construction();
				for(typename graph::underlying_type::const_iterator i(this->underlying().begin()) ; i != this->underlying().end() ; ++i) on_new_node(**i);
				for(typename graph::const_iterator i(this->begin()) ; i != this->end() ; ++i)
				{
					loggers::trace()("@@@@@@@@@@@@@@@@@@@@@@ generic wrappers graph::init node");
					typename Typenames::node & node(**i);
					for(typename Typenames::node::output_ports_type::const_iterator i(node.output_ports().begin()) ; i != node.output_ports().end() ; ++i)
					{
						loggers::trace()("@@@@@@@@@@@@@@@@@@@@@@ generic wrappers graph::init port");
						typename Typenames::ports::output & output_port(**i);
						for(typename Typenames::ports::output::underlying_type::input_ports_type::const_iterator i(output_port.underlying().input_ports().begin()) ; i != output_port.underlying().input_ports().end() ; ++i)
						{
							typename Typenames::ports::input & input_port(underlying_wrapper(**i));
							loggers::trace()("@@@@@@@@@@@@@@@@@@@@@@ generic wrappers graph::init connection");
							output_port.connect(input_port);
						}
					}
				}
			}

			virtual ~graph()
			{
				on_new_node_signal_connection.disconnect();
				on_new_connection_signal_connection.disconnect();
				on_delete_connection_signal_connection.disconnect();
			}
			
		///\name signal slots and connections
		///\{
			private:
				boost::signals::connection on_new_node_signal_connection;
				void on_new_node(typename Typenames::underlying::node & underlying_node)
				{
					Typenames::node::create(static_cast<typename Typenames::graph &>(*this), underlying_node);
				}

				//boost::signals::connection on_delete_node_signal_connection;
				//void on_delete_node(typename Typenames::underlying::node & underlying_node)
				//{
				//this->delete_node_signal()(underlying_wrapper(underlying_node));
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

		public:
			///\return the the node wrapper corresponding to the given underlying node
			typename Typenames::node & underlying_wrapper(typename Typenames::underlying::node & underlying_node)
			{
				assert("the underlying node must belong to the underlying graph" && &underlying_node.parent() == &this->underlying());
				typename graph::const_iterator i(std::find_if(this->begin(), this->end(), detail::underlying_wrapper_predicate<typename Typenames::node>(underlying_node))); // or composition with boost::bind
				assert(i != this->end());
				return **i;
			}

		private:
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
	
	/***********************************************************************/

	template<typename Typenames>
	class node
	:
		public basic::node<Typenames>,
		public universalis::compiler::cast::underlying_wrapper<typename Typenames::underlying::node>
	{
		protected:
			typedef node node_type;

			node(typename node::parent_type & parent, typename node::underlying_type & underlying)
			:
				basic::node<Typenames>(parent),
				node::underlying_wrapper_type(underlying)
			{
				on_new_output_port_signal_connection         = underlying.        new_output_port_signal().connect(boost::bind(&node::on_new_output_port        , this, _1));
				on_new_single_input_port_signal_connection   = underlying.  new_single_input_port_signal().connect(boost::bind(&node::on_new_single_input_port  , this, _1));
				on_new_multiple_input_port_signal_connection = underlying.new_multiple_input_port_signal().connect(boost::bind(&node::on_new_multiple_input_port, this, _1));
				on_delete_signal_connection                  = underlying.                 delete_signal().connect(boost::bind(&node::on_delete                 , this, _1));
			}

			void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES after_construction()
			{
				basic::node<Typenames>::after_construction();
				for(typename node::underlying_type::      output_ports_type::const_iterator i(this->underlying().      output_ports().begin()) ; i != this->underlying().      output_ports().end() ; ++i) on_new_output_port      (**i);
				for(typename node::underlying_type::single_input_ports_type::const_iterator i(this->underlying().single_input_ports().begin()) ; i != this->underlying().single_input_ports().end() ; ++i) on_new_single_input_port(**i);
				if(this->underlying().multiple_input_port()) on_new_multiple_input_port(*this->underlying().multiple_input_port());
			}

			virtual ~node()
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
					Typenames::ports::output::create(*this, underlying_output_port);
				}

				boost::signals::connection on_new_single_input_port_signal_connection;
				void on_new_single_input_port(typename Typenames::underlying::ports::inputs::single & underlying_single_input_port)
				{
					Typenames::ports::inputs::single::create(*this, underlying_single_input_port);
				}

				boost::signals::connection on_new_multiple_input_port_signal_connection;
				void on_new_multiple_input_port(typename Typenames::underlying::ports::inputs::multiple & underlying_multiple_input_port)
				{
					Typenames::ports::inputs::multiple::create(*this, underlying_multiple_input_port);
				}
		///\}

		public: //private:
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

	/***********************************************************************/

	template<typename Typenames>
	class port
	:
		public basic::port<Typenames>,
		public universalis::compiler::cast::underlying_wrapper<typename Typenames::underlying::port>
	{
		protected:
			typedef port port_type;

			port(typename port::parent_type & parent, typename port::underlying_type & underlying)
			:
				basic::port<Typenames>(parent),
				port::underlying_wrapper_type(underlying)
			{}
	};
	
	namespace ports
	{
		/***********************************************************************/
		
		template<typename Typenames>
		class output : public universalis::compiler::cast::underlying_wrapper< typename Typenames::underlying::ports::output, basic::ports::output<Typenames> >
		{
			protected:
				typedef output output_type;

				#define constructor(_, count, __) \
					BOOST_PP_EXPR_IF(count, template<) BOOST_PP_ENUM_PARAMS(count, typename Xtra) BOOST_PP_EXPR_IF(count, >) \
					output(typename output::parent_type & parent, typename output::underlying_type & underlying BOOST_PP_ENUM_TRAILING_BINARY_PARAMS(count, Xtra, & xtra)) \
					: output::underlying_wrapper_type(parent, underlying BOOST_PP_ENUM_TRAILING_PARAMS(count, xtra)) {}
					BOOST_PP_REPEAT(PSYCLE__GENERIC__TEMPLATE_CONSTRUCTORS__ARITY, constructor, ~)
				#undef constructor
		};

		/***********************************************************************/

		template<typename Typenames>
		class input : public universalis::compiler::cast::underlying_wrapper< typename Typenames::underlying::ports::input, basic::ports::input<Typenames> >
		{
			protected:
				typedef input input_type;

				#define constructor(_, count, __) \
					BOOST_PP_EXPR_IF(count, template<) BOOST_PP_ENUM_PARAMS(count, typename Xtra) BOOST_PP_EXPR_IF(count, >) \
					input(typename input::parent_type & parent, typename input::underlying_type & underlying BOOST_PP_ENUM_TRAILING_BINARY_PARAMS(count, Xtra, & xtra)) \
					: input::underlying_wrapper_type(parent, underlying BOOST_PP_ENUM_TRAILING_PARAMS(count, xtra)) {}
					BOOST_PP_REPEAT(PSYCLE__GENERIC__TEMPLATE_CONSTRUCTORS__ARITY, constructor, ~)
				#undef constructor
		};
		
		namespace inputs
		{
			/***********************************************************************/
			
			template<typename Typenames>
			class single : public universalis::compiler::cast::underlying_wrapper< typename Typenames::underlying::ports::inputs::single, basic::ports::inputs::single<Typenames> >
			{
				protected:
					typedef single single_type;

					#define constructor(_, count, __) \
						BOOST_PP_EXPR_IF(count, template<) BOOST_PP_ENUM_PARAMS(count, typename Xtra) BOOST_PP_EXPR_IF(count, >) \
						single(typename single::parent_type & parent, typename single::underlying_type & underlying BOOST_PP_ENUM_TRAILING_BINARY_PARAMS(count, Xtra, & xtra)) \
						: single::underlying_wrapper_type(parent, underlying BOOST_PP_ENUM_TRAILING_PARAMS(count, xtra)) {}
						BOOST_PP_REPEAT(PSYCLE__GENERIC__TEMPLATE_CONSTRUCTORS__ARITY, constructor, ~)
					#undef constructor
			};

			/***********************************************************************/

			template<typename Typenames>
			class multiple : public universalis::compiler::cast::underlying_wrapper< typename Typenames::underlying::ports::inputs::multiple, basic::ports::inputs::multiple<Typenames> >
			{
				protected:
					typedef multiple multiple_type;

					#define constructor(_, count, __) \
						BOOST_PP_EXPR_IF(count, template<) BOOST_PP_ENUM_PARAMS(count, typename Xtra) BOOST_PP_EXPR_IF(count, >) \
						multiple(typename multiple::parent_type & parent, typename multiple::underlying_type & underlying BOOST_PP_ENUM_TRAILING_BINARY_PARAMS(count, Xtra, & xtra)) \
						: multiple::underlying_wrapper_type(parent, underlying BOOST_PP_ENUM_TRAILING_PARAMS(count, xtra)) {}
						BOOST_PP_REPEAT(PSYCLE__GENERIC__TEMPLATE_CONSTRUCTORS__ARITY, constructor, ~)
					#undef constructor
			};
		}
	}
}}}
