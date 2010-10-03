// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\interface psycle::host::scheduler
#ifndef PSYCLE__HOST__SCHEDULER__INCLUDED
#define PSYCLE__HOST__SCHEDULER__INCLUDED
#pragma once
#include <universalis/stdlib/thread.hpp>
#include <universalis/stdlib/mutex.hpp>
#include <universalis/stdlib/condition.hpp>
#include <universalis/stdlib/date_time.hpp>
#include <list>
#define PSYCLE__DECL  PSYCLE__HOST
#include <psycle/detail/decl.hpp>
namespace psycle { namespace host {

using namespace universalis::stdlib;
using engine::exception;

class port;
namespace ports {
	class output;
	class input;
}

class node {
	public:
		node(engine::node &);

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
			void on_underlying_io_ready(engine::node &) { graph().io_ready_signal()(*this); }

		public:
			bool waiting_for_io_ready_signal() const throw() { return waiting_for_io_ready_signal_; }
			void waiting_for_io_ready_signal(bool value) throw() { waiting_for_io_ready_signal_ = value; }
		private:
			bool waiting_for_io_ready_signal_;

		public:  void process_first() { process(true); }
		public:  void process() { process(false); }
		private: void process(bool first);

		public:  bool const processed() const throw() { return processed_; }
		private: bool       processed_;
	///\}

	///\name schedule ... time measurement
	///\{
		public:  void reset_time_measurement();

		public:  nanoseconds accumulated_processing_time() const throw() { return accumulated_processing_time_; }
		private: nanoseconds accumulated_processing_time_;

		public:  uint64_t processing_count() const throw() { return processing_count_; }
		private: uint64_t processing_count_;

		public:  uint64_t processing_count_no_zeroes() const throw() { return processing_count_no_zeroes_; }
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

		///\name connected input ports
		///\{
			public:
				typedef std::vector<ports::input*> input_ports_type;
				input_ports_type const & input_ports() const throw() { return input_ports_; }
			private:
				input_ports_type input_ports_;
		///\}
	};

	class input {
		public:
			input(class node & node, engine::ports::input & engine) : node_(node), engine_(engine) {}

		///\name node
		///\{
			public:
				class node const & node() { return node_; }
				class node & node() { return node_; }
			private:
				class node & node_;
		///\}

		///\name engine
		///\{
			public:
				engine::ports::input const & engine() const { return engine_; }
				engine::ports::input & engine() { return engine_; }
			private:
				engine::ports::input & engine_;
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
		buffer & operator--() { assert(this->reference_count() > 0); --reference_count_; return *this; }
	private:
		std::size_t reference_count_;
};

/**********************************************************************************************************************/
/// a scheduler using several threads
class PSYCLE__DECL scheduler {
	public:
		scheduler(engine::graph &, std::size_t threads = 1) throw(std::exception);
		virtual ~scheduler() throw();

	///\name engine
	///\{
		public:
			engine::graph const & engine() const { return engine_; }
			engine::graph & engine() { return engine_; }
		private:
			engine::graph & engine_;
	///\}

	public:
		void start() throw(exception) /*override*/;
		bool started() /*override*/ { return threads_.size(); }
		void started(bool value) throw(exception) { if(value) start(); else stop(); }
		void stop() /*override*/;
		
		std::size_t threads() const throw() { return thread_count_; }
		void        threads(std::size_t threads);

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

	private:
		class buffer_pool;
		buffer_pool * buffer_pool_;
		
		std::size_t thread_count_;
		typedef std::list<thread *> threads_type;
		threads_type threads_;
		void thread_function(std::size_t thread_number);

		typedef class scoped_lock<mutex> scoped_lock;
		mutex mutable mutex_;
		condition<scoped_lock> mutable condition_;
		
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

		void process_loop() throw(std::exception);
		void process(node &) throw(std::exception);
		void set_buffer_for_output_port(ports::output &, buffer &);
		void set_buffers_for_all_output_ports_of_node_from_buffer_pool(node &);
		void check_whether_to_recycle_buffer_in_the_pool(buffer &);
};

/// Flyweight pattern [Gamma95].
/// a pool of buffers that can be used for input and output ports of the nodes of the graph.
class scheduler::buffer_pool {
	public:
		buffer_pool(std::size_t channels, std::size_t events) throw(std::exception);
		~buffer_pool() throw();
		/// gets a buffer from the pool.
		buffer & operator()() {
			scoped_lock lock(mutex_);
			if(false && loggers::trace()) {
				std::ostringstream s;
				s << "buffer requested, pool size before: " << list_.size();
				loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
			if(list_.empty()) return *new buffer(channels_, events_);
			buffer & result(*list_.back());
			assert("reference count is zero: " && !result.reference_count());
			list_.pop_back(); // note: on most implementations, this will not realloc memory, so might be realtime-safe. (looks like it's not on gnu libstdc++)
			return result;
		}
		/// recycles a buffer in the pool.
		void operator()(buffer & buffer) {
			assert(&buffer);
			assert("reference count is zero: " && !buffer.reference_count());
			assert(buffer.channels() >= this->channels_);
			assert(buffer.events() >= this->events_);
			scoped_lock lock(mutex_);
			if(false && loggers::trace()) {
				std::ostringstream s;
				s << "buffer " << &buffer << " given back, pool size before: " << list_.size();
				loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
			list_.push_back(&buffer); // note: does non-realtime realloc
		}
	private:
		typedef std::list<buffer*> list_type;
		list_type list_;
		std::size_t channels_, events_;
		mutex mutable mutex_;
};

}}
#include <psycle/detail/decl.hpp>
#endif
