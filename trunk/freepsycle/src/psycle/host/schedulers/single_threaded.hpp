/* -*- mode:c++, indent-tabs-mode:t -*- */
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 psycle development team http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\interface psycle::host::schedulers::single_threaded
#pragma once
#include "../scheduler.hpp"
#include <psycle/generic/wrappers.hpp>
#include <thread>
#include <mutex>
#include <list>
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__HOST__SCHEDULERS__SINGLE_THREADED
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace psycle { namespace host { namespace schedulers {
/// a scheduler using only one thread
namespace single_threaded {

namespace underlying = host::underlying;

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

namespace typenames {
	using namespace single_threaded;
	class typenames : public generic::typenames<graph, node, port, ports::output, ports::input, ports::inputs::single, ports::inputs::multiple, underlying::typenames::typenames> {};
}

/**********************************************************************************************************************/
/// underlying::buffer with a reference counter.
class buffer : public underlying::buffer {
	public:
		/// creates a buffer with an initial reference count set to 0.
		buffer(std::size_t channels, std::size_t events) throw(std::exception);
		/// deletes the buffer
		///\pre the reference count must be 0.
		virtual ~buffer() throw();
		/// convertible to std::size_t
		///\returns the reference count.
		std::size_t reference_count() const throw() { return reference_count_; }
		/// increments the reference count.
		buffer & operator+=(std::size_t more) throw() { reference_count_ += more; return *this; }
		/// decrements the reference count by 1.
		buffer & operator--() throw() { assert(this->reference_count() > 0); --reference_count_; return *this; }
	private:
		std::size_t reference_count_;
};

/**********************************************************************************************************************/
// graph
typedef generic::wrappers::graph<typenames::typenames> graph_base;
class UNIVERSALIS__COMPILER__DYNAMIC_LINK graph : public graph_base {
	protected: friend class virtual_factory_access;
		graph(underlying_type &);
		void after_construction() /*override*/;

	///\name signal slots
	///\{
		private:
			void on_new_node(typenames::node &);
	///\}
};

/**********************************************************************************************************************/
// port
typedef generic::wrappers::port<typenames::typenames> port_base;
class UNIVERSALIS__COMPILER__DYNAMIC_LINK port : public port_base {
	protected: friend class virtual_factory_access;
		port(parent_type &, underlying_type &);

	///\name buffer
	///\{
		public:
			/// assigns a buffer to this port (or unassigns if 0) only if the given buffer is different.
			void buffer(typenames::buffer * const buffer) { underlying().buffer(buffer); }
			/// the buffer to read or write data from or to (buffers are shared accross several ports).
			typenames::buffer & buffer() const throw() { return static_cast<typenames::buffer &>(underlying().buffer()); }
	///\}
};

namespace ports {

	/**********************************************************************************************************************/
	// output
	typedef generic::wrappers::ports::output<typenames::typenames> output_base;
	class UNIVERSALIS__COMPILER__DYNAMIC_LINK output : public output_base {
		protected: friend class virtual_factory_access;
			output(parent_type &, underlying_type &);
		
		///\name schedule
		///\{
			public:  std::size_t input_port_count() const throw() { return input_port_count_; }
			private: std::size_t input_port_count_;

			public:
				/// convertible to std::size_t
				///\returns the reference count.
				std::size_t input_ports_remaining() const throw() { return input_ports_remaining_; }
				output & operator--() throw() { assert(this->input_ports_remaining() > 0); --input_ports_remaining_; return *this; }
				void reset() throw() { input_ports_remaining_ = input_port_count(); }
			private:
				std::size_t input_ports_remaining_;
		///\}
	};

	/**********************************************************************************************************************/
	// input
	typedef generic::wrappers::ports::input<typenames::typenames> input_base;
	class UNIVERSALIS__COMPILER__DYNAMIC_LINK input : public input_base {
		protected: friend class virtual_factory_access;
			input(parent_type &, underlying_type &);
	};

	namespace inputs {

		/**********************************************************************************************************************/
		// single
		typedef generic::wrappers::ports::inputs::single<typenames::typenames> single_base;
		class UNIVERSALIS__COMPILER__DYNAMIC_LINK single : public single_base {
			protected: friend class virtual_factory_access;
				single(parent_type &, underlying_type &);
		};

		/**********************************************************************************************************************/
		// multiple
		typedef generic::wrappers::ports::inputs::multiple<typenames::typenames> multiple_base;
		class UNIVERSALIS__COMPILER__DYNAMIC_LINK multiple : public multiple_base {
			protected: friend class virtual_factory_access;
				multiple(parent_type &, underlying_type &);
		};
	}
}

/**********************************************************************************************************************/
// node
typedef generic::wrappers::node<typenames::typenames> node_base;
class UNIVERSALIS__COMPILER__DYNAMIC_LINK node : public node_base {
	protected: friend class virtual_factory_access;
		node(parent_type &, underlying_type &);
		void after_construction() /*override*/; friend class graph; // init code moved to graph since it deals with connections
		
	///\name signal slots
	///\{
		private:
			void on_new_output_port(typenames::ports::output &);
			void on_new_single_input_port(typenames::ports::inputs::single &);
			void on_new_multiple_input_port(typenames::ports::inputs::multiple &);
	///\}
	
	///\name schedule
	///\{
		public:  ports::output & multiple_input_port_first_output_port_to_process() throw() { assert(multiple_input_port_first_output_port_to_process_); return *multiple_input_port_first_output_port_to_process_; }
		private: ports::output * multiple_input_port_first_output_port_to_process_;

		public:  std::size_t output_port_count() const throw() { return output_port_count_; }
		private: std::size_t output_port_count_;
		
		public:  void reset() throw() /*override*/ { assert(processed()); processed(false); underlying().reset(); }
		public:  void mark_as_processed() throw() { processed(true); }
		public:  void         processed(bool processed) throw() { assert(this->processed() != processed); this->processed_ = processed; assert(this->processed() == processed); }
		public:  bool const & processed() const throw() { return processed_; }
		private: bool         processed_;
	///\}
};

/**********************************************************************************************************************/
/// a scheduler using only one thread
class UNIVERSALIS__COMPILER__DYNAMIC_LINK scheduler : public host::scheduler<graph> {
	public:
		scheduler(graph::underlying_type &) throw(std::exception);
		virtual ~scheduler() throw();
		void start() throw(underlying::exception) /*override*/;
		void stop() /*override*/;
		void operator()();
	private:
		/// Flyweight pattern [Gamma95].
		/// a pool of buffers that can be used for input and output ports of the nodes of the graph.
		class buffer_pool : protected std::list<buffer*> {
			public:
				buffer_pool(std::size_t channels, std::size_t events) throw(std::exception);
				virtual ~buffer_pool() throw();
				/// gets a buffer from the pool.
				buffer & operator()() {
					if(false && loggers::trace()()) {
						std::ostringstream s;
						s << "buffer requested, pool size before: " << size();
						loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
					}
					if(empty()) return *new buffer(channels_, events_);
					buffer & result(*back());
					assert("reference count is zero: " && !result.reference_count());
					pop_back(); // note: on most implementations, this will not realloc memory, so it's realtime-safe.
					return result;
				}
				/// recycles a buffer in the pool.
				void operator()(buffer & buffer) {
					assert(&buffer);
					assert("reference count is zero: " && !buffer.reference_count());
					assert(buffer.channels() >= this->channels_);
					assert(buffer.events() >= this->events_);
					if(false && loggers::trace()()) {
						std::ostringstream s;
						s << "buffer " << &buffer << " given back, pool size before: " << size();
						loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
					}
					push_back(&buffer); // note: does non-realtime realloc
				}
			private:
				std::size_t channels_, events_;
		} * buffer_pool_instance_;
		buffer_pool & buffer_pool_instance() throw() { return *buffer_pool_instance_; }
		std::thread * thread_;
		std::mutex mutable mutex_;
		bool stop_requested_;
		bool stop_requested();
		void process_loop();
		void process(node &);
		void process_node_of_output_port_and_set_buffer_for_input_port(ports::output &, ports::input &);
		void set_buffer_for_output_port(ports::output &, buffer &);
		void set_buffers_for_all_output_ports_of_node_from_buffer_pool(node &);
		void mark_buffer_as_read_once_more_and_check_whether_to_recycle_it_in_the_pool(ports::output &, ports::input &);
		void check_whether_to_recycle_buffer_in_the_pool(ports::output &);
		typedef std::list<node*> terminal_nodes_type;
		terminal_nodes_type terminal_nodes_;
		void allocate() throw(std::exception);
		void free() throw();
};

}}}}
#include <universalis/compiler/dynamic_link/end.hpp>

