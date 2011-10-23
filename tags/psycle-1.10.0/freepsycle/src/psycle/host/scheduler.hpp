// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2011 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

#pragma once
#include <psycle/engine/graph.hpp>
#include <universalis/stdlib/thread.hpp>
#include <universalis/stdlib/mutex.hpp>
#include <universalis/stdlib/condition_variable.hpp>
#include <universalis/stdlib/chrono.hpp>
#include <set>
#include <list>
#define PSYCLE__DECL  PSYCLE__HOST
#include <psycle/detail/decl.hpp>
namespace psycle { namespace host {

using namespace universalis::stdlib;
using engine::exception;

class scheduler;
class node;
namespace ports {
	class output;
}

class node {
	public:
		node(class scheduler &, engine::node &);

	///\name scheduler
	///\{
		public:
			class scheduler const & scheduler() const { return scheduler_; }
			class scheduler & scheduler() { return scheduler_; }
		private:
			class scheduler & scheduler_;
	///\}

	///\name engine
	///\{
		public:
			engine::node const & engine() const { return engine_; }
			engine::node & engine() { return engine_; }
		private:
			engine::node & engine_;
	///\}

	///\name output ports
	///\{
		public:
			typedef std::vector<ports::output*> output_ports_type;
			/// the output ports owned by this node
			output_ports_type const & output_ports() const { return output_ports_; }
		private:
			output_ports_type output_ports_;
	///\}

	///\name schedule
	///\{
		public:
			void compute_plan();
			void reset();
			/// called each time a direct predecessor node has been processed
			void predecessor_node_processed() { assert(predecessor_node_remaining_count_); --predecessor_node_remaining_count_; }
			/// indicates whether all the predecessors of this node have been processed
			bool is_ready_to_process() const { return !predecessor_node_remaining_count_; }
		private:
			std::size_t predecessor_node_count_;
			std::size_t predecessor_node_remaining_count_;

		public:  engine::ports::output & multiple_input_port_first_output_port_to_process() { assert(multiple_input_port_first_output_port_to_process_); return *multiple_input_port_first_output_port_to_process_; }
		private: engine::ports::output * multiple_input_port_first_output_port_to_process_;

		private:
			/// connection to the engine signal
			boost::signals::scoped_connection on_engine_io_ready_signal_connection;
			/// signal slot for the engine signal
			void inline on_engine_io_ready(engine::node &);

		public:
			bool waiting_for_io_ready_signal() const { return waiting_for_io_ready_signal_; }
			void waiting_for_io_ready_signal(bool value) { waiting_for_io_ready_signal_ = value; }
		private:
			bool waiting_for_io_ready_signal_;

		public:
			void process(bool first);
			bool processed() const { return processed_; }
		private:
			bool processed_;
	///\}

	///\name schedule ... time measurement
	///\{
		public:  void reset_time_measurement();

		public:  chrono::nanoseconds accumulated_processing_time() const { return accumulated_processing_time_; }
		private: chrono::nanoseconds accumulated_processing_time_;

		public:  uint64_t processing_count() const { return processing_count_; }
		private: uint64_t processing_count_;

		public:  uint64_t processing_count_no_zeroes() const { return processing_count_no_zeroes_; }
		private: uint64_t processing_count_no_zeroes_;
	///\}
};

namespace ports {
	class output {
		public:
			output(engine::ports::output & engine) : engine_(engine) {}

		///\name engine
		///\{
			public:
				engine::ports::output const & engine() const { return engine_; }
				engine::ports::output & engine() { return engine_; }
			private:
				engine::ports::output & engine_;
		///\}

		///\name connected nodes
		///\{
			public:
				typedef std::vector<node*> connected_nodes_type;
				connected_nodes_type const & connected_nodes() const { return connected_nodes_; }
				connected_nodes_type & connected_nodes() { return connected_nodes_; }
			private:
				connected_nodes_type connected_nodes_;
		///\}
	};
}

/**********************************************************************************************************************/
/// buffer with a reference counter.
class buffer : public engine::buffer {
	public:
		/// creates a buffer with an initial reference count set to 0.
		buffer(std::size_t channels, std::size_t events) throw(std::exception);
		/// deletes the buffer
		///\pre the reference count must be 0.
		virtual ~buffer() throw();
		/// convertible to std::size_t
		///\returns the reference count.
		std::size_t reference_count() const { return reference_count_; }
		/// increments the reference count.
		buffer & operator+=(std::size_t more) { reference_count_ += more; return *this; }
		/// decrements the reference count by 1.
		buffer & operator--() { assert(reference_count_); --reference_count_; return *this; }
	private:
		std::size_t reference_count_;
};

/**********************************************************************************************************************/
/// a scheduler using several threads
class PSYCLE__DECL scheduler : public std::set<node*> {
	public:
		scheduler(engine::graph &, std::size_t threads = 1);
		virtual ~scheduler();

	///\name engine
	///\{
		public:
			engine::graph const & engine() const { return engine_; }
			engine::graph & engine() { return engine_; }
		private:
			engine::graph & engine_;
	///\}

	public:
		void start();
		bool started() { return threads_.size(); }
		void started(bool value) { if(value) start(); else stop(); }
		void stop();
		
		std::size_t threads() const { return thread_count_; }
		void threads(std::size_t threads);

	///\name signal slots and connections
	///\{
		private:
			boost::signals::scoped_connection on_new_node_signal_connection;
			void on_new_node(engine::node &);
			
			boost::signals::scoped_connection on_delete_node_signal_connection;
			void on_delete_node(engine::node &);

			boost::signals::scoped_connection on_new_connection_signal_connection;
			void on_new_connection(engine::ports::input &, engine::ports::output &);

			boost::signals::scoped_connection on_delete_connection_signal_connection;
			void on_delete_connection(engine::ports::input &, engine::ports::output &);

			boost::signals::scoped_connection on_io_ready_signal_connection;
			void on_io_ready(node &);
	///\}

	///\name schedule
	///\{
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
			boost::signal<void (node &)> & io_ready_signal() throw() { return io_ready_signal_; }
		private:
			boost::signal<void (node &)> io_ready_signal_;
	///\}

	private:
		class buffer_pool;
		buffer_pool * buffer_pool_;
		
		std::size_t thread_count_;
		typedef std::list<thread *> threads_type;
		threads_type threads_;
		void thread_function(std::size_t thread_number);

		mutex mutable mutex_;
		typedef unique_lock<mutex> scoped_lock;
		condition_variable mutable condition_;
		
		bool stop_requested_;
		bool suspend_requested_;
		std::size_t suspended_;
		
		typedef std::list<node*> nodes_queue_type;
		/// nodes ready to be processed, just waiting for a free thread
		nodes_queue_type nodes_queue_;
		
		std::size_t processed_node_count_;

		void suspend_and_compute_plan();
		void compute_plan();
		void clear_plan();

		void process_loop();
		void process(node &);
		void set_buffer_for_output_port(engine::ports::output &, buffer &);
		void set_buffers_for_all_output_ports_of_node_from_buffer_pool(engine::node &);
		void check_whether_to_recycle_buffer_in_the_pool(buffer &);
};

/// Flyweight pattern [Gamma95].
/// a pool of buffers that can be used for input and output ports of the nodes of the graph.
class scheduler::buffer_pool {
	public:
		buffer_pool(std::size_t channels, std::size_t events);
		~buffer_pool();
		/// gets a buffer from the pool.
		buffer & operator()();
		/// recycles a buffer in the pool.
		void operator()(buffer &);
	private:
		typedef std::list<buffer*> list_type;
		list_type list_;
		std::size_t channels_, events_;
		mutex mutable mutex_;
};

void inline node::on_engine_io_ready(engine::node &) { scheduler_.io_ready_signal()(*this); }

}}
#include <psycle/detail/decl.hpp>
