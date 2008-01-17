// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2002-2008 psycledelics http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\interface psycle::generic::wrappers
#pragma once
#include "base.hpp"
#include <algorithm>
namespace psycle { namespace generic { namespace wrappers {

///\internal
namespace detail {
	template<typename Underlying_Wrapper>
	class underlying_wrapper_predicate {
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

		graph(typename graph::underlying_type & underlying) : graph::underlying_wrapper_type(underlying) {
			// register to the underlying signals
			on_new_node_signal_connection          = underlying.         new_node_signal().connect(boost::bind(&graph::on_new_node         , this, _1    ));
			on_new_connection_signal_connection    = underlying.   new_connection_signal().connect(boost::bind(&graph::on_new_connection   , this, _1, _2));
			on_delete_connection_signal_connection = underlying.delete_connection_signal().connect(boost::bind(&graph::on_delete_connection, this, _1, _2));
		}

		void after_construction() /*override*/ {
			// polymorphic virtual construction
			basic::graph<Typenames>::after_construction();
			
			// Iterate over all the nodes of the underlying graph
			// to emit the new_node signal to the wrapping graph.
			for(typename graph::underlying_type::const_iterator i(this->underlying().begin());
				i != this->underlying().end(); ++i
			) on_new_node(**i);
			
			// Iterate over all the underlying nodes of the graph
			// to connect the wrapping ports
			for(typename graph::const_iterator i(this->begin()) ; i != this->end() ; ++i) {
				typename Typenames::node & node(**i);
				for(typename Typenames::node::output_ports_type::const_iterator i(node.output_ports().begin());
					i != node.output_ports().end(); ++i
				) {
					typename Typenames::ports::output & output_port(**i);
					for(typename Typenames::ports::output::underlying_type::
						input_ports_type::const_iterator i(output_port.underlying().input_ports().begin());
						i != output_port.underlying().input_ports().end() ; ++i
					) {
						typename Typenames::ports::input & input_port(underlying_wrapper(**i));
						output_port.connect(input_port);
					}
				}
			}
		}

		virtual ~graph() {
			// disconnect from the underlying signals
			on_new_node_signal_connection.disconnect();
			on_new_connection_signal_connection.disconnect();
			on_delete_connection_signal_connection.disconnect();
		}
		
	///\name signal slots and connections
	///\{
		private:
			boost::signals::connection on_new_node_signal_connection;
			void on_new_node(typename Typenames::underlying::node & underlying_node) {
				// create a wrapper for the new underlying node
				Typenames::node::template create_on_heap(
					static_cast<typename Typenames::graph &>(*this),
					underlying_node
				);
			}

			#if 0
			boost::signals::connection on_delete_node_signal_connection;
				void on_delete_node(typename Typenames::underlying::node & underlying_node) {
				this->delete_node_signal()(underlying_wrapper(underlying_node));
			}
			#endif

			boost::signals::connection on_new_connection_signal_connection;
			void on_new_connection(
				typename Typenames::underlying::ports::input & underlying_input_port,
				typename Typenames::underlying::ports::output & underlying_output_port
			) {
				// emit the wrapper signal
				this->new_connection_signal()(
					underlying_wrapper(underlying_input_port),
					underlying_wrapper(underlying_output_port)
				);
			}

			boost::signals::connection on_delete_connection_signal_connection;
			void on_delete_connection(
				typename Typenames::underlying::ports::input & underlying_input_port,
				typename Typenames::underlying::ports::output & underlying_output_port
			) {
				// emit the wrapper signal
				this->delete_connection_signal()(
					underlying_wrapper(underlying_input_port),
					underlying_wrapper(underlying_output_port)
				);
			}
	///\}

	public:
		/// returns the node wrapper corresponding to the given underlying node
		typename Typenames::node & underlying_wrapper(typename Typenames::underlying::node & underlying_node) {
			assert("the underlying node must belong to the underlying graph" &&
				&underlying_node.parent() == &this->underlying()
			);
			typename graph::const_iterator i(std::find_if(
				this->begin(), this->end(),
				detail::underlying_wrapper_predicate<typename Typenames::node>(underlying_node)
			)); // or composition with boost::bind
			assert(i != this->end());
			return **i;
		}

	private:
		/// returns the output port wrapper corresponding to the given underlying output port
		typename Typenames::ports::output & underlying_wrapper(
			typename Typenames::underlying::ports::output & underlying_output_port
		) {
			return underlying_wrapper(underlying_output_port.parent()).underlying_wrapper(underlying_output_port);
		}

		/// returns the input port wrapper corresponding to the given underlying input port
		typename Typenames::ports::input & underlying_wrapper(
			typename Typenames::underlying::ports::input & underlying_input_port
		) {
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
			// register to the underlying signals
			on_new_output_port_signal_connection         = underlying.        new_output_port_signal().connect(boost::bind(&node::on_new_output_port        , this, _1));
			on_new_single_input_port_signal_connection   = underlying.  new_single_input_port_signal().connect(boost::bind(&node::on_new_single_input_port  , this, _1));
			on_new_multiple_input_port_signal_connection = underlying.new_multiple_input_port_signal().connect(boost::bind(&node::on_new_multiple_input_port, this, _1));
			on_delete_signal_connection                  = underlying.                 delete_signal().connect(boost::bind(&node::on_delete                 , this, _1));
		}

		void after_construction() /*override*/ {
			// polymorphic virtual construction
			basic::node<Typenames>::after_construction();

			// Iterate over all the underlying output ports of the underlying node
			// to emit the new_output_port signal to the wrapping node.
			for(typename node::underlying_type::output_ports_type::const_iterator
				i(this->underlying().output_ports().begin());
				i != this->underlying().output_ports().end(); ++i
			) on_new_output_port(**i);

			// Iterate over all the underlying single input ports of the underlying node
			// to emit the new_single_input_port signal to the wrapping node.
			for(typename node::underlying_type::single_input_ports_type::const_iterator
				i(this->underlying().single_input_ports().begin());
				i != this->underlying().single_input_ports().end(); ++i
			) on_new_single_input_port(**i);

			// If underlying node has a multiple input port,
			// emit the new_multiple_input_port signal to the wrapping node.
			if(this->underlying().multiple_input_port())
				on_new_multiple_input_port(*this->underlying().multiple_input_port());
		}

		virtual ~node() {
			// disconnect from the underlying signals
			on_new_output_port_signal_connection.disconnect();
			on_new_single_input_port_signal_connection.disconnect();
			on_new_multiple_input_port_signal_connection.disconnect();
			on_delete_signal_connection.disconnect();
		}
		
	///\name signal slots and connections
	///\{
		private:
			boost::signals::connection on_delete_signal_connection;
			void on_delete(typename node::underlying_type & underlying) {
				// automatic destruction of the wrapper when the underlying node is destroyed
				assert(&underlying == &this->underlying());
				delete this;
			}
			
			///\todo remove. unused.
			void static on_delete_(node & wrapper, typename node::underlying_type & underlying) {
				assert(&underlying == &wrapper.underlying());
				delete &wrapper;
			}

			boost::signals::connection on_new_output_port_signal_connection;
			void on_new_output_port(typename Typenames::underlying::ports::output & underlying_output_port) {
				// create a wrapper for the new underlying output port
				Typenames::ports::output::template create_on_heap(
					static_cast<typename Typenames::node &>(*this),
					underlying_output_port
				);
			}

			boost::signals::connection on_new_single_input_port_signal_connection;
			void on_new_single_input_port(typename Typenames::underlying::ports::inputs::single & underlying_single_input_port) {
				// create a wrapper for the new underlying single input port
				Typenames::ports::inputs::single::template create_on_heap(
					static_cast<typename Typenames::node &>(*this),
					underlying_single_input_port
				);
			}

			boost::signals::connection on_new_multiple_input_port_signal_connection;
			void on_new_multiple_input_port(typename Typenames::underlying::ports::inputs::multiple & underlying_multiple_input_port) {
				// create a wrapper for the new underlying multiple input port
				Typenames::ports::inputs::multiple::template create_on_heap(
					static_cast<typename Typenames::node &>(*this),
					underlying_multiple_input_port
				);
			}
	///\}

	public: //private:
		/// returns the the output port wrapper corresponding to the given underlying output port
		typename Typenames::ports::output & underlying_wrapper(typename Typenames::underlying::ports::output & underlying_output_port) {
			assert("the underlying port must belong to the underlying node" &&
				&underlying_output_port.parent() == &this->underlying()
			);
			typename node::output_ports_type::const_iterator i(std::find_if(
				this->output_ports().begin(), this->output_ports().end(),
				detail::underlying_wrapper_predicate<typename Typenames::ports::output>(underlying_output_port)
			));
			assert(i != this->output_ports().end());
			return **i;
		}

		/// returns the the input port wrapper corresponding to the given underlying input port
		typename Typenames::ports::input & underlying_wrapper(typename Typenames::underlying::ports::input & underlying_input_port) {
			assert("the underlying port must belong to the underlying node" &&
				&underlying_input_port.parent() == &this->underlying()
			);
			if(
				this->multiple_input_port() &&
				&this->multiple_input_port()->underlying() == &underlying_input_port
			) return *this->multiple_input_port();
			
			typename node::single_input_ports_type::const_iterator i(std::find_if(
				this->single_input_ports().begin(), this->single_input_ports().end(),
				detail::underlying_wrapper_predicate<typename Typenames::ports::input>(underlying_input_port)
			));
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

namespace ports {

	/***********************************************************************/
	template<typename Typenames>
	class output
	:
		public universalis::compiler::cast::underlying_wrapper<
			typename Typenames::underlying::ports::output,
			universalis::compiler::virtual_factory<
				typename Typenames::ports::output,
				basic::ports::output<Typenames>
			> >
	{
		protected:
			typedef output output_type;
			UNIVERSALIS__COMPILER__TEMPLATE_CONSTRUCTORS(
				output, output::underlying_wrapper_type, PSYCLE__GENERIC__TEMPLATE_CONSTRUCTORS__ARITY
			)
	};

	/***********************************************************************/
	template<typename Typenames>
	class input
	:
		public universalis::compiler::cast::underlying_wrapper<
			typename Typenames::underlying::ports::input,
			universalis::compiler::virtual_factory<
				typename Typenames::ports::input,
				basic::ports::input<Typenames>
			> >
	{
		protected:
			typedef input input_type;
			UNIVERSALIS__COMPILER__TEMPLATE_CONSTRUCTORS(
				input, input::underlying_wrapper_type, PSYCLE__GENERIC__TEMPLATE_CONSTRUCTORS__ARITY
			)
	};
	
	namespace inputs {

		/***********************************************************************/
		template<typename Typenames>
		class single
		:
			public universalis::compiler::cast::underlying_wrapper<
				typename Typenames::underlying::ports::inputs::single,
				universalis::compiler::virtual_factory<
					typename Typenames::ports::inputs::single,
					basic::ports::inputs::single<Typenames>
				> >
		{
			protected:
				typedef single single_type;
				UNIVERSALIS__COMPILER__TEMPLATE_CONSTRUCTORS(
					single, single::underlying_wrapper_type, PSYCLE__GENERIC__TEMPLATE_CONSTRUCTORS__ARITY
				)
		};

		/***********************************************************************/
		template<typename Typenames>
		class multiple
		:
			public universalis::compiler::cast::underlying_wrapper<
				typename Typenames::underlying::ports::inputs::multiple,
				universalis::compiler::virtual_factory<
					typename Typenames::ports::inputs::multiple,
					basic::ports::inputs::multiple<Typenames>
				> >
		{
			protected:
				typedef multiple multiple_type;
				UNIVERSALIS__COMPILER__TEMPLATE_CONSTRUCTORS(
					multiple, multiple::underlying_wrapper_type, PSYCLE__GENERIC__TEMPLATE_CONSTRUCTORS__ARITY
				)
		};
	}
}

}}}
