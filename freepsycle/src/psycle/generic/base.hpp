// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2002-2008 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\interface psycle::generic::basic
#pragma once
#include "typenames.hpp"
#include <psycle/engine/exception.hpp>
#include <universalis/compiler/cast.hpp>
#include <universalis/compiler/virtual_factory.hpp>
#include <universalis/operating_system/loggers.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_base_and_derived.hpp>
#include <boost/call_traits.hpp>
#include <boost/signal.hpp>
#include <boost/bind.hpp>
#include <set>
#include <vector>
#include <algorithm>
namespace psycle { namespace generic { namespace basic {

/// the minimum arity for the template constructors used in the virtual factory patterns
#define PSYCLE__GENERIC__TEMPLATE_CONSTRUCTORS__ARITY__MINIMUM \
	UNIVERSALIS__COMPILER__TEMPLATE_CONSTRUCTORS__ARITY__MINIMUM // use the same miminum arity as in universalis
	
// ensure a minimum arity
#if \
	PSYCLE__GENERIC__TEMPLATE_CONSTRUCTORS__ARITY < \
	PSYCLE__GENERIC__TEMPLATE_CONSTRUCTORS__ARITY__MINIMUM
	#undef \
		PSYCLE__GENERIC__TEMPLATE_CONSTRUCTORS__ARITY
	/// the arity for the template constructors used in the virtual factory patterns
	#define \
		PSYCLE__GENERIC__TEMPLATE_CONSTRUCTORS__ARITY \
		PSYCLE__GENERIC__TEMPLATE_CONSTRUCTORS__ARITY__MINIMUM
#endif

using engine::exception;
namespace loggers = universalis::operating_system::loggers;

/***********************************************************************/
/// This template declares a class convertible to its derived type Typenames::graph.
template<typename Typenames>
class graph
:
	// makes the class convertible to its derived type Typenames::graph
	public universalis::compiler::cast::derived<typename Typenames::graph>,
	// note: while the virtual factory pattern is useful for nodes and ports, it could possibly be removed for graph.
	public universalis::compiler::virtual_factory<typename Typenames::graph>,
	public std::set<typename Typenames::node*>
{
	protected: friend class graph::virtual_factory_access;
		typedef graph graph_type;

	///\name signals
	/// Signals are used to propagate changes from an underlying layer to the layers wrapping it.
	/// A wrapping layer will register to the signals of the underlying layer,
	/// to automatically create new wrapping elements (nodes, ports) as peers to new elements in the underlying layer.
	/// This mecanism needs the virtual factory pattern for nodes and ports, to be able to emit the signals only
	/// once the underlying elements are fully constructed: signals send Typenames::xxx types, that is the effective
	/// types, seen as the underlying types by the wrappers, rather than their basic<Typenames>::xxx base types.
	///\{
		public:
			/// signal emitted when a new node is added to the graph
			boost::signal<void (typename Typenames::node &)> & new_node_signal() throw() { return new_node_signal_; }
		private:
			boost::signal<void (typename Typenames::node &)> new_node_signal_;
			
		public:
			/// signal emitted when a node is deleted from the graph
			boost::signal<void (typename Typenames::node &)> & delete_node_signal() throw() { return delete_node_signal_; }
		private:
			boost::signal<void (typename Typenames::node &)> delete_node_signal_;

		public:
			/// signal emitted when two ports (of two different nodes) are connected
			boost::signal<void (typename Typenames::ports::input &, typename Typenames::ports::output &)> & new_connection_signal() throw() { return new_connection_signal_; }
		private:
			boost::signal<void (typename Typenames::ports::input &, typename Typenames::ports::output &)> new_connection_signal_;

		public:
			/// signal emitted when two ports (of two different nodes) are disconnected
			boost::signal<void (typename Typenames::ports::input &, typename Typenames::ports::output &)> & delete_connection_signal() throw() { return delete_connection_signal_; }
		private:
			boost::signal<void (typename Typenames::ports::input &, typename Typenames::ports::output &)> delete_connection_signal_;
	///\}
};

/***********************************************************************/
template<typename Parent>
class child_of {
	protected:
		child_of(Parent & parent) : parent_(parent) {}
	public:
		typedef Parent parent_type;
		Parent const & parent() const throw() { return parent_; }
		Parent       & parent()       throw() { return parent_; }
	private:
		Parent & parent_;
};

/***********************************************************************/
/// This template declares a class convertible to its derived type Typenames::node.
/// The virtual factory pattern is needed to be able to emit the signals only
/// once the objects are fully constructed (see note about the signals in the graph class).
template<typename Typenames>
class node
:
	// makes the class convertible to its derived type Typenames::node
	public universalis::compiler::cast::derived<typename Typenames::node>,
	public universalis::compiler::virtual_factory<typename Typenames::node>,
	public child_of<typename Typenames::graph>
{
	private:
	#if 0
		BOOST_STATIC_ASSERT((boost::is_base_and_derived<child_of<typename Typenames::graph>, typename Typenames::node                   >::value));
		BOOST_STATIC_ASSERT((boost::is_base_and_derived<child_of<typename Typenames::node >, typename Typenames::port                   >::value));
	#endif
		BOOST_STATIC_ASSERT((boost::is_base_and_derived<typename Typenames::port           , typename Typenames::ports::output          >::value));
		BOOST_STATIC_ASSERT((boost::is_base_and_derived<typename Typenames::port           , typename Typenames::ports::input           >::value));
		BOOST_STATIC_ASSERT((boost::is_base_and_derived<typename Typenames::ports::input   , typename Typenames::ports::inputs::single  >::value));
		BOOST_STATIC_ASSERT((boost::is_base_and_derived<typename Typenames::ports::input   , typename Typenames::ports::inputs::multiple>::value));
		
	protected: friend class node::virtual_factory_access;
		typedef node node_type;

		node(typename Typenames::graph & graph)
		:
			child_of<typename Typenames::graph>(graph),
			multiple_input_port_()
		{
			if(loggers::trace()()) loggers::trace()("new generic node", UNIVERSALIS__COMPILER__LOCATION);
		}

		void after_construction() /*override*/ {
			if(loggers::trace()()) loggers::trace()("generic node init", UNIVERSALIS__COMPILER__LOCATION);
			this->parent().insert(&this->derived_this());
			// emit the new_node signal to the wrappers
			this->parent().new_node_signal()(*this);
			
		}

		void before_destruction() /*override*/ {
			this->parent().erase(&this->derived_this());
			// emit the delete_node signal to the wrappers
			this->parent().delete_node_signal()(*this);
			// emit the delete signal to the wrappers
			this->delete_signal()(*this);
		}

		/// virtual destructor
		virtual inline ~node() {}

	///\name destruction
	///\{
		public:  boost::signal<void (node &)> & delete_signal() throw() { return delete_signal_; }
		private: boost::signal<void (node &)>   delete_signal_;
	///\}

	///\name ports: outputs
	///\{
		public:
			typedef std::vector<typename Typenames::ports::output*> output_ports_type;
			/// the output ports owned by this node
			output_ports_type const &  output_ports() const throw() { return output_ports_; }
		private: friend class ports::output<Typenames>;
			output_ports_type          output_ports_;
	///\}

	///\name ports: outputs: signals
	///\{
		public:
			/// signal emitted when a new output port is created for this node
			boost::signal<void (typename Typenames::ports::output &)> & new_output_port_signal() throw() { return new_output_port_signal_; }
		private:
			boost::signal<void (typename Typenames::ports::output &)>   new_output_port_signal_;
	///\}

	///\name ports: inputs
	///\{
		public:
			typedef std::vector<typename Typenames::ports::inputs::single*> single_input_ports_type;
			/// the input ports owned by this node
			single_input_ports_type const & single_input_ports() const throw() { return single_input_ports_; }
		private: friend class ports::inputs::single<Typenames>;
			single_input_ports_type         single_input_ports_;
	///\}
	
	///\name ports: inputs: single: signals
	///\{
		public:
			/// signal emitted when a new single input port is created for this node
			boost::signal<void (typename Typenames::ports::inputs::single &)> & new_single_input_port_signal() throw() { return new_single_input_port_signal_; }
		private:
			boost::signal<void (typename Typenames::ports::inputs::single &)>   new_single_input_port_signal_;
	///\}

	///\name ports: inputs: multiple
	///\{
		public:
			/// the multiple input port owned by this node, if any, or else 0
			typename Typenames::ports::inputs::multiple * const multiple_input_port() const throw() { return multiple_input_port_; }
		private: friend class ports::inputs::multiple<Typenames>;
			typename Typenames::ports::inputs::multiple *       multiple_input_port_;
	///\}
	
	///\name ports: inputs: multiple: signals
	///\{
		public:
			/// signal emitted when the multiple input port is created for this node
			boost::signal<void (typename Typenames::ports::inputs::multiple &)> & new_multiple_input_port_signal() throw() { return new_multiple_input_port_signal_; }
		private:
			boost::signal<void (typename Typenames::ports::inputs::multiple &)>   new_multiple_input_port_signal_;
	///\}
};

/***********************************************************************/
/// This template declares a class convertible to its derived type Typenames::port.
/// The virtual factory pattern is needed to be able to emit the signals only
/// once the objects are fully constructed (see note about the signals in the graph class).
template<typename Typenames>
class port
:
	// makes the class convertible to its derived type Typenames::port
	public universalis::compiler::cast::derived<typename Typenames::port>,
	public universalis::compiler::virtual_factory<typename Typenames::port>,
	public child_of<typename Typenames::node>
{
	protected: friend class port::virtual_factory_access;
		typedef port port_type;
		port(typename Typenames::node & node) : child_of<typename Typenames::node>(node) {}
		void connect(typename Typenames::port &) {} ///\todo any useful? redefined in derived classes?
};

namespace ports {
	
	/***********************************************************************/
	/// This template declares a class convertible to its derived type Typenames::ports::output.
	/// The virtual factory pattern is needed to be able to emit the signals only
	/// once the objects are fully constructed (see note about the signals in the graph class).
	template<typename Typenames>
	class output
	:
		// makes the class convertible to its derived type Typenames::ports::output
		public universalis::compiler::cast::derived<typename Typenames::ports::output>,
		public universalis::compiler::virtual_factory<typename Typenames::ports::output, typename Typenames::port>
	{
		protected: friend class output::virtual_factory_access;
			typedef output output_type;

			/// A wrapper is derived from the basic class, and hence needs at least two arguments in the constructor.
			/// Note that Typenames::port, which is the base class, is in this case derived from wrappers::port<Typenames>.
			UNIVERSALIS__COMPILER__TEMPLATE_CONSTRUCTORS(output, output::virtual_factory_type, PSYCLE__GENERIC__TEMPLATE_CONSTRUCTORS__ARITY)

			void after_construction() /*override*/ {
				this->parent().output_ports_.push_back(static_cast<typename Typenames::ports::output*>(this));
				// emit the new_output_port signal to the wrappers
				this->parent().new_output_port_signal()(*this);
			}

			void before_destruction() /*override*/ {
				disconnect_all();
				// Note that in the case of a wrapper, the underlying layer is already disconnected,
				// but since there is no polymorphic virtual call, this will only disconnect the wrapping layer.
			}

		///\name connected input ports
		///\{
			public:
				typedef std::vector<typename Typenames::ports::input*> input_ports_type;
				input_ports_type const & input_ports() const throw() { return input_ports_; }
			private:
				input_ports_type         input_ports_;
		///\}

		///\name (dis)connection functions
		///\{
			public:
				void connect(typename Typenames::ports::input & input_port) throw(exception) {
					// connect the input port to this output port
					input_port.connect(*this);
				}
			public: // private:
				void connect_internal_side(typename Typenames::ports::input & input_port) {
					// find the input port in our container
					typename input_ports_type::iterator i(std::find(
						input_ports_.begin(), input_ports_.end(), &input_port
					));
					if(i != input_ports_.end()) {
						if(loggers::warning()()) {
							std::ostringstream s;
							s << "already connected";
							loggers::warning()(s.str());
						}
						return;
					}
					// add the newly connected input port to our container
					input_ports_.push_back(&input_port);
				}
			public:
				void disconnect_all() {
					// iterate over all our input ports to disconnect them
					while(!input_ports_.empty()) disconnect(*input_ports_.back());
				}
				void disconnect(typename Typenames::ports::input & input_port) {
					// disconnect the input port from this output port
					input_port.disconnect(*this);
				}
			public: // private:
				void disconnect_internal_side(typename Typenames::ports::input & input_port) {
					// find the input port in our container
					typename input_ports_type::iterator i(std::find(
						input_ports_.begin(), input_ports_.end(), &input_port)
					);
					if(i == input_ports_.end()) {
						if(loggers::warning()()) {
							std::ostringstream s;
							s << "was not connected";
							loggers::warning()(s.str());
						}
						return;
					}
					// remove the disconnected input port from our container
					input_ports_.erase(i);
				}
		///\}
	};

	/***********************************************************************/
	/// This template declares a class convertible to its derived type Typenames::ports::input
	/// The virtual factory pattern is needed to be able to emit the signals only
	/// once the objects are fully constructed (see note about the signals in the graph class).
	template<typename Typenames>
	class input
	:
		// makes the class convertible to its derived type Typenames::ports::input
		public universalis::compiler::cast::derived<typename Typenames::ports::input>,
		public universalis::compiler::virtual_factory<typename Typenames::ports::input, typename Typenames::port>
	{
		protected: friend class input::virtual_factory_access;
			typedef input input_type;

			/// A wrapper is derived from the basic class, and hence needs at least two arguments in the constructor.
			/// Note that Typenames::port, which is the base class, is in this case derived from wrappers::port<Typenames>.
			UNIVERSALIS__COMPILER__TEMPLATE_CONSTRUCTORS(input, input::virtual_factory_type, PSYCLE__GENERIC__TEMPLATE_CONSTRUCTORS__ARITY)

			void before_destruction() /*override*/ {
				disconnect_all();
				// Note that in the case of a wrapper, the underlying layer is already disconnected,
				// but since there is no polymorphic virtual call, this will only disconnect the wrapping layer.
			}

		///\name (dis)connection functions
		///\{
			public:
				void connect(typename Typenames::ports::output & output_port) throw(exception) {
					assert("ports must belong to different nodes:" &&
						&output_port.parent() != &this->parent()
					);
					assert("nodes of both ports must belong to the same graph:" &&
						&output_port.parent().parent() == &this->parent().parent()
					);
					// connect the output port internal side to this input port
					output_port.connect_internal_side(*this);
					// call our base port class connection function
					Typenames::port::connect(output_port);
					// connect this input port internal side to the output port
					this->connect_internal_side(output_port);
					// signal grand parent graph wrappers of the new connection
					this->parent().parent().new_connection_signal()(*this, output_port);
				}
			protected:
				void virtual connect_internal_side(typename Typenames::ports::output &) = 0;

			public:
				void virtual disconnect_all() = 0;
				void disconnect(typename Typenames::ports::output & output_port) {
					// disconnect this input port internal side from the output port
					this->disconnect_internal_side(output_port);
					// disconnect the output port internal side from this input port
					output_port.disconnect_internal_side(*this);
					// signal grand parent graph wrappers of the disconnection
					this->parent().parent().delete_connection_signal()(*this, output_port);
				}
			protected:
				void virtual disconnect_internal_side(typename Typenames::ports::output &) = 0;
		///\}
	};
	
	namespace inputs {

		/***********************************************************************/
		/// This template declares a class convertible to its derived type Typenames::ports::inputs::single
		/// The virtual factory pattern is needed to be able to emit the signals only
		/// once the objects are fully constructed (see note about the signals in the graph class).
		template<typename Typenames>
		class single
		:
			// makes the class convertible to its derived type Typenames::ports::inputs::single
			public universalis::compiler::cast::derived<typename Typenames::ports::inputs::single>,
			public universalis::compiler::virtual_factory<typename Typenames::ports::inputs::single, typename Typenames::ports::input>
		{
			protected: friend class single::virtual_factory_access;
				typedef single single_type;

				/// A wrapper is derived from the basic class, and hence needs at least two arguments in the constructor.
				/// Note that Typenames::ports::input, which is the base class, is in this case derived from wrappers::ports::input<Typenames>.
				#define constructor(_, count, __) \
					BOOST_PP_EXPR_IF(count, template<) BOOST_PP_ENUM_PARAMS(count, typename Xtra) BOOST_PP_EXPR_IF(count, >) \
					single(BOOST_PP_ENUM_BINARY_PARAMS(count, Xtra, & xtra)) \
					: \
						single::virtual_factory_type(BOOST_PP_ENUM_PARAMS(count, xtra)), \
						output_port_() \
					{}
					BOOST_PP_REPEAT(PSYCLE__GENERIC__TEMPLATE_CONSTRUCTORS__ARITY, constructor, ~)
				#undef constructor

				#if 0
				UNIVERSALIS__COMPILER__TEMPLATE_CONSTRUCTORS__WITH_BODY(
					single, single::virtual_factory_type,
					(BOOST_PP_COMMA output_port_() {}),
					PSYCLE__GENERIC__TEMPLATE_CONSTRUCTORS__ARITY
				)
				#endif

				void after_construction() /*override*/ {
					this->parent().single_input_ports_.push_back(static_cast<typename Typenames::ports::inputs::single*>(this));
					// emit the new_single_input_port signal to the wrappers
					this->parent().new_single_input_port_signal()(*this);
				}

			///\name connected output port
			///\{
				public:
					typename Typenames::ports::output * const & output_port() const throw() { return output_port_; }
				private:
					typename Typenames::ports::output *         output_port_;
			///\}
			
			///\name (dis)connection functions
			///\{
				public:
					void disconnect_all() /*override*/ {
						// disconnect the connected output port
						if(output_port_) disconnect(*output_port_);
					}
				
				protected:
					void connect_internal_side(typename Typenames::ports::output & output_port) /*override*/ {
						if(&output_port == this->output_port_) {
							if(loggers::warning()()) {
								std::ostringstream s;
								s << "already connected";
								loggers::warning()(s.str());
							}
							return;
						}
						// store a pointer to the newly connected output port
						this->output_port_ = &output_port;
					}
					void disconnect_internal_side(typename Typenames::ports::output & output_port) /*override*/ {
						if(&output_port != this->output_port_) {
							if(loggers::warning()()) {
								std::ostringstream s;
								s << "was not connected";
								loggers::warning()(s.str());
							}
							return;
						}
						// clear the pointer to the disconnected output port
						this->output_port_ = 0;
					}
			///\}
		};

		/***********************************************************************/
		/// This template declares a class convertible to its derived type Typenames::ports::inputs::multiple
		/// The virtual factory pattern is needed to be able to emit the signals only
		/// once the objects are fully constructed (see note about the signals in the graph class).
		template<typename Typenames>
		class multiple
		:
			// makes the class convertible to its derived type Typenames::ports::inputs::multiple
			public universalis::compiler::cast::derived<typename Typenames::ports::inputs::multiple>,
			public universalis::compiler::virtual_factory<typename Typenames::ports::inputs::multiple, typename Typenames::ports::input>
		{
			protected: friend class multiple::virtual_factory_access;

				typedef multiple multiple_type;

				/// A wrapper is derived from the basic class, and hence needs at least two arguments in the constructor.
				/// Note that Typenames::ports::input, which is the base class, is in this case derived from wrappers::ports::input<Typenames>.
				UNIVERSALIS__COMPILER__TEMPLATE_CONSTRUCTORS(
					multiple, multiple::virtual_factory_type, PSYCLE__GENERIC__TEMPLATE_CONSTRUCTORS__ARITY
				)

				void after_construction() /*override*/ {
					assert(!this->parent().multiple_input_port_);
					this->parent().multiple_input_port_ = static_cast<typename Typenames::ports::inputs::multiple*>(this);
					// emit the new_single_input_port signal to the wrappers
					this->parent().new_multiple_input_port_signal()(*this);
				}
				
			///\name connected output ports
			///\{
				public:
					typedef std::vector<typename Typenames::ports::output*> output_ports_type;
					output_ports_type inline const & output_ports() const throw() { return output_ports_; }
				private:
					output_ports_type                output_ports_;
			///\}

			///\name (dis)connection functions
			///\{
				public:
					void disconnect_all() /*override*/ {
						// iterate over all our connected output ports to disconnect them
						while(!output_ports_.empty()) disconnect(*output_ports_.back());
					}
				protected:
					void connect_internal_side(typename Typenames::ports::output & output_port) /*override*/ {
						// find the output port in our container
						typename output_ports_type::iterator i(std::find(
							output_ports_.begin(), output_ports_.end(), &output_port)
						);
						if(i != output_ports_.end()) {
							if(loggers::warning()()) {
								std::ostringstream s;
								s << "already connected";
								loggers::warning()(s.str());
							}
							return;
						}
						// add the newly connected output port to our container
						output_ports_.push_back(&output_port);
					}
					void disconnect_internal_side(typename Typenames::ports::output & output_port) /*override*/ {
						// find the output port in our container
						typename output_ports_type::iterator i(std::find(
							output_ports_.begin(), output_ports_.end(), &output_port
						));
						if(i == output_ports_.end()) {
							if(loggers::warning()()) {
								std::ostringstream s;
								s << "was not connected";
								loggers::warning()(s.str());
							}
							return;
						}
						// remove the disconnected output port from our container
						output_ports_.erase(i);
					}
			///\}
		};
	}
}

}}}
