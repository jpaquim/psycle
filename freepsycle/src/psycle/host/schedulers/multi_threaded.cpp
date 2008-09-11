// -*- mode:c++; indent-tabs-mode:t -*-
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2008 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\implementation psycle::host::schedulers::multi_threaded
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <psycle/detail/project.private.hpp>
#include "multi_threaded.hpp"
#include <universalis/processor/exception.hpp>
#include <universalis/compiler/typenameof.hpp>
#include <universalis/compiler/exceptions/ellipsis.hpp>
#include <universalis/operating_system/clocks.hpp>
#include <sstream>
#include <limits>
namespace psycle { namespace host { namespace schedulers { namespace multi_threaded {
using engine::exceptions::runtime_error;

namespace {
	std::nanoseconds cpu_time_clock() {
		#if 0
			return std::hiresolution_clock<std::utc_time>::universal_time().nanoseconds_since_epoch();
		#elif 0
			return universalis::operating_system::clocks::thread_cpu_time::current();
		#elif 0
			return universalis::operating_system::clocks::process_cpu_time::current();
		#else
			return universalis::operating_system::clocks::monotonic::current();
		#endif
	}

	static UNIVERSALIS__COMPILER__THREAD_LOCAL_STORAGE bool this_thread_suspended_ = false;
}

/**********************************************************************************************************************/
// graph

void graph::clear_plan() {
	terminal_nodes_.clear();
}

void graph::compute_plan() {
	terminal_nodes_.clear();
	channels_ = 0;

	// iterate over all the nodes
	for(const_iterator i(begin()), e(end()); i != e; ++i) {
		typenames::node & node(**i);

		node.compute_plan();

		// find the terminal nodes in the graph (nodes with no connected input ports)
		if(node.is_ready_to_process()) {
			if(loggers::trace()()) {
				std::ostringstream s;
				s << "terminal node: " << node.underlying().name();
				loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
			terminal_nodes_.push_back(&node);
		}

		// find the maximum number of channels needed for buffers
		// iterate over all output ports of the node
		for(typenames::node::output_ports_type::const_iterator
			i(node.output_ports().begin()),
			e(node.output_ports().end()); i != e; ++i
		) channels_ = std::max(channels(), (**i).underlying().channels());
	}

	if(loggers::trace()()) {
		std::ostringstream s;
		s << "channels: " << channels();
		loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
}

/**********************************************************************************************************************/
// node

node::node(node::parent_type & parent, underlying_type & underlying)
:
	node_base(parent, underlying),
	multiple_input_port_first_output_port_to_process_(),
	on_underlying_io_ready_signal_connection(underlying.io_ready_signal().connect(boost::bind(&node::on_underlying_io_ready, this, _1))),
	waiting_for_io_ready_signal_(),
	accumulated_processing_time_(),
	processing_count_(),
	processing_count_no_zeroes_()
{}

void node::reset_time_measurement() {
	accumulated_processing_time_ = 0;
	processing_count_ = processing_count_no_zeroes_ = 0;
}

void node::compute_plan() {
	// initialise time measurement
	reset_time_measurement();
	
	processed_ = true; // set to true because reset() is called first in the processing loop
	waiting_for_io_ready_signal_ = false;

	// count the number of predecessor nodes
	if(multiple_input_port()) predecessor_node_count_ = multiple_input_port()->output_ports().size();
	else predecessor_node_count_ = 0;
	for(typenames::node::single_input_ports_type::const_iterator
		i(single_input_ports().begin()),
		e(single_input_ports().end()); i != e; ++i
	) if((**i).output_port()) ++predecessor_node_count_;
	predecessor_node_remaining_count_ = predecessor_node_count_;

	if(multiple_input_port()) {
		// If the node has a multiple input port,
		// find which output port connected to it has the minimum number of connections.
		// note: this is a simplistic heuristic to minimise the number of buffer copy operations,
		// the best algorithm would be to order the inputs with a recursive evaluation on the graph.
		{
			std::size_t minimum_size(std::numeric_limits<std::size_t>::max());
			for(ports::inputs::multiple::output_ports_type::const_iterator
				i(multiple_input_port()->output_ports().begin()),
				e(multiple_input_port()->output_ports().end()); i != e; ++i
			) {
				ports::output & output_port(**i);
				if(output_port.input_ports().size() < minimum_size) {
					minimum_size = output_port.input_ports().size();
					multiple_input_port_first_output_port_to_process_ = &output_port;
					if(minimum_size == 1) break; // it's already an ideal case, we can't find a better one.
				}
			}
		}
		assert(!multiple_input_port()->output_ports().size() || multiple_input_port_first_output_port_to_process_);
	}
}

void node::reset() throw() {
	assert(processed());
	processed_ = false;
	predecessor_node_remaining_count_ = predecessor_node_count_;
	underlying().reset();
}

void node::process(bool first) {
	std::nanoseconds const t0(cpu_time_clock());
	if(first) underlying().process_first(); else underlying().process();
	std::nanoseconds const t1(cpu_time_clock());
	if(t1 != t0) {
		accumulated_processing_time_ += t1 - t0;
		++processing_count_no_zeroes_;
	}
	++processing_count_;
	processed_ = true;
}

/**********************************************************************************************************************/
// scheduler

scheduler::scheduler(underlying::graph & graph, std::size_t threads) throw(std::exception)
:
	host::scheduler<graph_type>(graph),
	// register to the graph signals
	on_new_node_signal_connection         (graph.         new_node_signal().connect(boost::bind(&scheduler::on_new_node         , this, _1    ))),
	on_delete_node_signal_connection      (graph.      delete_node_signal().connect(boost::bind(&scheduler::on_delete_node      , this, _1    ))),
	on_new_connection_signal_connection   (graph.   new_connection_signal().connect(boost::bind(&scheduler::on_new_connection   , this, _1, _2))),
	on_delete_connection_signal_connection(graph.delete_connection_signal().connect(boost::bind(&scheduler::on_delete_connection, this, _1, _2))),
	buffer_pool_instance_(),
	thread_count_(threads)
{}

scheduler::~scheduler() throw() {
	stop();
	delete &graph(); // note that this doesn't delete the underying graph, only the scheduler's wrapping layer
}

void scheduler::on_new_node(node::underlying_type &) {
	suspend_and_compute_plan();
}

void scheduler::on_delete_node(node::underlying_type &) {
	suspend_and_compute_plan();
}

void scheduler::on_new_connection(ports::input::underlying_type &, ports::output::underlying_type &) {
	suspend_and_compute_plan();
}

void scheduler::on_delete_connection(ports::input::underlying_type &, ports::output::underlying_type &) {
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
	if(loggers::information()()) loggers::information()("starting scheduler threads on graph " + graph().underlying().name() + " ...", UNIVERSALIS__COMPILER__LOCATION);
	if(threads_.size()) {
		if(loggers::information()()) loggers::information()("scheduler threads are already running", UNIVERSALIS__COMPILER__LOCATION);
		return;
	}

	compute_plan();

	// ensure the nodes are started
	// iterate over all the nodes in the graph
	for(graph_type::const_iterator i(graph().begin()), e(graph().end()); i != e; ++i)
		(**i).underlying().start();

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

	delete buffer_pool_instance_;
	buffer_pool_instance_ = new buffer_pool(graph().channels(), graph().underlying().events_per_buffer());
}

void scheduler::clear_plan() {
	delete buffer_pool_instance_; buffer_pool_instance_ = 0;
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
	clear_plan();

	// dump time measurements
	std::cout << "time measurements: \n";
	std::nanoseconds total;
	for(graph::const_iterator i(graph().begin()), e(graph().end()); i != e; ++i) {
		node & node(**i);
		std::cout
			<< node.underlying().qualified_name()
			<< " (" << universalis::compiler::typenameof(node.underlying())
			<< ", lib " << node.underlying().plugin_library_reference().name()
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
	if(loggers::information()()) loggers::information()("scheduler thread started on graph " + graph().underlying().name(), UNIVERSALIS__COMPILER__LOCATION);

	{ // set thread name and install cpu/os exception handler/translator
		std::ostringstream s;
		s << universalis::compiler::typenameof(*this) << '#' << graph().underlying().name() << '#' << thread_number;
		universalis::processor::exception::install_handler_in_thread(s.str());
	}

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
			s << "exception: " << universalis::compiler::exceptions::ellipsis();
			loggers::exception()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
		throw;
	}
	loggers::information()("scheduler thread on graph " + graph().underlying().name() + " terminated", UNIVERSALIS__COMPILER__LOCATION);
}

void scheduler::on_io_ready(node & node) {
	if(loggers::trace()()) loggers::trace()("io ready slot, node: "  + node.underlying().qualified_name(), UNIVERSALIS__COMPILER__LOCATION);
	{ scoped_lock lock(mutex_);
		if(!node.waiting_for_io_ready_signal()) return;
		node.waiting_for_io_ready_signal(false);
		nodes_queue_.push_front(&node);
	}
	condition_.notify_all(); // notify all threads that we added a node to the queue
}

void scheduler::process_loop() throw(std::exception) {
	while(true) {
		typenames::node * node_;
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

			typenames::node & node(*node_);

			// If the node drives an underlying device that is not ready,
			// we just wait for its io_ready_signal to be emitted and handled by the scheduler's on_io_ready slot.
			// on_io_ready will readd it to the processing queue.
			if(!node.underlying().io_ready()) {
				if(!loggers::trace()()) loggers::trace()("node io not ready: "  + node.underlying().qualified_name(), UNIVERSALIS__COMPILER__LOCATION);
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
		typenames::node & node(*node_);

		//if(node.processed()) throw /*logic_error*/runtime_error("bug: node already processed: " + node.underlying().qualified_name(), UNIVERSALIS__COMPILER__LOCATION);

		process(node); // note: the node's do_process() is supposed to wait until io_ready()

		bool notify(false);
		{ scoped_lock lock(mutex_);
			// check whether all nodes have been processed
			if(++processed_node_count_ == graph().size()) {
				processed_node_count_ = 0;
				// reset the queue to the terminal nodes in the graph (nodes with no connected input ports)
				nodes_queue_ = graph().terminal_nodes();
				notify = true;
			} else // check whether successors of the node we processed are now ready.
				// iterate over all the output ports of the node we processed
				for(typenames::node::output_ports_type::const_iterator
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
						typenames::node & node((**ii).parent());
						node.predecessor_node_processed();
						if(node.is_ready_to_process()) {
							// All the dependencies of the node have been processed.
							// We add the node to the processing queue.
							// (note: for the first node, we could reserve it for ourselves)
							nodes_queue_.push_back(&node);
							notify = true;
						}
					}
				}
		}
		if(notify) condition_.notify_all(); // notify all threads that we added nodes to the queue
	}
}

void scheduler::process(typenames::node & node) throw(std::exception) {
	if(false && loggers::trace()()) {
		std::ostringstream s;
		s << "scheduling " << node.underlying().qualified_name();
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
			if(node.multiple_input_port()->underlying().single_connection_is_identity_transform()) { // this is the identity transform when we have a single input
				ports::output & output_port(*node.output_ports().front());
				if(
					node.multiple_input_port()->buffer().reference_count() == 1 || // We are the last input port to read the buffer of the output port, so, we can take over its buffer.
					node.multiple_input_port()->output_ports().size() == 1 // We have a single input, so, this is the identity transform, i.e., the buffer will not be modified.
				) {
					if(false && loggers::trace()()) {
						std::ostringstream s;
						s << node.underlying().qualified_name() << ": copying pointer of input buffer to pointer of output buffer";
						loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
					}
					// copy pointer of input buffer to pointer of output buffer
					set_buffer_for_output_port(output_port, node.multiple_input_port()->buffer());
				} else { // we have several inputs, so, this cannot be the identity transform, i.e., the buffer would be modified. but its content must be preserved for further reading
					// get buffer for output port
					set_buffer_for_output_port(output_port, buffer_pool_instance()());
					// copy content of input buffer to output buffer
					if(false && loggers::trace()()) {
						std::ostringstream s;
						s << node.underlying().qualified_name() << ": copying content of input buffer to output buffer";
						loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
					}
					output_port.buffer().copy(node.multiple_input_port()->buffer(), node.multiple_input_port()->underlying().channels());
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
		for(typenames::node::single_input_ports_type::const_iterator
			i(node.single_input_ports().begin()),
			e(node.single_input_ports().end()); i != e; ++i
		) {
			ports::inputs::single & single_input_port(**i);
			if(single_input_port.output_port())
				check_whether_to_recycle_buffer_in_the_pool(--single_input_port.output_port()->buffer());
		}
		// check if the content of the node output ports buffers must be preserved for further reading
		for(typenames::node::output_ports_type::const_iterator
			i(node.output_ports().begin()),
			e(node.output_ports().end()); i != e; ++i
		) {
			ports::output & output_port(**i);
			check_whether_to_recycle_buffer_in_the_pool(output_port.buffer());
		}
	}
	if(false && loggers::trace()()) {
		std::ostringstream s;
		s << "scheduling of " << node.underlying().qualified_name() << " done";
		loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
}

/// set buffers for all output ports of the node from the buffer pool.
void inline scheduler::set_buffers_for_all_output_ports_of_node_from_buffer_pool(node & node) {
	for(typenames::node::output_ports_type::const_iterator
		i(node.output_ports().begin()),
		e(node.output_ports().end()); i != e; ++i
	) {
		ports::output & output_port(**i);
		if(output_port.input_ports().size())
			set_buffer_for_output_port(output_port, buffer_pool_instance()());
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
	if(!buffer.reference_count()) (*buffer_pool_instance_)(buffer); // recycle the buffer in the pool
}

/**********************************************************************************************************************/
// buffer pool

scheduler::buffer_pool::buffer_pool(std::size_t channels, std::size_t events) throw(std::exception)
:
	channels_(channels),
	events_(events)
{}

scheduler::buffer_pool::~buffer_pool() throw() {
	for(const_iterator i(begin()), e(end()); i != e; ++i) delete *i;
}

/**********************************************************************************************************************/
// buffer

buffer::buffer(std::size_t channels, std::size_t events) throw(std::exception)
:
	underlying::buffer(channels, events),
	reference_count_()
{}

buffer::~buffer() throw() {
	assert(!this->reference_count());
}

}}}}
