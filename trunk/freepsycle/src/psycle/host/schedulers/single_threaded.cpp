// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

///\implementation psycle::host::schedulers::single_threaded
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <psycle/detail/project.private.hpp>
#include "single_threaded.hpp"
#include <universalis/processor/exception.hpp>
#include <universalis/compiler/typenameof.hpp>
#include <universalis/compiler/exceptions/ellipsis.hpp>
#include <sstream>
#include <limits>
namespace psycle { namespace host { namespace schedulers { namespace single_threaded {

	graph::graph(graph::underlying_type & underlying) : graph_base(underlying) {
		new_node_signal().connect(boost::bind(&graph::on_new_node, this, _1));
	}

	void graph::after_construction() {
		graph_base::after_construction();
		for(const_iterator i(begin()) ; i != end() ; ++i) {
			loggers::trace()("@@@@@@@@@@@@@@@@@@@@@@ sched graph::init node");
			typenames::node & node(**i);
			if(node.multiple_input_port()) {
				/// note: the best algorithm would be to order the inputs with a recursive evaluation on the graph.
				// find the output port which has the minimum number of connections.
				{
					std::size_t minimum_size(std::numeric_limits<std::size_t>::max());
					ports::inputs::multiple::output_ports_type::const_iterator i(node.multiple_input_port()->output_ports().begin());
					for( ; i != node.multiple_input_port()->output_ports().end() ; ++i) {
						ports::output & output_port(**i);
						if(output_port.input_ports().size() < minimum_size) {
							minimum_size = output_port.input_ports().size();
							node.multiple_input_port_first_output_port_to_process_ = &output_port;
							if(minimum_size == 1) break; // it's already an ideal case, we can't find a better one.
						}
					}
					assert(!node.multiple_input_port()->output_ports().size() || node.multiple_input_port_first_output_port_to_process_);
				}
			}
			// count the number of output ports that are connected.
			for(typenames::node::output_ports_type::const_iterator i(node.output_ports().begin()) ; i != node.output_ports().end() ; ++i) {
				if((**i).input_ports().size()) ++node.output_port_count_;
				if(loggers::trace()) {
					std::ostringstream s;
					s << "@@@@@@@@@@@@@@@@@@@@@@ sched node::init connected output port count " << node.output_port_count_;
					loggers::trace()(s.str());
				}
			}
		}
	}

	void graph::on_new_node(typenames::node & underlying_node)
	{}
	
	node::node(node::parent_type & parent, underlying_type & underlying)
	:
		node_base(parent, underlying),
		multiple_input_port_first_output_port_to_process_(),
		output_port_count_(),
		processed_(true) // set to true because reset() is called first in the processing loop
	{
		new_output_port_signal()        .connect(boost::bind(&node::on_new_output_port        , this, _1));
		new_single_input_port_signal()  .connect(boost::bind(&node::on_new_single_input_port  , this, _1));
		new_multiple_input_port_signal().connect(boost::bind(&node::on_new_multiple_input_port, this, _1));
	}

	void node::after_construction() {
		node_base::after_construction();
	}

	void node::on_new_output_port(typenames::ports::output & output_port)
	{}

	void node::on_new_single_input_port(typenames::ports::inputs::single & single_input_port)
	{}

	void node::on_new_multiple_input_port(typenames::ports::inputs::multiple & multiple_input_port)
	{}
	
	port::port(port::parent_type & parent, underlying_type & underlying) : port_base(parent, underlying) {}
	
	namespace ports {
		output::output(output::parent_type & parent, output::underlying_type & underlying)
		:
			output_base(parent, underlying),
			input_port_count_(underlying.input_ports().size())
		{
			reset();
		}
		
		input::input(input::parent_type & parent, input::underlying_type & underlying) : input_base(parent, underlying) {}
		
		namespace inputs {
			single::single(single::parent_type & parent, single::underlying_type & underlying) : single_base(parent, underlying)
			{}
			
			multiple::multiple(multiple::parent_type & parent, multiple::underlying_type & underlying) : multiple_base(parent, underlying)
			{}
		}
	}
	
	scheduler::scheduler(underlying::graph & graph) throw(std::exception)
	:
		host::scheduler<graph_type>(graph),
		buffer_pool_instance_(),
		thread_(),
		stop_requested_()
	{}

	scheduler::~scheduler() throw() {
		stop();
		delete &graph();
	}
	
	namespace {
		class thread {
			public:
				inline thread(scheduler & scheduler) : scheduler_(scheduler) {}
				void inline operator()() { scheduler_(); }
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
				loggers::information()("thread is already running", UNIVERSALIS__COMPILER__LOCATION);
			}
			return;
		}
		try {
			thread_ = new boost::thread(thread(*this));
		} catch(boost::thread_resource_error const & e) {
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
			boost::mutex::scoped_lock lock(mutex_);
			stop_requested_ = true;
		}
		thread_->join();
		if(loggers::information()()) {
			loggers::information()("scheduler thread joined", UNIVERSALIS__COMPILER__LOCATION);
		}
		delete thread_; thread_ = 0;
	}

	bool scheduler::stop_requested() {
		boost::mutex::scoped_lock lock(mutex_);
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
				boost::mutex::scoped_lock lock(mutex_);
				stop_requested_ = false;
			}
			throw;
		}
		loggers::information()("scheduler thread on graph " + graph().underlying().name() + " terminated", UNIVERSALIS__COMPILER__LOCATION);
		{
			boost::mutex::scoped_lock lock(mutex_);
			stop_requested_ = false;
		}
	}
	
	void scheduler::allocate() throw(std::exception) {
		loggers::trace()("allocating ...", UNIVERSALIS__COMPILER__LOCATION);
		std::size_t channels(0);
		for(graph_type::const_iterator i(graph().begin()) ; i != graph().end() ; ++i) {
			typenames::node & node(**i);
			node.underlying().start();
			if(!node.output_port_count()) {
				if(loggers::trace()()) {
					std::ostringstream s;
					s << "terminal node: " << node.underlying().name();
					loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
				}
				terminal_nodes_.push_back(&node);
			}
			for(typenames::node::output_ports_type::const_iterator i(node.output_ports().begin()) ; i != node.output_ports().end() ; ++i) {
				ports::output & output_port(**i);
				channels = std::max(channels, output_port.underlying().channels());
			}
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
				//loggers::trace()("process loop", UNIVERSALIS__COMPILER__LOCATION);
				boost::mutex::scoped_lock lock(graph().underlying().mutex());
				for(graph::const_iterator i(graph().begin()) ; i != graph().end() ; ++i) {
					node & node(**i);
					if(node.processed()) node.reset();
				}
				for(terminal_nodes_type::iterator i(terminal_nodes_.begin()) ; i != terminal_nodes_.end() ; ++i) {
					node & node(**i);
					process(node);
				}
			}
		} catch(...) {
			loggers::exception()("caught exception", UNIVERSALIS__COMPILER__LOCATION);
			free();
			throw;
		}
		free();
	}
	
	void scheduler::process(node & node) {
		if(node.processed()) return;
		node.mark_as_processed();
		if(false && loggers::trace()()) {
			std::ostringstream s;
			s << "scheduling " << node.underlying().qualified_name();
			loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
		{
			// get node input buffers by processing the dependencies of the node
			for(node::single_input_ports_type::const_iterator i(node.single_input_ports().begin()) ; i != node.single_input_ports().end() ; ++i) {
				ports::inputs::single & single_input_port(**i);
				if(single_input_port.output_port()) process_node_of_output_port_and_set_buffer_for_input_port(*single_input_port.output_port(), single_input_port);
			}
		}
		if(!node.multiple_input_port()) { // the node has no multiple input port: simple case
			set_buffers_for_all_output_ports_of_node_from_the_buffer_pool(node);
			node.underlying().process();
		}
		else if(node.multiple_input_port()->output_ports().size()) { // the node has a multiple input port: complex case
			// get first output to process 
			ports::output & first_output_port_to_process(node.multiple_input_port_first_output_port_to_process());
			{
				// process with first input buffer
				process_node_of_output_port_and_set_buffer_for_input_port(first_output_port_to_process, *node.multiple_input_port());
				if(node.multiple_input_port()->underlying().single_connection_is_identity_transform()) { // this is the identity transform when we have a single input
					ports::output & output_port(*node.output_ports().front());
					if(
						node.multiple_input_port()->buffer() == 1 || // We are the last input port to read the buffer of the output port, so, we can take over its buffer.
						node.multiple_input_port()->output_ports().size() == 1 // We have a single input, so, this is the identity transform, i.e., the buffer will not be modified.
					) {
						if(false && loggers::trace()()) {
							std::ostringstream s;
							s << node.underlying().qualified_name() << ": copying pointer of input buffer to pointer of output buffer";
							loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
						}
						// copy pointer of input buffer to pointer of output buffer
						set_buffer_for_output_port(output_port, node.multiple_input_port()->buffer());
					} else { // we have several inputs, so, this cannot by the identity transform, i.e., the buffer would be modified. but its content must be preserved for further reading
						// get buffer for output port
						set_buffer_for_output_port(output_port, buffer_pool_instance()());
						// copy content of input buffer to output buffer
						if(false && loggers::trace()())
						{
							std::ostringstream s;
							s << node.underlying().qualified_name() << ": copying content of input buffer to output buffer";
							loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
						}
						output_port.buffer().copy(node.multiple_input_port()->buffer(), node.multiple_input_port()->underlying().channels());
					}
				} else { // this is never the identity transform
					set_buffers_for_all_output_ports_of_node_from_the_buffer_pool(node);
					node.underlying().process_first();
				}
				mark_buffer_as_read_once_more_and_check_whether_to_recycle_it_in_the_pool(first_output_port_to_process, *node.multiple_input_port());
			}
			// process with remaining input buffers
			for(ports::inputs::multiple::output_ports_type::const_iterator i(node.multiple_input_port()->output_ports().begin()) ; i != node.multiple_input_port()->output_ports().end() ; ++i) {
				ports::output & output_port(**i);
				if(&output_port == &first_output_port_to_process) continue;
				process_node_of_output_port_and_set_buffer_for_input_port(output_port, *node.multiple_input_port());
				node.underlying().process();
				mark_buffer_as_read_once_more_and_check_whether_to_recycle_it_in_the_pool(output_port, *node.multiple_input_port());
			}
		}
		// check if the content of the node input ports buffers must be preserved for further reading
		for(typenames::node::single_input_ports_type::const_iterator i(node.single_input_ports().begin()) ; i != node.single_input_ports().end() ; ++i) {
			ports::inputs::single & single_input_port(**i);
			if(single_input_port.output_port()) mark_buffer_as_read_once_more_and_check_whether_to_recycle_it_in_the_pool(*single_input_port.output_port(), single_input_port);
		}
		// check if the content of the node output ports buffers must be preserved for further reading
		for(typenames::node::output_ports_type::const_iterator i(node.output_ports().begin()) ; i != node.output_ports().end() ; ++i) {
			ports::output & output_port(**i);
			check_whether_to_recycle_buffer_in_the_pool(output_port);
		}
		if(false && loggers::trace()()) {
			std::ostringstream s;
			s << "scheduling of " << node.underlying().qualified_name() << " done";
			loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
	}
	
	/// processes the node of the output port connected to the input port and sets the buffer for the input port
	void inline scheduler::process_node_of_output_port_and_set_buffer_for_input_port(ports::output & output_port, ports::input & input_port) {
		process(output_port.parent());
		assert(&output_port.buffer());
		if(false && loggers::trace()()) {
			std::ostringstream s;
			s << "back to scheduling of input port " << input_port.underlying().qualified_name();
			loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
		input_port.underlying().buffer(&output_port.underlying().buffer());
	}

	/// set buffers for all output ports of the node from the buffer pool.
	void inline scheduler::set_buffers_for_all_output_ports_of_node_from_the_buffer_pool(node & node) {
		for(typenames::node::output_ports_type::const_iterator i(node.output_ports().begin()) ; i != node.output_ports().end() ; ++i) {
			ports::output & output_port(**i);
			set_buffer_for_output_port(output_port, buffer_pool_instance()());
		}
	}

	/// sets a buffer for the output port
	void inline scheduler::set_buffer_for_output_port(ports::output & output_port, buffer & buffer) {
		output_port.buffer(&buffer);
		output_port.reset();
		buffer += output_port;
	}

	/// decrements the remaining expected read count of the buffer and
	/// checks if the content of the buffer must be preserved for further reading.
	void inline scheduler::mark_buffer_as_read_once_more_and_check_whether_to_recycle_it_in_the_pool(ports::output & output_port, ports::input & input_port) {
		input_port.underlying().buffer(0);
		--output_port;
		--output_port.buffer();
		check_whether_to_recycle_buffer_in_the_pool(output_port);
	}
	
	/// checks if the content of the buffer must be preserved for further reading and
	/// if not recycle it in the pool.
	void inline scheduler::check_whether_to_recycle_buffer_in_the_pool(ports::output & output_port) {
		if(false && loggers::trace()()) {
			std::ostringstream s;
			s
				<< "output port " << output_port.underlying().qualified_name()
				<< ": " << static_cast<std::size_t>(output_port) << " to go, "
				<< "buffer: " << &output_port.underlying().buffer()
				<< ": " << output_port.buffer() << " to go";
			loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
		if(!output_port) {
			if(!output_port.buffer()) (*buffer_pool_instance_)(output_port.buffer()); // recycle the buffer in the pool
			output_port.buffer(0);
		}
	}

	buffer::buffer(std::size_t channels, std::size_t events) throw(std::exception)
	:
		underlying::buffer(channels, events),
		reference_count_()
	{}

	buffer::~buffer() throw() {
		assert(!*this);
	}

	scheduler::buffer_pool::buffer_pool(std::size_t channels, std::size_t events) throw(std::exception)
	:
		channels_(channels),
		events_(events)
	{}
	
	scheduler::buffer_pool::~buffer_pool() throw() {
		for(iterator i(begin()) ; i != end() ; ++i) delete *i;
	}
}}}}
