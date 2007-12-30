// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 psycle development team http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\interface psycle::engine::graph
#pragma once
#include "forward_declarations.hpp"
#include "named.hpp"
#include <psycle/generic/generic.hpp>
#include <set>
#include <mutex>
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__ENGINE__GRAPH
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace psycle { namespace engine {

class plugin_library_reference;

/**********************************************************************************************************************/
// graph
/// a set of nodes
class UNIVERSALIS__COMPILER__DYNAMIC_LINK graph : public typenames::typenames::bases::graph, public named {
	protected: friend class virtual_factory_access;
		graph(name_type const &);
		virtual ~graph();

	public:
		std::mutex       & mutex() const { return mutex_; }
	private:
		std::mutex mutable mutex_;

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
			void virtual dump(std::ostream &, std::size_t tabulations = 0) const;
	///\}
};
/// outputs a textual representation of a graph.
///\relates graph
///\see graph::dump
UNIVERSALIS__COMPILER__DYNAMIC_LINK std::ostream & operator<<(std::ostream & out, graph const &);

/**********************************************************************************************************************/
// port
/// handles a stream of signal coming to or parting from a engine::node
class UNIVERSALIS__COMPILER__DYNAMIC_LINK port : public typenames::typenames::bases::port, public named {
	friend class ports::output;
	friend class ports::input;
	friend class ports::inputs::single;
	friend class ports::inputs::multiple;
	
	protected: friend class virtual_factory_access;
		port(parent_type &, name_type const &, std::size_t channels = 0);
		virtual ~port();

	protected:
		/// connects this port to another port.
		void connect(port &) throw(exception);

	///\name buffer
	///\{
		public:
			/// assigns a buffer to this port (or unassigns if 0) only if the given buffer is different.
			void buffer(typenames::buffer * const);
			/// the buffer to read or write data from or to (buffers are shared accross several ports).
			typenames::buffer & buffer() const throw() { return *buffer_; }
		protected:
			/// assigns a buffer to this port (or unassigns if 0) without checking if the given buffer is different.
			///\pre either the given buffer is 0, or else, this port has not yet been assigned a buffer.
			void virtual do_buffer(typenames::buffer * const);
		private:
			/// the buffer to read or write data from or to (buffers are shared accross several ports).
			typenames::buffer * buffer_;
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
			void virtual do_channels(std::size_t) throw(exception);
			/// propagates the channel count to its connected ports.
			void virtual do_propagate_channels() throw(exception) {}
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
			void virtual do_propagate_seconds_per_event() {}
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
			name_type      qualified_name() const;
			/// the      path of the node (within its node).
			name_type semi_qualified_name() const;
	///\}

	/// outputs a textual representation of this port.
	///\see operator<<()
	void virtual dump(std::ostream &, std::size_t tabulations = 0) const;
};
/// outputs a textual representation of a port.
///\relates port
///\see port::dump
UNIVERSALIS__COMPILER__DYNAMIC_LINK std::ostream & operator<<(std::ostream &, port const &);

namespace ports {

	/**********************************************************************************************************************/
	// output
	/// handles an output stream of signal parting from a node
	class UNIVERSALIS__COMPILER__DYNAMIC_LINK output : public typenames::typenames::bases::ports::output {
		friend class graph;
		friend class node;
		friend class input;
		friend class inputs::single;
		friend class inputs::multiple;
			
		protected: friend class virtual_factory_access;
			output(parent_type &, name_type const &, std::size_t channels = 0);
			virtual ~output();

		protected:
			void do_buffer(engine::buffer * const) /*override*/;
			void do_propagate_channels() throw(exception) /*override*/;
			void do_propagate_seconds_per_event() /*override*/;

		public:
			void dump(std::ostream &, std::size_t tabulations = 0) const /*override*/;
	};
	
	/**********************************************************************************************************************/
	// input
	/// handles an input stream of signal coming to a node.
	class UNIVERSALIS__COMPILER__DYNAMIC_LINK input : public typenames::typenames::bases::ports::input {
		friend class node;
		
		protected: friend class virtual_factory_access;
			input(parent_type &, name_type const &, std::size_t channels = 0);
			virtual ~input();

		public:
			void    connect(typenames::ports::output &) throw(exception);
			void disconnect(typenames::ports::output &);

		public:
			void dump(std::ostream &, std::size_t tabulations = 0) const = 0 /*override*/;
	};
	
	namespace inputs {

		/**********************************************************************************************************************/
		// single
		/// handles a single input stream of signal coming to a node.
		class UNIVERSALIS__COMPILER__DYNAMIC_LINK single : public typenames::typenames::bases::ports::inputs::single {
			friend class node;

			protected: friend class virtual_factory_access;
				single(parent_type &, name_type const &, std::size_t channels = 0);
				virtual ~single();

			protected:
				void do_propagate_channels() throw(exception) /*override*/;
				void do_propagate_seconds_per_event() /*override*/;

			public:
				void dump(std::ostream &, std::size_t tabulations = 0) const /*override*/;
		};

		/**********************************************************************************************************************/
		// multiple
		/// handles multiple input streams of signal coming to a node.
		class UNIVERSALIS__COMPILER__DYNAMIC_LINK multiple : public typenames::typenames::bases::ports::inputs::multiple
		{
			friend class node;

			protected: friend class virtual_factory_access;
				multiple(parent_type &, name_type const &, bool single_connection_is_identity_transform, std::size_t channels = 0);
				virtual ~multiple();

			protected:
				void do_propagate_channels() throw(exception) /*override*/;
				void do_propagate_seconds_per_event() /*override*/;

			public:
				bool single_connection_is_identity_transform() const throw() { return single_connection_is_identity_transform_; }
			private:
				bool single_connection_is_identity_transform_;

			public:
				void dump(std::ostream &, std::size_t tabulations = 0) const /*override*/;
		};
	}
}

/**********************************************************************************************************************/
// node
/// node of a graph, placeholder for a dsp, aka "plugin machine".
class UNIVERSALIS__COMPILER__DYNAMIC_LINK node : public typenames::bases::node, public named {
	friend class graph;
	friend class port;
	friend class ports::output;
	friend class ports::input;
	friend class ports::inputs::single;
	friend class ports::inputs::multiple;
	
	protected: friend class virtual_factory_access;
		node(typenames::plugin_library_reference &, parent_type &, name_type const &);

		void before_destruction() /*override*/ {
			typenames::typenames::bases::node::before_destruction();
			close();
		}

		virtual ~node();

	///\name types
	///\{
		public:
			typedef typenames::graph graph;
			class ports {
				public:
					typedef typenames::ports::output output;
					typedef typenames::ports::input input;
					class inputs {
						public:
							typedef typenames::ports::inputs::single single;
							typedef typenames::ports::inputs::multiple multiple;
					};
			};
			typedef typenames::buffer buffer;
			//this is currently a typedef: class channel;
			//only a forward declaration at this point: typedef typenames::buffer::channel channel;
			typedef typenames::event event;
	///\}
		
	///\name reference to plugin library
	///\{
		public:
			typenames::plugin_library_reference & plugin_library_reference() const throw() { return plugin_library_reference_; }
		private:
			typenames::plugin_library_reference & plugin_library_reference_;
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
			void            open  (          ) throw(std::exception) { if(!opened()) try { do_open(); } catch(...) { do_close(); throw; } }
			void            opened(bool value) throw(std::exception) { if(value) open(); else close(); }
			bool virtual    opened(          ) const;
		protected:
			void virtual do_open() throw(std::exception);
	///\}

	///\name start
	///\{
		public:
			/// called by schedulers
			void            start  (          ) throw(std::exception) { open(); if(!started()) try { do_start(); } catch(...) { do_stop(); throw; } }
			void            started(bool value) throw(std::exception) { if(value) start(); else stop(); }
			bool virtual    started(          ) const;
		protected:
			void virtual do_start() throw(std::exception);
	///\}

	///\name process
	///\{
		public:
			/// called by schedulers
			void inline     process_first() throw(std::exception);
		protected:
			/// this function is the placeholder where to put the dsp algorithm.
			/// re-implement this function in a derived class and put your own code in it.
			void virtual do_process_first() throw(std::exception) {}

		public:
			/// called by schedulers
			void inline     process() throw(std::exception);
		protected:
			/// this function is the placeholder where to put the dsp algorithm.
			/// re-implement this function in a derived class and put your own code in it.
			void virtual do_process() throw(std::exception) = 0;

		public:
			/// called by schedulers, reset the state of this node so that it prepares for the next call to process()
			void inline     reset();
		protected:
			void virtual do_reset() {}
	///\}

	///\name stop
	///\{
		public:
			/// called by schedulers
			void            stop() throw(std::exception) { if(started()) do_stop(); }
		protected:
			void virtual do_stop() throw(std::exception);
	///\}
	
	///\name close
	///\{
		public:
			/// called by schedulers
			void            close() throw(std::exception) { stop(); if(opened()) do_close(); }
		protected:
			void virtual do_close() throw(std::exception);
	///\}

	protected:
		void virtual channel_change_notification_from_port(port const &) throw(std::exception) {}
		void virtual seconds_per_event_change_notification_from_port(port const &) {}

	///\name name
	///\{
		public:
			/// the full path of the node (within its graph)
			name_type qualified_name() const;
			void virtual dump(std::ostream &, std::size_t tabulations = 0) const;
	///\}
};
/// outputs a textual representation of a node.
///\relates node
///\see node::dump
UNIVERSALIS__COMPILER__DYNAMIC_LINK std::ostream & operator<<(std::ostream &, node const &);
}}
#include <universalis/compiler/dynamic_link/end.hpp>

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

