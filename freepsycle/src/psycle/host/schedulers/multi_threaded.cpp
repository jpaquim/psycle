// -*- mode:c++; indent-tabs-mode:t -*-
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2008 psycle development team http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

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

		// find the terminal nodes in the graph (nodes with no connected input ports, i.e. leaves)
		if(node.is_ready_to_process()) terminal_nodes_.push_back(&node);

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
	processed_(true), // set to true because reset() is called first in the processing loop
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

	// count the number of predecessor nodes
	if(multiple_input_port()) predecessor_node_count_ = multiple_input_port()->output_ports().size();
	else predecessor_node_count_ = 0;
	for(typenames::node::single_input_ports_type::const_iterator
		i(single_input_ports().begin()),
		e(single_input_ports().end()); i != e; ++i
	) if((**i).output_port()) ++predecessor_node_count_;

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

scheduler::scheduler(underlying::graph & graph) throw(std::exception)
:
	host::scheduler<graph_type>(graph),
	buffer_pool_instance_()
{
	#if 0
	// register to the graph signals
	graph.         new_node_signal().connect(boost::bind(&scheduler::on_new_node         , this, _1    ));
	graph.      delete_node_signal().connect(boost::bind(&scheduler::on_delete_node      , this, _1    ));
	graph.   new_connection_signal().connect(boost::bind(&scheduler::on_new_connection   , this, _1, _2));
	graph.delete_connection_signal().connect(boost::bind(&scheduler::on_delete_connection, this, _1, _2));
	#endif
}

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

	try {
		// start the scheduling threads
		std::size_t thread_count(2); ///\todo parametrable
		for(std::size_t i(0); i < thread_count; ++i)
			threads_.push_back(new std::thread(boost::bind(&scheduler::thread_function, this, i)));
	} catch(...) {
		{ scoped_lock lock(mutex_);
			stop_requested_ = true;
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
	}
	condition_.notify_all();
	{ scoped_lock lock(mutex_);
		while(suspended_ != threads_.size()) condition_.wait(lock);
		compute_plan();
		suspend_requested_ = false;
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
	for(graph::const_iterator i(graph().begin()), e(graph().end()); i != e; ++i) {
		node & node(**i);
		std::cout
			<< node.underlying().qualified_name()
			<< " (" << universalis::compiler::typenameof(node.underlying())
			<< ", lib " << node.underlying().plugin_library_reference().name()
			<< "): ";
		if(!node.processing_count()) std::cout << "not processed";
		else std::cout
			<< node.accumulated_processing_time().get_count() * 1e-9 << "s / "
			<< node.processing_count() << " = "
			<< node.accumulated_processing_time().get_count() * 1e-9 / node.processing_count() << "s"
			", zeroes: " << node.processing_count() - node.processing_count_no_zeroes() << '\n';
			
	}
}

void scheduler::thread_function(std::size_t thread_number) {
	if(loggers::information()()) loggers::information()("scheduler thread started on graph " + graph().underlying().name(), UNIVERSALIS__COMPILER__LOCATION);

	{ // set thread name and install cpu/os exception handler/translator
		std::ostringstream s;
		s << universalis::compiler::typenameof(*this) << "#" << graph().underlying().name() << thread_number;
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

void scheduler::process_loop() {
	while(true) {
		typenames::node * node_;
		{ scoped_lock lock(mutex_);
			while((!nodes_queue_.size() || suspend_requested_) && !stop_requested_) condition_.wait(lock);
			if(stop_requested_) break;
			if(suspend_requested_) {
				++suspended_; ///\todo spurious wakeups will make the counter bogus!
				condition_.notify_all();
				continue;
			}
			suspended_ = 0;
			// There are nodes waiting in the queue. We pop the first one.
			node_ = nodes_queue_.front();
			nodes_queue_.pop_front();
		}
		typenames::node & node(*node_);
		if(node.processed()) continue;
		node.reset();

		///\todo set the buffer pointers before calling node.process()
		node.process();

		#if 0
		if(!node.multiple_input_port()) { // the node has no multiple input port: simple case
			set_buffers_for_all_output_ports_of_node_from_buffer_pool(node);
			node.process();
		} else if(node.multiple_input_port()->output_ports().size()) { // the node has a multiple input port, which is connected: complex case
			...
		}
		#endif

		bool notify(false);
		{ scoped_lock lock(mutex_);
			// check whether all nodes have been processed
			if(++processed_node_count_ == graph().size()) {
				processed_node_count_ = 0;
				// reset the queue to the terminal nodes in the graph (nodes with no connected input ports, i.e. leaves)
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
