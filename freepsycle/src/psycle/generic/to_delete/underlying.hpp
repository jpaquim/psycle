// -*- mode:c++; indent-tabs-mode:t -*-
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2008-2008 psycledelics http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\interface psycle::generic::basic
#pragma once
#include "basic.hpp"
#include <boost/signal.hpp>
#include <boost/bind.hpp>
namespace psycle { namespace generic { namespace underlying {

using engine::exception;
namespace loggers = universalis::operating_system::loggers;

/***********************************************************************/
template<typename Typenames>
class graph : public universalis::compiler::virtual_factory<typename Typenames::graph, basic::graph<Typenames> > {
	protected: friend class graph::virtual_factory_access;

	///\name signals
	///\{
		public:
			/// signal emitted to the wrappers when a new node is added to the graph
			boost::signal<void (typename Typenames::node &)> & new_node_signal() throw() { return new_node_signal_; }
		private:
			boost::signal<void (typename Typenames::node &)> new_node_signal_;
			
		public:
			/// signal emitted to the wrappers when a node is deleted from the graph
			boost::signal<void (typename Typenames::node &)> & delete_node_signal() throw() { return delete_node_signal_; }
		private:
			boost::signal<void (typename Typenames::node &)> delete_node_signal_;

		public:
			/// signal emitted to the wrappers when two ports (of two different nodes) are connected
			boost::signal<void (typename Typenames::ports::input &, typename Typenames::ports::output &)> & new_connection_signal() throw() { return new_connection_signal_; }
		private:
			boost::signal<void (typename Typenames::ports::input &, typename Typenames::ports::output &)> new_connection_signal_;

		public:
			/// signal emitted to the wrappers when two ports (of two different nodes) are disconnected
			boost::signal<void (typename Typenames::ports::input &, typename Typenames::ports::output &)> & delete_connection_signal() throw() { return delete_connection_signal_; }
		private:
			boost::signal<void (typename Typenames::ports::input &, typename Typenames::ports::output &)> delete_connection_signal_;
	///\}
};

/***********************************************************************/
template<typename Typenames>
class node : public universalis::compiler::virtual_factory<typename Typenames::node, basic::node<Typenames> > {
	protected: friend class node::virtual_factory_access;
		node(typename Typenames::graph & graph)
		:
			child_of_type(graph),
			multiple_input_port_()
		{
			this->parent().insert(static_cast<typename Typenames::node*>(this));
			// emit the new_node signal to the wrappers
			this->parent().new_node_signal()(*this);
		}

		/// virtual destructor
		virtual ~node() {
			this->parent().erase(static_cast<typename Typenames::node*>(this));
			// emit the delete_node signal to the wrappers
			this->parent().delete_node_signal()(*this);
			// emit the delete signal to the wrappers
			this->delete_signal()(*this);
		}

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
			output_ports_type output_ports_;
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
			single_input_ports_type single_input_ports_;
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
			typename Typenames::ports::inputs::multiple * multiple_input_port_;
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
/// This template declares a class convertible to its derived type Typenames::port
template<typename Typenames>
class port
:
	// makes the class convertible to its derived type Typenames::port
	public universalis::compiler::cast::derived<typename Typenames::port>,
	public child_of<typename Typenames::node>
{
	protected: typedef port basic_port_type;
	public: port(typename Typenames::node & node) : child_of_type(node) {}
};

namespace ports {
	
	/***********************************************************************/
	/// This template declares a class convertible to its derived type Typenames::ports::output
	template<typename Typenames>
	class output
	:
		// makes the class convertible to its derived type Typenames::ports::output
		public universalis::compiler::cast::derived<typename Typenames::ports::output>,
		public Typenames::port
	{
		protected:
			typedef output basic_output_type;

			/// a wrapper is derived from the basic class, and hence needs two arguments in the constructor.
			/// note that Typenames::port in this case is derived from wrappers::port<Typenames>
			template<typename Underlying_Output_Port>
			output(typename Typenames::node & node, Underlying_Output_Port & underlying_output_port)
			: Typenames::port(node, underlying_output_port) {
				this->parent().output_ports_.push_back(static_cast<typename Typenames::ports::output*>(this));
				// emit the new_output_port signal to the wrappers
				this->parent().new_output_port_signal()(*this);
			}

		public:
			output(typename Typenames::node & node) : port<Typenames>(node) {
				this->parent().output_ports_.push_back(static_cast<typename Typenames::ports::output*>(this));
				// emit the new_output_port signal to the wrappers
				this->parent().new_output_port_signal()(*this);
			}

			~output() throw() {
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
				input_ports_type input_ports_;
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
	template<typename Typenames>
	class input
	:
		// makes the class convertible to its derived type Typenames::ports::input
		public universalis::compiler::cast::derived<typename Typenames::ports::input>,
		public Typenames::port
	{
		protected:
			typedef input basic_input_type;

			/// a wrapper is derived from the basic class, and hence needs two arguments in the constructor.
			/// note that Typenames::port in this case is derived from wrappers::port<Typenames>
			template<typename Underlying_Input_Port>
			input(typename Typenames::node & node, Underlying_Input_Port & underlying_input_port)
			: Typenames::port(node, underlying_input_port) {}

			input(typename Typenames::node & node) : Typenames::port(node) {}

			#if 0 // disconnect_all is overridden in derived classes
				~input() throw() {
					//disconnect_all();
				}
			#endif

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
		template<typename Typenames>
		class single
		:
			// makes the class convertible to its derived type Typenames::ports::inputs::single
			public universalis::compiler::cast::derived<typename Typenames::ports::inputs::single>,
			public input<Typenames>
		{
			protected:
				typedef single basic_single_type;

				/// a wrapper is derived from the basic class, and hence needs two arguments in the constructor.
				/// note that Typenames::port in this case is derived from wrappers::port<Typenames>
				template<typename Underlying_Input_Port>
				input(typename Typenames::node & node, Underlying_Input_Port & underlying_input_port)
				: Typenames::port(node, underlying_input_port) {}

			public: 
				single(typename Typenames::node & node)
				:
					input<Typenames>(node),
					output_port_
				{
					this->parent().single_input_ports_.push_back(static_cast<typename Typenames::ports::inputs::single*>(this));
					// emit the new_single_input_port signal to the wrappers
					this->parent().new_single_input_port_signal()(*this);
				}

				~single() {
					disconnect_all();
					// Note that in the case of a wrapper, the underlying layer is already disconnected,
					// but since there is no polymorphic virtual call, this will only disconnect the wrapping layer.
				}

			///\name connected output port
			///\{
				public:  typename Typenames::ports::output * const & output_port() const throw() { return output_port_; }
				private: typename Typenames::ports::output *         output_port_;
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
		template<typename Typenames>
		class multiple
		:
			// makes the class convertible to its derived type Typenames::ports::inputs::multiple
			public universalis::compiler::cast::derived<typename Typenames::ports::inputs::multiple>,
			public input<Typenames>
		{
			protected:
				typedef multiple basic_multiple_type;

				/// a wrapper is derived from the basic class, and hence needs two arguments in the constructor.
				/// note that Typenames::port in this case is derived from wrappers::port<Typenames>
				template<typename Underlying_Input_Port>
				input(typename Typenames::node & node, Underlying_Input_Port & underlying_input_port)
				: Typenames::port(node, underlying_input_port) {}

			public:
				multiple(typename Typenames::node & node)
				:
					input<Typenames>(node)
				{
					assert(!this->parent().multiple_input_port_);
					this->parent().multiple_input_port_ = static_cast<typename Typenames::ports::inputs::multiple*>(this);
					// emit the new_single_input_port signal to the wrappers
					this->parent().new_multiple_input_port_signal()(*this);
				}
				
				~multiple() {
					disconnect_all();
					// Note that in the case of a wrapper, the underlying layer is already disconnected,
					// but since there is no polymorphic virtual call, this will only disconnect the wrapping layer.
				}

			///\name connected output ports
			///\{
				public:
					typedef std::vector<typename Typenames::ports::output*> output_ports_type;
					output_ports_type inline const & output_ports() const throw() { return output_ports_; }
				private:
					output_ports_type output_ports_;
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
