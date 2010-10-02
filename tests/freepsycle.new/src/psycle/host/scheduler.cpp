// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\implementation psycle::host::scheduler
#include <psycle/detail/project.private.hpp>
#include "scheduler.hpp"
#include <universalis/os/clocks.hpp>
#include <universalis/os/thread_name.hpp>
#include <sstream>
#include <limits>
namespace psycle { namespace host {

using engine::exceptions::runtime_error;

namespace {
	nanoseconds cpu_time_clock() {
		#if 0
			return hiresolution_clock<utc_time>::universal_time().nanoseconds_since_epoch();
		#elif 0
			return universalis::os::clocks::thread_cpu_time::current();
		#elif 0
			return universalis::os::clocks::process_cpu_time::current();
		#else
			return universalis::os::clocks::monotonic::current();
		#endif
	}

	static UNIVERSALIS__COMPILER__THREAD_LOCAL_STORAGE bool this_thread_suspended_ = false;
}

/**********************************************************************************************************************/
// scheduler

scheduler::scheduler(engine::graph & graph, std::size_t threads) throw(std::exception)
:
	// register to the graph signals
	on_new_node_signal_connection(graph.new_node_signal().connect(
		boost::bind(&scheduler::on_new_node, this, _1))
	),
	on_delete_node_signal_connection(graph.delete_node_signal().connect(
		boost::bind(&scheduler::on_delete_node, this, _1))
	),
	on_new_connection_signal_connection(graph.new_connection_signal().connect(
		boost::bind(&scheduler::on_new_connection, this, _1, _2))
	),
	on_delete_connection_signal_connection(graph.delete_connection_signal().connect(
		boost::bind(&scheduler::on_delete_connection, this, _1, _2))
	),
	buffer_pool_(),
	thread_count_(threads)
{}

scheduler::~scheduler() throw() {
	stop();
	delete &graph(); // note that this doesn't delete the underying graph, only the scheduler's wrapping layer
}

void scheduler::on_new_node(engine::node &) {
	suspend_and_compute_plan();
}

void scheduler::on_delete_node(engine::node &) {
	suspend_and_compute_plan();
}

void scheduler::on_new_connection(engine::ports::input &, engine::ports::output &) {
	suspend_and_compute_plan();
}

void scheduler::on_delete_connection(engine::ports::input &, engine::ports::output &) {
	suspend_and_compute_plan();
}

void scheduler::threads(std::size_t threads) {
	// It could be achieved without recreating all the threads, but to keep it simple, we recreate them all.
	bool const was_started(started());
	if(was_started) stop();
	thread_count_ = threads;
	if(was_started) start();
}

void scheduler::start() throw(engine::exception) {
	if(loggers::information()()) loggers::information()("starting scheduler threads on graph " + graph().engine().name() + " ...", UNIVERSALIS__COMPILER__LOCATION);
	if(threads_.size()) {
		if(loggers::information()()) loggers::information()("scheduler threads are already running", UNIVERSALIS__COMPILER__LOCATION);
		return;
	}

	compute_plan();

	// ensure the nodes are started
	// iterate over all the nodes in the graph
	for(graph::const_iterator i(graph().begin()), e(graph().end()); i != e; ++i)
		(**i).engine().start();

	stop_requested_ = suspend_requested_ = false;
	processed_node_count_ = suspended_ = 0;
	on_io_ready_signal_connection = graph().io_ready_signal().connect(boost::bind(&scheduler::on_io_ready, this, _1));

	try {
		// start the scheduling threads
		if(loggers::information()()) {
			std::ostringstream s;
			s << "using " << thread_count_ << " threads";
			loggers::information()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
		for(std::size_t i(0); i < thread_count_; ++i)
			threads_.push_back(new std::thread(boost::bind(&scheduler::thread_function, this, i)));
	} catch(...) {
		{ scoped_lock lock(mutex_);
			stop_requested_ = true;
			on_io_ready_signal_connection.disconnect();
		}
		condition_.notify_all();
		for(threads_type::const_iterator i(threads_.begin()), e(threads_.end()); i != e; ++i) {
			(**i).join();
			delete *i;
		}
		threads_.clear();
		// ensure the nodes are stopped
		// iterate over all the nodes in the graph
		for(graph::const_iterator i(graph().begin()), e(graph().end()); i != e; ++i)
			(**i).engine().stop();
		clear_plan();
		throw;
	}
}

void scheduler::suspend_and_compute_plan() {
	if(!started()) {
		compute_plan();
		return;
	}
	{ scoped_lock lock(mutex_);
		suspend_requested_ = true;
		on_io_ready_signal_connection.block();
	}
	condition_.notify_all();
	{ scoped_lock lock(mutex_);
		while(suspended_ != threads_.size()) condition_.wait(lock);
		compute_plan();
		suspend_requested_ = false;
		on_io_ready_signal_connection.unblock();
	}
	condition_.notify_all();
}

void scheduler::compute_plan() {
	graph().compute_plan();

	// copy the initial processing queue
	nodes_queue_ = graph().terminal_nodes();

	delete buffer_pool_;
	buffer_pool_ = new buffer_pool(graph().channels(), graph().engine().events_per_buffer());
}

void scheduler::clear_plan() {
	delete buffer_pool_; buffer_pool_ = 0;
	nodes_queue_.clear();
	graph().clear_plan();
}

void scheduler::stop() {
	if(loggers::information()()) loggers::information()("terminating and joining scheduler threads ...", UNIVERSALIS__COMPILER__LOCATION);
	if(!threads_.size()) {
		if(loggers::information()()) loggers::information()("scheduler threads were not running", UNIVERSALIS__COMPILER__LOCATION);
		return;
	}
	{ scoped_lock lock(mutex_);
		stop_requested_ = true;
		on_io_ready_signal_connection.disconnect();
	}
	condition_.notify_all();
	for(threads_type::const_iterator i(threads_.begin()), e(threads_.end()); i != e; ++i) {
		(**i).join();
		delete *i;
	}
	if(loggers::information()()) loggers::information()("scheduler threads joined", UNIVERSALIS__COMPILER__LOCATION);
	threads_.clear();
	// ensure the nodes are stopped
	// iterate over all the nodes in the graph
	for(graph::const_iterator i(graph().begin()), e(graph().end()); i != e; ++i)
		(**i).engine().stop();
	clear_plan();

	// dump time measurements
	std::cout << "time measurements: \n";
	nanoseconds total;
	for(graph::const_iterator i(graph().begin()), e(graph().end()); i != e; ++i) {
		node & node(**i);
		std::cout
			<< node.engine().qualified_name()
			<< " (" << universalis::compiler::typenameof(node.engine())
			<< ", lib " << node.engine().plugin_library_reference().name()
			<< "): ";
		if(!node.processing_count()) std::cout << "not processed\n";
		else {
			std::cout
				<< node.accumulated_processing_time().get_count() * 1e-9 << "s / "
				<< node.processing_count() << " = "
				<< node.accumulated_processing_time().get_count() * 1e-9 / node.processing_count() << 's';
			if(node.processing_count() > node.processing_count_no_zeroes())
				std::cout << ", zeroes: " << node.processing_count() - node.processing_count_no_zeroes();
			std::cout << '\n';
		}
		total += node.accumulated_processing_time();
	}
	std::cout << "total: " << 1e-9 * total.get_count() << "s\n";
}

void scheduler::thread_function(std::size_t thread_number) {
	if(loggers::information()()) loggers::information()("scheduler thread started on graph " + graph().engine().name(), UNIVERSALIS__COMPILER__LOCATION);

	universalis::os::thread_name thread_name;
	{ // set thread name
		std::ostringstream s;
		s << universalis::compiler::typenameof(*this) << '#' << graph().engine().name() << '#' << thread_number;
		thread_name.set(s.str());
	}
	// install cpu/os exception handler/translator
	universalis::cpu::exceptions::install_handler_in_thread();

	try {
		try {
			process_loop();
		} catch(...) {
			loggers::exception()("caught exception in scheduler thread", UNIVERSALIS__COMPILER__LOCATION);
			throw;
		}
	} catch(std::exception const & e) {
		if(loggers::exception()()) {
			std::ostringstream s;
			s << "exception: " << universalis::compiler::typenameof(e) << ": " << e.what();
			loggers::exception()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
		throw;
	} catch(...) {
		if(loggers::exception()()) {
			std::ostringstream s;
			s << "exception: " << universalis::compiler::exceptions::ellipsis_desc();
			loggers::exception()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
		throw;
	}
	loggers::information()("scheduler thread on graph " + graph().engine().name() + " terminated", UNIVERSALIS__COMPILER__LOCATION);
}

void scheduler::on_io_ready(node & node) {
	if(false && loggers::trace()()) loggers::trace()("io ready slot, node: " + node.engine().qualified_name(), UNIVERSALIS__COMPILER__LOCATION);
	{ scoped_lock lock(mutex_);
		if(!node.waiting_for_io_ready_signal()) return;
		node.waiting_for_io_ready_signal(false);
		nodes_queue_.push_front(&node);
	}
	condition_.notify_all(); // notify all threads that we added a node to the queue
}

void scheduler::process_loop() throw(std::exception) {
	while(true) {
		class node * node_;
		{ scoped_lock lock(mutex_);
			while(
				!nodes_queue_.size() &&
				!suspend_requested_ &&
				!stop_requested_
			) condition_.wait(lock);

			if(stop_requested_) break;

			if(suspend_requested_) {
				if(!this_thread_suspended_) {
					this_thread_suspended_ = true;
					++suspended_;
					condition_.notify_all();
				}
				continue;
			}
			if(this_thread_suspended_) {
				this_thread_suspended_ = false;
				--suspended_;
			}

			// There are nodes waiting in the queue. We pop the first one.
			node_ = nodes_queue_.front();
			nodes_queue_.pop_front();

			class node & node(*node_);

			// If the node drives an underlying device that is not ready,
			// we just wait for its io_ready_signal to be emitted and handled by the scheduler's on_io_ready slot.
			// on_io_ready will readd it to the processing queue.
			if(!node.engine().io_ready()) {
				if(!loggers::trace()()) loggers::trace()("node io not ready: "  + node.engine().qualified_name(), UNIVERSALIS__COMPILER__LOCATION);
				// signal the node we want to be notified when it's eventually ready to be processed
				node.waiting_for_io_ready_signal(true);
				continue; // continue with other nodes in the queue
			}
			
			if(false && !loggers::trace()()) {
				static unsigned int i(0);
				static const char c [] = { '-', '\\', '|', '/' };
				std::cout << ' ' << c[++i %= sizeof c] << '\r' << std::flush;
			}
		}
		class node & node(*node_);

		//if(node.processed()) throw /*logic_error*/runtime_error("bug: node already processed: " + node.engine().qualified_name(), UNIVERSALIS__COMPILER__LOCATION);

		process(node); // note: the node's do_process() is supposed to wait until io_ready()

		unsigned int notify(0);
		{ scoped_lock lock(mutex_);
			// check whether all nodes have been processed
			if(++processed_node_count_ == graph().size()) {
				processed_node_count_ = 0;
				// reset the queue to the terminal nodes in the graph (nodes with no connected input ports)
				nodes_queue_ = graph().terminal_nodes();
				notify = 2;
			} else // check whether successors of the node we processed are now ready.
				// iterate over all the output ports of the node we processed
				for(node::output_ports_type::const_iterator
					i(node.output_ports().begin()),
					e(node.output_ports().end()); i != e; ++i
				) {
					ports::output & output_port(**i);
					// iterate over all the input ports connected to our output port
					for(ports::output::input_ports_type::const_iterator
						ii(output_port.input_ports().begin()),
						ie(output_port.input_ports().end()); ii != ie; ++ii
					) {
						// get the node of the input port
						class node & node((**ii).node());
						node.predecessor_node_processed();
						if(node.is_ready_to_process()) {
							// All the dependencies of the node have been processed.
							// We add the node to the processing queue.
							nodes_queue_.push_back(&node);
							++notify;
						}
					}
				}
		}
		switch(notify) {
			case 0:
				// no successor ready
			break;
			case 1:
				// If there's only one successor ready, we don't notify since it can be processed in the same thread.
			break;
			case 2:
				condition_.notify_one(); // notify one thread that we added nodes to the queue
			break;
			default:
				condition_.notify_all(); // notify all threads that we added nodes to the queue
		}
	}
}

void scheduler::process(class node & node) throw(std::exception) {
	if(false && loggers::trace()()) {
		std::ostringstream s;
		s << "scheduling " << node.engine().qualified_name();
		loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}

	node.reset();

	// get buffers for the single input ports from the buffers of their connected output ports
	for(node::single_input_ports_type::const_iterator
		i(node.single_input_ports().begin()),
		e(node.single_input_ports().end()); i !=e ; ++i
	) {
		ports::inputs::single & single_input_port(**i);
		if(single_input_port.output_port())
			single_input_port.buffer(&single_input_port.output_port()->buffer());
	}
	
	if(!node.multiple_input_port()) { // the node has no multiple input port: simple case
		set_buffers_for_all_output_ports_of_node_from_buffer_pool(node);
		node.process();
	} else if(node.multiple_input_port()->output_ports().size()) { // the node has a multiple input port, which is connected: complex case
		// get first output to process 
		ports::output & first_output_port_to_process(node.multiple_input_port_first_output_port_to_process());
		{ // process with first input buffer
			node.multiple_input_port()->buffer(&first_output_port_to_process.buffer());
			if(node.multiple_input_port()->engine().single_connection_is_identity_transform()) { // this is the identity transform when we have a single input
				ports::output & output_port(*node.output_ports().front());
				if(
					node.multiple_input_port()->buffer().reference_count() == 1 || // We are the last input port to read the buffer of the output port, so, we can take over its buffer.
					node.multiple_input_port()->output_ports().size() == 1 // We have a single input, so, this is the identity transform, i.e., the buffer will not be modified.
				) {
					if(false && loggers::trace()()) {
						std::ostringstream s;
						s << node.engine().qualified_name() << ": copying pointer of input buffer to pointer of output buffer";
						loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
					}
					// copy pointer of input buffer to pointer of output buffer
					set_buffer_for_output_port(output_port, node.multiple_input_port()->buffer());
				} else { // we have several inputs, so, this cannot be the identity transform, i.e., the buffer would be modified. but its content must be preserved for further reading
					// get buffer for output port
					set_buffer_for_output_port(output_port, (*buffer_pool_)());
					// copy content of input buffer to output buffer
					if(false && loggers::trace()()) {
						std::ostringstream s;
						s << node.engine().qualified_name() << ": copying content of input buffer to output buffer";
						loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
					}
					output_port.buffer().copy(node.multiple_input_port()->buffer(), node.multiple_input_port()->engine().channels());
				}
			} else { // this is never the identity transform
				set_buffers_for_all_output_ports_of_node_from_buffer_pool(node);
				node.process_first();
			}
			{ scoped_lock lock(mutex_);
				check_whether_to_recycle_buffer_in_the_pool(--first_output_port_to_process.buffer());
			}
		}
		// process with remaining input buffers
		for(ports::inputs::multiple::output_ports_type::const_iterator
			i(node.multiple_input_port()->output_ports().begin()),
			e(node.multiple_input_port()->output_ports().end()); i !=e; ++i
		) {
			ports::output & output_port(**i);
			if(&output_port == &first_output_port_to_process) continue;
			node.multiple_input_port()->buffer(&output_port.buffer());
			node.process();
			{ scoped_lock lock(mutex_);
				check_whether_to_recycle_buffer_in_the_pool(--output_port.buffer());
			}
		}
	}
	{ scoped_lock lock(mutex_);
		// check if the content of the node input ports buffers must be preserved for further reading
		for(node::single_input_ports_type::const_iterator
			i(node.single_input_ports().begin()),
			e(node.single_input_ports().end()); i != e; ++i
		) {
			ports::inputs::single & single_input_port(**i);
			if(single_input_port.output_port())
				check_whether_to_recycle_buffer_in_the_pool(--single_input_port.output_port()->buffer());
		}
		// check if the content of the node output ports buffers must be preserved for further reading
		for(node::output_ports_type::const_iterator
			i(node.output_ports().begin()),
			e(node.output_ports().end()); i != e; ++i
		) {
			ports::output & output_port(**i);
			check_whether_to_recycle_buffer_in_the_pool(output_port.buffer());
		}
	}
	if(false && loggers::trace()()) {
		std::ostringstream s;
		s << "scheduling of " << node.engine().qualified_name() << " done";
		loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
}

/// set buffers for all output ports of the node from the buffer pool.
void inline scheduler::set_buffers_for_all_output_ports_of_node_from_buffer_pool(node & node) {
	for(node::output_ports_type::const_iterator
		i(node.output_ports().begin()),
		e(node.output_ports().end()); i != e; ++i
	) {
		ports::output & output_port(**i);
		if(output_port.input_ports().size())
			set_buffer_for_output_port(output_port, (*buffer_pool_)());
	}
}

/// sets a buffer for the output port
void inline scheduler::set_buffer_for_output_port(ports::output & output_port, buffer & buffer) {
	output_port.buffer(&buffer);
	buffer += output_port.input_ports().size(); // set the expected pending read count
}

/// checks if the content of the buffer must be preserved for further reading and
/// if not recycles it in the pool.
void inline scheduler::check_whether_to_recycle_buffer_in_the_pool(buffer & buffer) {
	if(false && loggers::trace()()) {
		std::ostringstream s;
		s << "buffer: " << &buffer << ": " << buffer.reference_count() << " to go";
		loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
	if(!buffer.reference_count()) (*buffer_pool_)(buffer); // recycle the buffer in the pool
}

/**********************************************************************************************************************/
// buffer pool

scheduler::buffer_pool::buffer_pool(std::size_t channels, std::size_t events) throw(std::exception)
:
	channels_(channels),
	events_(events)
{}

scheduler::buffer_pool::~buffer_pool() throw() {
	for(list_type::const_iterator i(list_.begin()), e(list_.end()); i != e; ++i) delete *i;
}

/**********************************************************************************************************************/
// buffer

buffer::buffer(std::size_t channels, std::size_t events) throw(std::exception)
:
	engine::buffer(channels, events),
	reference_count_()
{}

buffer::~buffer() throw() {
	assert(!this->reference_count());
}

}}
