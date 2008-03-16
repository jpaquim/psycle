// -*- mode:c++; indent-tabs-mode:t -*-
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2002-2008 psycledelics http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\interface psycle::generic::wrappers
#pragma once
#include "basic.hpp"
#include <algorithm>
namespace psycle { namespace generic { namespace wrappers {

using engine::exception;
namespace loggers = universalis::operating_system::loggers;

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
/// This template declares a class whose underlying type is Typenames::underlying::graph
template<typename Typenames>
class graph
:
	public basic::graph<Typenames>,
	public universalis::compiler::cast::underlying_wrapper<typename Typenames::underlying::graph>
{
	protected: typedef graph wrapper_graph_type;
		
	public:
		graph(typename graph::underlying_type & underlying) : graph::underlying_wrapper_type(underlying) {
			// register to the underlying signals
			on_new_node_signal_connection = underlying.new_node_signal().connect(
				boost::bind(&Typenames::graph::on_new_node, static_cast<typename Typenames::graph *>(this), _1)
			);
			#if 0
				on_delete_node_signal_connection = underlying.delete_node_signal().connect(
					boost::bind(&Typenames::graph::on_delete_node, static_cast<typename Typenames::graph *>(this), _1)
				);
			#endif
			on_new_connection_signal_connection = underlying.new_connection_signal().connect(
				boost::bind(&Typenames::graph::on_new_connection, static_cast<typename Typenames::graph *>(this), _1, _2)
			);
			on_delete_connection_signal_connection = underlying.delete_connection_signal().connect(
				boost::bind(&Typenames::graph::on_delete_connection, static_cast<typename Typenames::graph *>(this), _1, _2)
			);

			// Iterate over all the nodes of the underlying graph to create wrapping nodes.
			for(typename graph::underlying_type::const_iterator i(this->underlying().begin());
				i != this->underlying().end(); ++i
			) on_new_node(**i);

			// Iterate over all the nodes of the graph to connect the wrapping ports.
			for(typename graph::const_iterator i(this->begin()) ; i != this->end() ; ++i) {
				typename Typenames::node & node(**i);
				for(typename Typenames::node::output_ports_type::const_iterator i(node.output_ports().begin());
					i != node.output_ports().end(); ++i
				) {
					typename Typenames::ports::output & output_port(**i);
					for(typename Typenames::ports::output::underlying_type::
						input_ports_type::const_iterator i(output_port.underlying().input_ports().begin());
						i != output_port.underlying().input_ports().end() ; ++i
					)
						// The underlying layer is already connected, we only have to connect this wrapping layer.
						static_cast<basic::ports::input<Typenames> &>(underlying_wrapper(**i)).connect(output_port);
				}
			}
		}

		virtual ~graph() {
			// disconnect from the underlying signals
			on_new_node_signal_connection.disconnect();
			//on_delete_node_signal_connection.disconnect();
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

			#if 0 // this is done by the nodes themselves (they call "delete this" directly, so it's faster)
				boost::signals::connection on_delete_node_signal_connection;
				void on_delete_node(typename Typenames::underlying::node & underlying_node) {
						// automatic destruction of the wrapper when the underlying node is destroyed
						delete &underlying_wrapper(underlying_node);
				}
			#endif

			boost::signals::connection on_new_connection_signal_connection;
			void on_new_connection(
				typename Typenames::underlying::ports::input & underlying_input_port,
				typename Typenames::underlying::ports::output & underlying_output_port
			) {
				// The underlying layer is already connected, we only have to connect this wrapping layer.
				static_cast<basic::ports::input<Typenames> &>(underlying_wrapper(underlying_input_port)).connect(underlying_wrapper(underlying_output_port));
			}

			boost::signals::connection on_delete_connection_signal_connection;
			void on_delete_connection(
				typename Typenames::underlying::ports::input & underlying_input_port,
				typename Typenames::underlying::ports::output & underlying_output_port
			) {
				// The underlying layer is already disconnected, we only have to disconnect this wrapping layer.
				basic::ports:: input<Typenames> & in (underlying_wrapper(underlying_input_port));
				basic::ports::output<Typenames> & out(underlying_wrapper(underlying_output_port));
				in.disconnect(out);
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
/// This template declares a class whose underlying type is Typenames::underlying::node
template<typename Typenames>
class node
:
	public basic::node<Typenames>,
	public universalis::compiler::cast::underlying_wrapper<typename Typenames::underlying::node>
{
	protected:
		typedef node wrapper_node_type;
		
	public:
		node(typename node::parent_type & parent, typename node::underlying_type & underlying)
		:
			basic::node<Typenames>(parent),
			node::underlying_wrapper_type(underlying)
		{
			// register to the underlying signals
			on_new_output_port_signal_connection = underlying.new_output_port_signal().connect(
				boost::bind(&Typenames::node::on_new_output_port, static_cast<typename Typenames::node *>(this), _1)
			);
			on_new_single_input_port_signal_connection = underlying.new_single_input_port_signal().connect(
				boost::bind(&Typenames::node::on_new_single_input_port, static_cast<typename Typenames::node *>(this), _1)
			);
			on_new_multiple_input_port_signal_connection = underlying.new_multiple_input_port_signal().connect(
				boost::bind(&Typenames::node::on_new_multiple_input_port, static_cast<typename Typenames::node *>(this), _1)
			);
			on_delete_signal_connection = underlying.delete_signal().connect(
				boost::bind(&Typenames::node::on_delete, static_cast<typename Typenames::node *>(this), _1)
			);

			// Iterate over all the underlying output ports of the underlying node to create wrapping output ports.
			for(typename node::underlying_type::output_ports_type::const_iterator
				i(this->underlying().output_ports().begin());
				i != this->underlying().output_ports().end(); ++i
			) on_new_output_port(**i);

			// Iterate over all the underlying single input ports of the underlying node to create wrapping single input ports.
			for(typename node::underlying_type::single_input_ports_type::const_iterator
				i(this->underlying().single_input_ports().begin());
				i != this->underlying().single_input_ports().end(); ++i
			) on_new_single_input_port(**i);

			// If underlying node has a multiple input port, create a wrapping multiple input port.
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
			void on_delete(typename node::underlying_type & underlying) throw() {
				// automatic destruction of the wrapper when the underlying node is destroyed
				assert(&underlying == &this->underlying());
				delete this;
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
/// This template declares a class whose underlying type is Typenames::underlying::port
template<typename Typenames>
class port
:
	public basic::port<Typenames>,
	public universalis::compiler::cast::underlying_wrapper<typename Typenames::underlying::port>
{
	protected:
		typedef port wrapper_port_type;

		port(typename port::parent_type & parent, typename port::underlying_type & underlying)
		:
			basic::port<Typenames>(parent),
			port::underlying_wrapper_type(underlying)
		{}
};

namespace ports {

	/***********************************************************************/
	/// This template declares a class derived from Typenames::port,
	/// which is derived from wrappers::port<Typenames>,
	/// and whose underlying type is Typenames::underlying::ports::output
	template<typename Typenames>
	class output
	:
		public universalis::compiler::cast::underlying_wrapper<
			typename Typenames::underlying::ports::output,
			basic::ports::output<Typenames>
			// note that the base class of basic::ports::output<Typenames>
			// is Typenames::port, which is derived from wrappers::port<Typenames>
		>
	{
		protected:
			typedef output wrapper_output_type;
			
			output(typename Typenames::node & node, typename output::underlying_type & underlying_output_port)
			: output::underlying_wrapper_type(node, underlying_output_port) {}
	};

	/***********************************************************************/
	/// This template declares a class derived from Typenames::port,
	/// which is derived from wrappers::port<Typenames>,
	/// and whose underlying type is Typenames::underlying::ports::input
	template<typename Typenames>
	class input
	:
		public universalis::compiler::cast::underlying_wrapper<
			typename Typenames::underlying::ports::input,
			basic::ports::input<Typenames>
			// note that the base class of basic::ports::input<Typenames>
			// is Typenames::port, which is derived from wrappers::port<Typenames>
		>
	{
		protected:
			typedef input wrapper_input_type;
			
			input(typename Typenames::node & node, typename input::underlying_type & underlying_input_port)
			: input::underlying_wrapper_type(node underlying_input_port) {}

		///\name (dis)connection functions
		///\{
			public:
				/// delegate the connection to the underlying layer
				void connect(typename Typenames::ports::output & output_port) throw(exception) {
					this->underlying().connect(output_port.underlying());
				}

				/// delegate the disconnection to the underlying layer
				void disconnect(typename Typenames::ports::output & output_port) {
					this->underlying().disconnect(output_port.underlying());
				}

				/// delegate the disconnection to the underlying layer
				void disconnect_all() /*override*/ {
					this->underlying().disconnect_all();
				}
		///\}
	};
	
	namespace inputs {

		/***********************************************************************/
		/// This template declares a class derived from Typenames::ports::input,
		/// which is derived from wrappers::ports::input<Typenames>,
		/// and whose underlying type is Typenames::underlying::ports::inputs::single
		template<typename Typenames>
		class single
		:
			public universalis::compiler::cast::underlying_wrapper<
				typename Typenames::underlying::ports::inputs::single,
				basic::ports::inputs::single<Typenames>
				// note that the base class of basic::ports::inputs::single<Typenames>
				// is Typenames::ports::input, which is derived from wrappers::ports::input<Typenames>
			>
		{
			protected: typedef single wrapper_single_type;
				
			public:
				single(typename Typenames::node & node, typename single::underlying_type & underlying_single_input_port)
				: single::underlying_wrapper_type(node, underlying_single_input_port) {}
		};

		/***********************************************************************/
		/// This template declares a class derived from Typenames::ports::input,
		/// which is derived from wrappers::ports::input<Typenames>,
		/// and whose underlying type is Typenames::underlying::ports::inputs::multiple
		template<typename Typenames>
		class multiple
		:
			public universalis::compiler::cast::underlying_wrapper<
				typename Typenames::underlying::ports::inputs::multiple,
				basic::ports::inputs::multiple<Typenames>
				// note that the base class of basic::ports::inputs::multiple<Typenames>
				// is Typenames::ports::input, which is derived from wrappers::ports::input<Typenames>
			>
		{
			protected: typedef multiple wrapper_multiple_type;
				
			public:
				multiple(typename Typenames::node & node, typename multiple::underlying_type & underlying_multiple_input_port)
				: multiple::underlying_wrapper_type(node, underlying_multiple_input_port) {}
		};
	}
}

}}}
