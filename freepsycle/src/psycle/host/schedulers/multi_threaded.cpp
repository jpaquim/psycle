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
#if 1
#else
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

graph::graph(graph::underlying_type & underlying) : graph_base(underlying) {
	// register to the signals
	new_node_signal().connect(boost::bind(&graph::on_new_node, this, _1));
	delete_node_signal().connect(boost::bind(&graph::on_delete_node, this, _1));
	new_connection_signal().connect(boost::bind(&graph::on_new_connection, this, _1, _2));
	delete_connection_signal().connect(boost::bind(&graph::on_delete_connection, this, _1, _2));
}

void graph::after_construction() {
	graph_base::after_construction();
	compute_plan();
}

void graph::on_new_node(node &) {
	//graph_base::on_new_node(node);
	//compute_plan();
}

void graph::on_delete_node(node &) {
	//graph_base::on_delete_node(node);
	//compute_plan();
}

void graph::on_new_connection(ports::input &, ports::output &) {
	//graph_base::on_new_connection(in, out);
	//compute_plan();
}

void graph::on_delete_connection(ports::input &, ports::output &) {
	//graph_base::on_delete_connection(in, out);
	//compute_plan();
}

void graph::compute_plan() {
	// iterate over all the nodes.
	for(const_iterator i(begin()) ; i != end() ; ++i) {
		typenames::node & node(**i);
		// determine whether the node is a terminal one (i.e. whether some input ports are connected).
		if(node.multiple_input_port()) node.has_connected_input_ports_ = true;
		else for(
			typenames::node::single_input_ports_type::const_iterator i(node.single_input_ports().begin());
			i != node.single_input_ports().end() ; ++i
		) if((**i).output_port()) {
			node.has_connected_input_ports_ = true;
			break;
		}
	}
}

/**********************************************************************************************************************/
// node

node::node(node::parent_type & parent, underlying_type & underlying)
:
	node_base(parent, underlying),
	multiple_input_port_first_output_port_to_process_(),
	output_port_count_(),
	processed_(true) // set to true because reset() is called first in the processing loop
{
	// register to the signals
	new_output_port_signal()        .connect(boost::bind(&node::on_new_output_port        , this, _1));
	new_single_input_port_signal()  .connect(boost::bind(&node::on_new_single_input_port  , this, _1));
	new_multiple_input_port_signal().connect(boost::bind(&node::on_new_multiple_input_port, this, _1));
}

void node::after_construction() {
	node_base::after_construction();
}

void node::on_new_output_port(ports::output &) {
}

void node::on_new_single_input_port(ports::inputs::single &) {
}

void node::on_new_multiple_input_port(ports::inputs::multiple &) {
}

void node::reset_time_measurement() {
	accumulated_processing_time_ = 0;
	processing_count_ = processing_count_no_zeroes_ = 0;
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
}

/**********************************************************************************************************************/
// port
port::port(port::parent_type & parent, underlying_type & underlying) : port_base(parent, underlying) {}

namespace ports {

	/**********************************************************************************************************************/
	// output
	output::output(output::parent_type & parent, output::underlying_type & underlying)
	: output_base(parent, underlying)
	{
		reset();
	}
	
	/**********************************************************************************************************************/
	// input
	input::input(input::parent_type & parent, input::underlying_type & underlying) : input_base(parent, underlying) {}
	
	namespace inputs {

		/**********************************************************************************************************************/
		// single
		single::single(single::parent_type & parent, single::underlying_type & underlying) : single_base(parent, underlying) {}

		/**********************************************************************************************************************/
		// multiple
		multiple::multiple(multiple::parent_type & parent, multiple::underlying_type & underlying) : multiple_base(parent, underlying) {}
	}
}

/**********************************************************************************************************************/
// scheduler

scheduler::scheduler(underlying::graph & graph) throw(std::exception)
:
	host::scheduler<graph_type>(graph),
	buffer_pool_instance_(),
	thread_(),
	stop_requested_()
{
	#if 0
	// register to the graph signals
	graph.         new_node_signal().connect(boost::bind(&scheduler::on_new_node         , this, _1    ));
	//graph.      delete_node_signal().connect(boost::bind(&scheduler::on_delete_node      , this, _1    ));
	graph.   new_connection_signal().connect(boost::bind(&scheduler::on_new_connection   , this, _1, _2));
	graph.delete_connection_signal().connect(boost::bind(&scheduler::on_delete_connection, this, _1, _2));
	#endif
}

scheduler::~scheduler() throw() {
	stop();
	delete &graph();
}

void scheduler::on_new_node(node &) {
	compute_plan();
}

void scheduler::on_delete_node(node &) {
	compute_plan();
}

void scheduler::on_new_connection(ports::input &, ports::output &) {
	compute_plan();
}

void scheduler::on_delete_connection(ports::input &, ports::output &) {
	compute_plan();
}

void scheduler::compute_plan() {
	if(!started()) return;
	stop();
	start();
}

namespace {
	class thread {
		public:
			thread(scheduler & scheduler) : scheduler_(scheduler) {}
			void operator()() { scheduler_(); }
		private:
			scheduler & scheduler_;
	};
}

void scheduler::start() throw(engine::exception) {
	if(loggers::information()()) loggers::information()("starting scheduler threads on graph " + graph().underlying().name() + " ...", UNIVERSALIS__COMPILER__LOCATION);
	if(threads_.size()) {
		if(loggers::information()()) loggers::information()("scheduler threads are already running", UNIVERSALIS__COMPILER__LOCATION);
		return;
	}
	try {
		allocate();
		try {
			stop_requested_ = false;
			// start the threads
			std::size_t thread_count(2);
			for(std::size_t i(O); i < thread_count; ++i) threads_.push_back(new std::thread(thread(*this)));
		} catch(...) {
			free();
			throw;
		}
	} catch(std::exception /*boost::thread_resource_error*/ const & e) {
		loggers::exception()("caught exception", UNIVERSALIS__COMPILER__LOCATION);
		std::ostringstream s; s << universalis::compiler::typenameof(e) << ": " << e.what();
		throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
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
	for(threads_type::const_iterator i(threads_.begin()), e(threads_.end()); i != e; ++i) {
		(**i).join();
		delete *i;
	}
	if(loggers::information()()) loggers::information()("scheduler threads joined", UNIVERSALIS__COMPILER__LOCATION);
	threads_.clear();
	free();
}

void scheduler::operator()() {
	if(loggers::information()()) loggers::information()("scheduler thread started on graph " + graph().underlying().name(), UNIVERSALIS__COMPILER__LOCATION);
	std::string thread_name(universalis::compiler::typenameof(*this) + "#" + graph().underlying().name()); ///\todo + number
	universalis::processor::exception::install_handler_in_thread(thread_name);
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

void scheduler::allocate() throw(std::exception) {
	loggers::trace()("allocating ...", UNIVERSALIS__COMPILER__LOCATION);
	graph().compute_plan();
	std::size_t channels(0);
	// find the terminal nodes in the graph (nodes with no connected input ports, i.e. leaves)
	for(graph_type::const_iterator i(graph().begin()) ; i != graph().end() ; ++i) {
		typenames::node & node(**i);
		node.underlying().start();
		if(node.has_connected_input_ports()) blocked_nodes_.push_back(&node);
		else waiting_nodes_.push_back(&node);
		// find the maximum number of channels needed for buffers
		for(typenames::node::output_ports_type::const_iterator i(node.output_ports().begin()) ; i != node.output_ports().end() ; ++i) {
			ports::output & output_port(**i);
			channels = std::max(channels, output_port.underlying().channels());
		}
		// initialise time measurement
		node.reset_time_measurement();
	}
	if(loggers::trace()()) {
		std::ostringstream s;
		s << "channels: " << channels;
		loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
	buffer_pool_instance_ = new buffer_pool(channels, graph().underlying().events_per_buffer());
}

void scheduler::free() throw() {
	loggers::trace()("freeing ...", UNIVERSALIS__COMPILER__LOCATION);
	delete buffer_pool_instance_; buffer_pool_instance_ = 0;
	waiting_nodes_.clear();
}

void scheduler::process_loop() {
	while(true) {
		typenames::node * node;
		{ scoped_lock lock(mutex_);
			while(!waiting_nodes_.size() && !stop_requested_) condition_.wait(lock);
			if(stop_requested_) return;
			// There are nodes waiting in the queue. We pop the first one.
			node = waiting_nodes_.front();
			waiting_nodes_.pop_front();
		}
		node->reset();
		node->process();
		bool notify(false);
		{ scoped_lock lock(mutex_);
			// iterate over all the output ports of the node we processed
			for(typenames::node::output_ports_type::const_iterator i(n->output_ports().begin()), e(n->output_ports().end()); i != e; ++i) {
				ports::output & output_port(**i);
				// iterate over all the input ports connected to our output port
				for(ports::output::input_ports_type::const_iterator i(output_port
				// check whether the output port is connected
				if(output_port.input_port()) {
					// get the input port connected to our output port
					ports::input & input_port(*output_port.input_port());
					--input_port;
					if(!input_port.remaining_output_port_count) {
						node & n(input_port.parent());
						--n;
						if(!n.remaining_count()) {
							// All the dependencies of the node have been processed.
							// We add the node to the processing queue.
							// (note: for the first node, we could reserve it for ourselves)
							waiting_nodes.push_back(&n);
							notify = true;
						}
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
	for(iterator i(begin()) ; i != end() ; ++i) delete *i;
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
#endif
