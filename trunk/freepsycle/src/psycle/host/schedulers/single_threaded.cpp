// -*- mode:c++; indent-tabs-mode:t -*-
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2008 psycle development team http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\implementation psycle::host::schedulers::single_threaded
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <psycle/detail/project.private.hpp>
#include "single_threaded.hpp"
#include <universalis/processor/exception.hpp>
#include <universalis/compiler/typenameof.hpp>
#include <universalis/compiler/exceptions/ellipsis.hpp>
#include <universalis/operating_system/clocks.hpp>
#include <sstream>
#include <limits>
namespace psycle { namespace host { namespace schedulers { namespace single_threaded {

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
		if(node.multiple_input_port()) {
			// If the node has a multiple input port,
			// find which output port connected to it has the minimum number of connections.
			// note: this is a simplistic heuristic to minimise the number of buffer copy operations,
			// the best algorithm would be to order the inputs with a recursive evaluation on the graph.
			{
				std::size_t minimum_size(std::numeric_limits<std::size_t>::max());
				for(
					ports::inputs::multiple::output_ports_type::const_iterator i(node.multiple_input_port()->output_ports().begin());
					i != node.multiple_input_port()->output_ports().end() ; ++i
				) {
					ports::output & output_port(**i);
					if(output_port.input_ports().size() < minimum_size) {
						minimum_size = output_port.input_ports().size();
						node.multiple_input_port_first_output_port_to_process_ = &output_port;
						if(minimum_size == 1) break; // it's already an ideal case, we can't find a better one.
					}
				}
			}
			assert(!node.multiple_input_port()->output_ports().size() || node.multiple_input_port_first_output_port_to_process_);
		}
		// determine whether the node is a terminal one (i.e. whether some output ports are connected).
		for(
			typenames::node::output_ports_type::const_iterator i(node.output_ports().begin());
			i != node.output_ports().end() ; ++i
		) if((**i).input_ports().size()) {
			node.has_connected_output_ports_ = true;
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
	has_connected_output_ports_(),
	processed_(true), // set to true because reset() is called first in the processing loop
	accumulated_processing_time_(),
	processing_count_(),
	processing_count_no_zeroes_()
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
	output::output(output::parent_type & parent, output::underlying_type & underlying) : output_base(parent, underlying) {}
	
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
	if(loggers::information()()) {
		loggers::information()("starting scheduler thread on graph " + graph().underlying().name() + " ...", UNIVERSALIS__COMPILER__LOCATION);
	}
	if(thread_) {
		if(loggers::information()()) {
			loggers::information()("scheduler thread is already running", UNIVERSALIS__COMPILER__LOCATION);
		}
		return;
	}
	try {
		thread_ = new std::thread(thread(*this));
	} catch(std::exception /*boost::thread_resource_error*/ const & e) {
		loggers::exception()("caught exception", UNIVERSALIS__COMPILER__LOCATION);
		std::ostringstream s; s << universalis::compiler::typenameof(e) << ": " << e.what();
		throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
}

void scheduler::stop() {
	if(loggers::information()()) {
		loggers::information()("terminating and joining scheduler thread ...", UNIVERSALIS__COMPILER__LOCATION);
	}
	if(!thread_) {
		if(loggers::information()()) {
			loggers::information()("scheduler thread was not running", UNIVERSALIS__COMPILER__LOCATION);
		}
		return;
	}
	{
		std::scoped_lock<std::mutex> lock(mutex_);
		stop_requested_ = true;
	}
	thread_->join();
	if(loggers::information()()) {
		loggers::information()("scheduler thread joined", UNIVERSALIS__COMPILER__LOCATION);
	}
	delete thread_; thread_ = 0;
}

bool scheduler::stop_requested() {
	std::scoped_lock<std::mutex> lock(mutex_);
	return stop_requested_;
}

void scheduler::operator()() {
	loggers::information()("scheduler thread started on graph " + graph().underlying().name(), UNIVERSALIS__COMPILER__LOCATION);
	std::string thread_name(universalis::compiler::typenameof(*this) + "#" + graph().underlying().name());
	universalis::processor::exception::install_handler_in_thread(thread_name);
	try {
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
	} catch(...) {
		{
			std::scoped_lock<std::mutex> lock(mutex_);
			stop_requested_ = false;
		}
		throw;
	}
	loggers::information()("scheduler thread on graph " + graph().underlying().name() + " terminated", UNIVERSALIS__COMPILER__LOCATION);
	{
		std::scoped_lock<std::mutex> lock(mutex_);
		stop_requested_ = false;
	}
}

void scheduler::allocate() throw(std::exception) {
	loggers::trace()("allocating ...", UNIVERSALIS__COMPILER__LOCATION);
	graph().compute_plan();
	std::size_t channels(0);
	// find the terminal nodes in the graph (nodes with no connected output ports, i.e. leaves)
	for(graph_type::const_iterator i(graph().begin()) ; i != graph().end() ; ++i) {
		typenames::node & node(**i);
		node.underlying().start();
		if(!node.has_connected_output_ports()) {
			if(loggers::trace()()) {
				std::ostringstream s;
				s << "terminal node: " << node.underlying().name();
				loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
			terminal_nodes_.push_back(&node);
		}
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
	terminal_nodes_.clear();
}

void scheduler::process_loop() {
	try {
		allocate();
		while(!stop_requested()) {
			std::scoped_lock<std::mutex> lock(graph().underlying().mutex());
			for(graph::const_iterator i(graph().begin()) ; i != graph().end() ; ++i) {
				node & node(**i);
				if(node.processed()) node.reset();
			}
			for(terminal_nodes_type::iterator i(terminal_nodes_.begin()) ; i != terminal_nodes_.end() ; ++i) {
				node & node(**i);
				process_recursively(node);
			}
		}
		// dump time measurements
		std::cout << "time measurements: \n";
		for(graph::const_iterator i(graph().begin()) ; i != graph().end() ; ++i) {
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
	} catch(...) {
		loggers::exception()("caught exception", UNIVERSALIS__COMPILER__LOCATION);
		free();
		throw;
	}
	free();
}

void scheduler::process_recursively(node & node) {
	if(node.processed()) return;
	node.mark_as_processed();

	if(false && loggers::trace()()) {
		std::ostringstream s;
		s << "scheduling " << node.underlying().qualified_name();
		loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
	
	// get node input buffers by processing the dependencies of the node
	for(node::single_input_ports_type::const_iterator i(node.single_input_ports().begin()) ; i != node.single_input_ports().end() ; ++i) {
		ports::inputs::single & single_input_port(**i);
		if(single_input_port.output_port()) process_node_of_output_port_and_set_buffer_for_input_port(*single_input_port.output_port(), single_input_port);
	}

	if(!node.multiple_input_port()) { // the node has no multiple input port: simple case
		set_buffers_for_all_output_ports_of_node_from_buffer_pool(node);
		node.process();
	} else if(node.multiple_input_port()->output_ports().size()) { // the node has a multiple input port: complex case
		// get first output to process 
		ports::output & first_output_port_to_process(node.multiple_input_port_first_output_port_to_process());
		{ // process with first input buffer
			process_node_of_output_port_and_set_buffer_for_input_port(first_output_port_to_process, *node.multiple_input_port());
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
			check_whether_to_recycle_buffer_in_the_pool(--first_output_port_to_process.buffer());
		}
		// process with remaining input buffers
		for(ports::inputs::multiple::output_ports_type::const_iterator i(node.multiple_input_port()->output_ports().begin()) ; i != node.multiple_input_port()->output_ports().end() ; ++i) {
			ports::output & output_port(**i);
			if(&output_port == &first_output_port_to_process) continue;
			process_node_of_output_port_and_set_buffer_for_input_port(output_port, *node.multiple_input_port());
			node.process();
			check_whether_to_recycle_buffer_in_the_pool(--output_port.buffer());
		}
	}
	// check if the content of the node input ports buffers must be preserved for further reading
	for(typenames::node::single_input_ports_type::const_iterator i(node.single_input_ports().begin()) ; i != node.single_input_ports().end() ; ++i) {
		ports::inputs::single & single_input_port(**i);
		if(single_input_port.output_port()) check_whether_to_recycle_buffer_in_the_pool(--single_input_port.output_port()->buffer());
	}
	// check if the content of the node output ports buffers must be preserved for further reading
	for(typenames::node::output_ports_type::const_iterator i(node.output_ports().begin()) ; i != node.output_ports().end() ; ++i) {
		ports::output & output_port(**i);
		check_whether_to_recycle_buffer_in_the_pool(output_port.buffer());
	}
	if(false && loggers::trace()()) {
		std::ostringstream s;
		s << "scheduling of " << node.underlying().qualified_name() << " done";
		loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
}

/// processes the node of the output port connected to the input port and sets the buffer for the input port
void inline scheduler::process_node_of_output_port_and_set_buffer_for_input_port(ports::output & output_port, ports::input & input_port) {
	process_recursively(output_port.parent());
	assert(&output_port.buffer());
	if(false && loggers::trace()()) {
		std::ostringstream s;
		s << "back to scheduling of input port " << input_port.underlying().qualified_name();
		loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
	input_port.buffer(&output_port.buffer());
}

/// set buffers for all output ports of the node from the buffer pool.
void inline scheduler::set_buffers_for_all_output_ports_of_node_from_buffer_pool(node & node) {
	for(typenames::node::output_ports_type::const_iterator i(node.output_ports().begin()), e(node.output_ports().end()); i != e; ++i) {
		ports::output & output_port(**i);
		// we don't need to check since we don't get here in this case: if(output_port.input_ports().size())
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
