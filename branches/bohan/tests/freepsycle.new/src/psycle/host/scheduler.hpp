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

	protected:
		engine::graph const & engine() const { return engine_; }
		engine::graph & engine() { return engine_; }
	private:
		engine::graph & engine_;

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
