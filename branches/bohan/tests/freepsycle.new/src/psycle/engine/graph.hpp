// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2010 members of the psycle project http://psycle.sourceforge.net : johan boule <bohan@jabber.org>

///\interface psycle::engine::graph
#pragma once
#include "forward_declarations.hpp"
#include "named.hpp"
#include "buffer.hpp"
#include <universalis/stdlib/mutex.hpp>
#include <set>
#define PSYCLE__DECL  PSYCLE__ENGINE
#include <psycle/detail/decl.hpp>
namespace psycle { namespace engine {

using namespace universalis::stdlib;

/**********************************************************************************************************************/
// graph
/// a set of nodes
class PSYCLE__DECL graph : public named {
	public:
		graph(name_type const &);
		virtual ~graph();

	///\name signals
	///\{
		public:
			/// signal emitted when a new node is added to the graph
			boost::signal<void (node &)> & new_node_signal() throw() { return new_node_signal_; }
		private:
			boost::signal<void (node &)> new_node_signal_;

		public:
			/// signal emitted when a node is deleted from the graph
			boost::signal<void (node &)> & delete_node_signal() throw() { return delete_node_signal_; }
		private:
			boost::signal<void (node &)> delete_node_signal_;

		public:
			/// signal emitted when two ports (of two different nodes) are connected
			boost::signal<void (ports::input &, ports::output &)> & new_connection_signal() throw() { return new_connection_signal_; }
		private:
			boost::signal<void (ports::input &, ports::output &)> new_connection_signal_;

		public:
			/// signal emitted when two ports (of two different nodes) are disconnected
			boost::signal<void (ports::input &, ports::output &)> & delete_connection_signal() throw() { return delete_connection_signal_; }
		private:
			boost::signal<void (ports::input &, ports::output &)> delete_connection_signal_;
	///\}

	///\name thread synchronisation
		public:
			typedef class scoped_lock<mutex> scoped_lock;
			operator mutex & () const { return mutex_; }
		private:
			mutex mutable mutex_;
	///\}

	public:
		/// the length of each channel of the buffers
		std::size_t events_per_buffer() { return events_per_buffer_; }
	private:
		std::size_t events_per_buffer_;

	///\name name
	///\{
		public:
			/// the fully qualified name (a path)
			name_type qualified_name() const;
			virtual void dump(std::ostream &, std::size_t tabulations = 0) const;
	///\}
};
/// outputs a textual representation of a graph.
///\relates graph
///\see graph::dump
PSYCLE__DECL std::ostream & operator<<(std::ostream & out, graph const &);

/**********************************************************************************************************************/
// node
/// node of a graph, placeholder for a dsp, aka "plugin machine".
class PSYCLE__DECL node : public named {
	friend class graph;
	friend class port;
	friend class ports::output;
	friend class ports::input;
	friend class ports::inputs::single;
	friend class ports::inputs::multiple;

	public:
		node(class plugin_library_reference &, class graph &, name_type const &);
		virtual ~node();

	///\name reference to plugin library
	///\{
		public:
			class plugin_library_reference & plugin_library_reference() const throw() { return plugin_library_reference_; }
		private:
			class plugin_library_reference & plugin_library_reference_;
	///\}

	///\name output ports
	///\{
		public:
			/// finds an output port by its name
			ports::output * const output_port(name_type const &) const;
	///\}

	///\name input ports
	///\{
		public:
			/// finds an input port by its name
			ports::input * const input_port(name_type const &) const;
	///\}

	///\name open
	///\{
		public:
			/// called by schedulers
			void open() throw(std::exception) { if(!opened()) try { do_open(); } catch(...) { do_close(); throw; } }
			void opened(bool value) throw(std::exception) { if(value) open(); else close(); }
			virtual bool opened() const { return opened_; }
		protected:
			virtual void do_open() throw(std::exception);
		private:
			bool opened_;
	///\}

	///\name start
	///\{
		public:
			/// called by schedulers
			void start() throw(std::exception) { open(); if(!started()) try { do_start(); } catch(...) { do_stop(); throw; } }
			void started(bool value) throw(std::exception) { if(value) start(); else stop(); }
			virtual bool started() const { return started_; }
		protected:
			virtual void do_start() throw(std::exception);
		private:
			bool started_;
	///\}

	///\name process
	///\{
		public:
			/// indicates whether the underlying device (if any) is ready to process.
			/// called by schedulers
			bool io_ready() const throw() { scoped_lock lock(mutex_); return io_ready_; }
		protected:
			/// Derived classes that drive an underlying device should call this setter.
			/// When changed from false to true, the io_ready_signal is emitted.
			void io_ready(bool io_ready);
		private:
			bool io_ready_;
			typedef class scoped_lock<mutex> scoped_lock;
			mutex mutable mutex_;

		public:
			/// signal to be emitted when the underlying device (if any) becomes ready to process
			/// This signal is to be registered by schedulers.
			boost::signal<void (node &)> & io_ready_signal() throw() { return io_ready_signal_; }
		private:
			boost::signal<void (node &)> io_ready_signal_;

		public:
			/// called by schedulers
			void inline process_first() throw(std::exception);
		protected:
			/// this function is the placeholder where to put the dsp algorithm.
			/// re-implement this function in a derived class and put your own code in it.
			virtual void do_process_first() throw(std::exception) {}

		public:
			/// called by schedulers
			void inline process() throw(std::exception);
		protected:
			/// this function is the placeholder where to put the dsp algorithm.
			/// re-implement this function in a derived class and put your own code in it.
			virtual void do_process() throw(std::exception) = 0;

		public:
			/// called by schedulers, reset the state of this node so that it prepares for the next call to process()
			void inline reset();
		protected:
			virtual void do_reset() {}
	///\}

	///\name stop
	///\{
		public:
			/// called by schedulers
			void stop() throw(std::exception) { if(started()) do_stop(); }
		protected:
			virtual void do_stop() throw(std::exception);
	///\}

	///\name close
	///\{
		public:
			/// called by schedulers
			void close() throw(std::exception) { stop(); if(opened()) do_close(); }
		protected:
			virtual void do_close() throw(std::exception);
	///\}

	protected:
		virtual void channel_change_notification_from_port(port const &) throw(std::exception) {}
		virtual void seconds_per_event_change_notification_from_port(port const &) {}
		void quaquaversal_propagation_of_seconds_per_event_change_notification_from_port(port const &);

	///\name name
	///\{
		public:
			/// the full path of the node (within its graph)
			name_type qualified_name() const;
			virtual void dump(std::ostream &, std::size_t tabulations = 0) const;
	///\}

	///\name destruction signal
	///\{
		public:  boost::signal<void (node &)> & delete_signal() throw() { return delete_signal_; }
		private: boost::signal<void (node &)>   delete_signal_;
	///\}

	///\name graph
	///\{
		public:
			class graph & graph() { return graph_; }
			class graph const & graph() const { return graph_; }
		private:
			class graph & graph_;
	///\}

	///\name ports: outputs
	///\{
		public:
			typedef std::vector<ports::output*> output_ports_type;
			/// the output ports owned by this node
			output_ports_type const & output_ports() const throw() { return output_ports_; }
		private: friend class ports::output;
			output_ports_type output_ports_;
	///\}

	///\name ports: outputs: signals
	///\{
		public:
			/// signal emitted when a new output port is created for this node
			boost::signal<void (ports::output &)> & new_output_port_signal() throw() { return new_output_port_signal_; }
		private:
			boost::signal<void (ports::output &)>   new_output_port_signal_;
	///\}

	///\name ports: inputs
	///\{
		public:
			typedef std::vector<ports::inputs::single*> single_input_ports_type;
			/// the input ports owned by this node
			single_input_ports_type const & single_input_ports() const throw() { return single_input_ports_; }
		private: friend class ports::inputs::single;
			single_input_ports_type single_input_ports_;
	///\}

	///\name ports: inputs: single: signals
	///\{
		public:
			/// signal emitted when a new single input port is created for this node
			boost::signal<void (ports::inputs::single &)> & new_single_input_port_signal() throw() { return new_single_input_port_signal_; }
		private:
			boost::signal<void (ports::inputs::single &)>   new_single_input_port_signal_;
	///\}

	///\name ports: inputs: multiple
	///\{
		public:
			/// the multiple input port owned by this node, if any, or else 0
			ports::inputs::multiple * const multiple_input_port() const throw() { return multiple_input_port_; }
		private: friend class ports::inputs::multiple;
			ports::inputs::multiple * multiple_input_port_;
	///\}

	///\name ports: inputs: multiple: signals
	///\{
		public:
			/// signal emitted when the multiple input port is created for this node
			boost::signal<void (ports::inputs::multiple &)> & new_multiple_input_port_signal() throw() { return new_multiple_input_port_signal_; }
		private:
			boost::signal<void (ports::inputs::multiple &)>   new_multiple_input_port_signal_;
	///\}
};
/// outputs a textual representation of a node.
///\relates node
///\see node::dump
PSYCLE__DECL std::ostream & operator<<(std::ostream &, node const &);

/**********************************************************************************************************************/
// port
/// handles a stream of signal coming to or parting from an engine::node
class PSYCLE__DECL port : public named {
	friend class ports::output;
	friend class ports::input;
	friend class ports::inputs::single;
	friend class ports::inputs::multiple;
	
	public:
		port(class node &, name_type const &, std::size_t channels = 0);
		virtual ~port();

	protected:
		/// connects this port to another port.
		void connect(port &) throw(exception);

	///\name node
	///\{
		public:
			class node & node() { return node_; }
			class node const & node() const { return node_; }
		private:
			class node & node_;
	///\}

	///\name buffer
	///\{
		public:
			/// assigns a buffer to this port (or unassigns if 0).
			void buffer(class buffer *);
			/// the buffer to read or write data from or to (buffers are shared accross several ports).
			class buffer & buffer() const throw() { return *buffer_; }
		private:
			/// the buffer to read or write data from or to (buffers are shared accross several ports).
			class buffer * buffer_;
	///\}

	///\name channels
	///\{
		private:
			std::size_t channels_;
			bool channels_immutable_;
			/// tries to set the channel count of this port and rolls back on failure.
			void channels_transaction(std::size_t) throw(exception);
			/// sets the channel count of this port and propagates it to its node.
			void propagate_channels_to_node(std::size_t) throw(exception);
		protected:
			/// sets the channel count of this port.
			virtual void do_channels(std::size_t) throw(exception);
			/// propagates the channel count to its connected ports.
			virtual void do_propagate_channels() throw(exception) {}
		public:
			/// the channel count of this port.
			std::size_t channels() const { return channels_; }
			/// wether or not the channel count is allowed to be changed.
			bool channels_immutable() { return channels_immutable_; }
			/// sets the channel count of this port and propagates it to both its node and its connected ports.
			///\pre the channel count is not immutable: !channels_immutable()
			void channels(std::size_t) throw(exception);
			/// sets the channel count of this port and propagates it to its connected ports
			void propagate_channels(std::size_t) throw(exception);
			/// copies the channel count of the given port to this port and propagates it to its connected ports
			void propagate_channels(port const & port) throw(exception) { propagate_channels(port.channels()); }
	///\}

	///\name event rate
	///\{
		private:
			real seconds_per_event_;
			/// sets the seconds per event of this port and propagates it to its node.
			void propagate_seconds_per_event_to_node(real const &);
			/// propagates the seconds per event to its connected ports.
			virtual void do_propagate_seconds_per_event() {}
		public:
			/// events per second.
			real events_per_second() const { return 1 / seconds_per_event(); }
			/// sets the events per seconds.
			void events_per_second(real const & events_per_second) { seconds_per_event(1 / events_per_second); }
			/// seconds per event.
			real seconds_per_event() const { return seconds_per_event_; }
			/// sets the seconds per event.
			void seconds_per_event(real const &);
			/// sets the seconds per event of this port and propagates it to its connected ports.
			void propagate_seconds_per_event(real const &);
	///\}

	///\name name
	///\{
		public:
			/// the full path of the node (within its node and graph).
			name_type qualified_name() const;
			/// the      path of the node (within its node).
			name_type semi_qualified_name() const;
	///\}

	/// outputs a textual representation of this port.
	///\see operator<<()
	virtual void dump(std::ostream &, std::size_t tabulations = 0) const;
};
/// outputs a textual representation of a port.
///\relates port
///\see port::dump
PSYCLE__DECL std::ostream & operator<<(std::ostream &, port const &);

namespace ports {

	/**********************************************************************************************************************/
	// output
	/// handles an output stream of signal parting from a node
	class PSYCLE__DECL output : public port {
		friend class graph;
		friend class node;
		friend class input;
		friend class inputs::single;
		friend class inputs::multiple;
			
		public:
			output(class node &, name_type const &, std::size_t channels = 0);
			virtual ~output();

		protected:
			void do_propagate_channels() throw(exception) /*override*/;
			void do_propagate_seconds_per_event() /*override*/;

		public:
			operator bool() const { return input_ports().size(); }
			void dump(std::ostream &, std::size_t tabulations = 0) const /*override*/;
	};
	
	/**********************************************************************************************************************/
	// input
	/// handles an input stream of signal coming to a node.
	class PSYCLE__DECL input : public port {
		friend class node;
		
		public:
			input(class node &, name_type const &, std::size_t channels = 0);
			virtual ~input();

		public:
			void connect(ports::output &) throw(exception);
			void disconnect(ports::output &);

		public:
			operator bool() const {
				class buffer const & b(this->buffer());
				for(std::size_t i(0), e(channels()); i < e; ++i) {
					channel const & c(b[i]);
					if(c.size() && c.size() > c[0].index()) return true;
				}
				return false;
			}
			
		public:
			void dump(std::ostream &, std::size_t tabulations = 0) const = 0;
	};
	
	namespace inputs {

		/**********************************************************************************************************************/
		// single
		/// handles a single input stream of signal coming to a node.
		class PSYCLE__DECL single : public input {
			friend class node;

			public:
				single(class node &, name_type const &, std::size_t channels = 0);
				virtual ~single();

			protected:
				void do_propagate_channels() throw(exception) /*override*/;
				void do_propagate_seconds_per_event() /*override*/;

			public:
				operator bool() const {
					return this->output_port() &&
					static_cast<bool>(static_cast<input const &>(*this));
				}
				void dump(std::ostream &, std::size_t tabulations = 0) const /*override*/;
		};

		/**********************************************************************************************************************/
		// multiple
		/// handles multiple input streams of signal coming to a node.
		class PSYCLE__DECL multiple : public input {
			friend class node;

			public:
				multiple(class node &, name_type const &, bool single_connection_is_identity_transform, std::size_t channels = 0);
				virtual ~multiple();

			protected:
				void do_propagate_channels() throw(exception) /*override*/;
				void do_propagate_seconds_per_event() /*override*/;

			public:
				bool single_connection_is_identity_transform() const throw() { return single_connection_is_identity_transform_; }
			private:
				bool single_connection_is_identity_transform_;

			public:
				operator bool() const {
					return this->output_ports().size() &&
					static_cast<bool>(static_cast<input const &>(*this));
				}
				void dump(std::ostream &, std::size_t tabulations = 0) const /*override*/;
		};
	}
}
}}

#include <psycle/detail/decl.hpp>

/**********************************************************************************************************************/
// implementation details

namespace psycle { namespace engine {

/**********************************************************************************************************************/
// node

void node::process_first() throw(std::exception) {
	if(false && loggers::trace()()) {
		std::ostringstream s;
		s << qualified_name() << " processing node first input";
		loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
	do_process_first();
}

void node::process() throw(std::exception) {
	if(false && loggers::trace()()) {
		std::ostringstream s;
		s << qualified_name() << " processing node";
		loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
	do_process();
}

void node::reset() {
	if(false && loggers::trace()()) {
		std::ostringstream s;
		s << qualified_name() << " resetting node";
		loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
	do_reset();
}

}}
