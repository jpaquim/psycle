// -*- mode:c++; indent-tabs-mode:t -*-
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2008 psycle development team http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\interface psycle::host::schedulers::multi_threaded
#pragma once
#include "../scheduler.hpp"
#include <psycle/generic/wrappers.hpp>
#include <thread>
#include <date_time>
#include <mutex>
#include <condition>
#include <list>
#include <cstdint>
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__HOST__SCHEDULERS__MULTI_THREADED
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace psycle { namespace host { namespace schedulers {
/// a scheduler using several threads
namespace multi_threaded {

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
class scheduler;

namespace typenames {
	using namespace multi_threaded;
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
		graph(underlying_type & underlying) : graph_base(underlying) {}

	///\name schedule
	///\{
		public:
			void compute_plan();
			void clear_plan();
			
		public:
			/// maximum number of channels needed for buffers
			std::size_t channels() const throw() { return channels_; }
		private:
			std::size_t channels_;
			
		public:
			typedef std::list<node*> terminal_nodes_type;
			/// nodes with no dependency, that are processed first
			terminal_nodes_type const & terminal_nodes() const throw() { return terminal_nodes_; }
		private:
			terminal_nodes_type terminal_nodes_;

		public:
			boost::signal<void (node & node)> & io_ready_signal() throw() { return io_ready_signal_; }
		private:
			boost::signal<void (node &)> io_ready_signal_;
	///\}
};

/**********************************************************************************************************************/
// port
typedef generic::wrappers::port<typenames::typenames> port_base;
class UNIVERSALIS__COMPILER__DYNAMIC_LINK port : public port_base {
	protected: friend class virtual_factory_access;
		port(parent_type & parent, underlying_type & underlying) : port_base(parent, underlying) {}

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
			output(parent_type & parent, underlying_type & underlying) : output_base(parent, underlying) {}
	};

	/**********************************************************************************************************************/
	// input
	typedef generic::wrappers::ports::input<typenames::typenames> input_base;
	class UNIVERSALIS__COMPILER__DYNAMIC_LINK input : public input_base {
		protected: friend class virtual_factory_access;
			input(parent_type & parent, underlying_type & underlying) : input_base(parent, underlying) {}
	};

	namespace inputs {

		/**********************************************************************************************************************/
		// single
		typedef generic::wrappers::ports::inputs::single<typenames::typenames> single_base;
		class UNIVERSALIS__COMPILER__DYNAMIC_LINK single : public single_base {
			protected: friend class virtual_factory_access;
				single(parent_type & parent, underlying_type & underlying) : single_base(parent, underlying) {}
		};

		/**********************************************************************************************************************/
		// multiple
		typedef generic::wrappers::ports::inputs::multiple<typenames::typenames> multiple_base;
		class UNIVERSALIS__COMPILER__DYNAMIC_LINK multiple : public multiple_base {
			protected: friend class virtual_factory_access;
				multiple(parent_type & parent, underlying_type & underlying) : multiple_base(parent, underlying) {}
		};
	}
}

/**********************************************************************************************************************/
// node
typedef generic::wrappers::node<typenames::typenames> node_base;
class UNIVERSALIS__COMPILER__DYNAMIC_LINK node : public node_base {
	protected: friend class virtual_factory_access;
		node(parent_type &, underlying_type &);
		
	///\name schedule
	///\{
		public:
			void compute_plan();
			void reset() throw() /*override*/;
			/// called each time a direct predecessor node has been processed
			void predecessor_node_processed() { assert(predecessor_node_remaining_count_); --predecessor_node_remaining_count_; }
			/// indicates whether all the predecessors of this node have been processed
			bool is_ready_to_process() { return !predecessor_node_remaining_count_; }
		private:
			std::size_t predecessor_node_count_;
			std::size_t predecessor_node_remaining_count_;

		public:  ports::output & multiple_input_port_first_output_port_to_process() throw() { assert(multiple_input_port_first_output_port_to_process_); return *multiple_input_port_first_output_port_to_process_; }
		private: ports::output * multiple_input_port_first_output_port_to_process_;

		private:
			/// connection to the underlying signal
			boost::signals::scoped_connection on_underlying_io_ready_signal_connection;
			/// signal slot for the underlying signal
			void on_underlying_io_ready(node::underlying_type & /*underlying_node*/) { parent().io_ready_signal()(*this); }
			
		public:
			bool waiting_for_io_ready_signal() const throw() { return waiting_for_io_ready_signal_; }
			void waiting_for_io_ready_signal(bool waiting_for_io_ready_signal) throw() { waiting_for_io_ready_signal_ = waiting_for_io_ready_signal; }
		private:
			bool waiting_for_io_ready_signal_;

		public:  void process_first() { process(true); }
		public:  void process() { process(false); }
		private: void process(bool first);

		public:  bool const processed() const throw() { return processed_; }
		private: bool       processed_;
	///\}
		
	///\name schedule ... time measurement (to be used for heuristics)
	///\{
		public:  void reset_time_measurement();

		public:  std::nanoseconds accumulated_processing_time() throw() { return accumulated_processing_time_; }
		private: std::nanoseconds accumulated_processing_time_;

		public:  std::uint64_t processing_count() throw() { return processing_count_; }
		private: std::uint64_t processing_count_;

		public:  std::uint64_t processing_count_no_zeroes() throw() { return processing_count_no_zeroes_; }
		private: std::uint64_t processing_count_no_zeroes_;
	///\}
};

/**********************************************************************************************************************/
/// a scheduler using several threads
class UNIVERSALIS__COMPILER__DYNAMIC_LINK scheduler : public host::scheduler<graph> {
	public:
		scheduler(graph::underlying_type &, std::size_t threads = 1) throw(std::exception);
		virtual ~scheduler() throw();
		void start() throw(underlying::exception) /*override*/;
		bool started() /*override*/ { return threads_.size(); }
		void started(bool started) { host::scheduler<typenames::graph>::started(started); }
		void stop() /*override*/;
		
		std::size_t threads() const throw() { return thread_count_; }
		void        threads(std::size_t threads);

	///\name signal slots and connections
	///\{
		private:
			boost::signals::scoped_connection on_new_node_signal_connection;
			void on_new_node(node::underlying_type &);
			
			boost::signals::scoped_connection on_delete_node_signal_connection;
			void on_delete_node(node::underlying_type &);

			boost::signals::scoped_connection on_new_connection_signal_connection;
			void on_new_connection(ports::input::underlying_type &, ports::output::underlying_type &);

			boost::signals::scoped_connection on_delete_connection_signal_connection;
			void on_delete_connection(ports::input::underlying_type &, ports::output::underlying_type &);

			boost::signals::scoped_connection on_io_ready_signal_connection;
			void on_io_ready(node &);
	///\}

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
		
		std::size_t thread_count_;
		typedef std::list<std::thread *> threads_type;
		threads_type threads_;
		void thread_function(std::size_t thread_number);

		typedef std::scoped_lock<std::mutex> scoped_lock;
		std::mutex mutable mutex_;
		std::condition<scoped_lock> mutable condition_;
		
		bool stop_requested_;
		bool suspend_requested_;
		std::size_t suspended_;
		
		typedef std::list<node*> nodes_queue_type;
		/// nodes ready to be processed, just waiting for a free thread
		nodes_queue_type nodes_queue_;
		/// nodes ready to be processed initially (leaves), just waiting for a free thread
		nodes_queue_type initial_nodes_queue_;
		
		std::size_t processed_node_count_;

		void suspend_and_compute_plan();
		void compute_plan();
		void clear_plan();

		void process_loop();
		void process(node &);
		void set_buffer_for_output_port(ports::output &, buffer &);
		void set_buffers_for_all_output_ports_of_node_from_buffer_pool(node &);
		void check_whether_to_recycle_buffer_in_the_pool(buffer &);
};

}}}}
#include <universalis/compiler/dynamic_link/end.hpp>
