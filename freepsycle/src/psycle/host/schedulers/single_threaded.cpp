// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2006 Johan Boule <bohan@jabber.org>
// Copyright (C) 2004-2006 Psycledelics http://psycle.pastnotecut.org

///\implementation psycle::host::schedulers::single_threaded
#include PACKAGENERIC__PRE_COMPILED
#include PACKAGENERIC
#include <psycle/detail/project.private.hpp>
#include "single_threaded.hpp"
#include <universalis/processor/exception.hpp>
#include <universalis/compiler/typenameof.hpp>
#include <universalis/compiler/exceptions/ellipsis.hpp>
#include <sstream>
#include <limits>

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
#if defined PSYCLE__EXPERIMENTAL
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

namespace psycle
{
	namespace host
	{
		namespace schedulers
		{
			namespace single_threaded
			{
				graph::graph(graph::underlying_type & underlying) : graph_base(underlying)
				{
					new_node_signal().connect(boost::bind(&graph::on_new_node, this, _1));
				}

				void graph::on_new_node(typenames::node & underlying_node)
				{
std::cerr << "@@@@@@@@@@@@@@@@@@@@@ sched graph::on_new_node" << std::endl;
				}
				
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
					if(multiple_input_port())
					{
						/// note: the best algorithm would be to order the inputs with a recursive evaluation on the graph.
						// find the output port which has the minimum number of connections.
						{
							std::size_t minimum_size(std::numeric_limits<std::size_t>::max());
							ports::inputs::multiple::output_ports_type::const_iterator i(multiple_input_port()->output_ports().begin());
							for( ; i != multiple_input_port()->output_ports().end() ; ++i)
							{
								ports::output & output_port(**i);
								if(output_port.input_ports().size() < minimum_size)
								{
									minimum_size = output_port.input_ports().size();
									this->multiple_input_port_first_output_port_to_process_ = &output_port;
									if(minimum_size == 1) break; // it's already an ideal case, we can't find a better one.
								}
							}
							assert(!multiple_input_port()->output_ports().size() || this->multiple_input_port_first_output_port_to_process_);
						}
					}
					// count the number of output ports that are connected.
					for(output_ports_type::const_iterator i(output_ports().begin()) ; i != output_ports().end() ; ++i)
					{
						if((**i).input_ports().size()) ++output_port_count_;
std::cerr << "@@@@@@@@@@@@@@@@@@@@@ sched node::node " << output_port_count_ << std::endl;
					}
				}
						
				void node::on_new_output_port(typenames::ports::output & output_port)
				{
std::cerr << "@@@@@@@@@@@@@@@@@@@@@ sched node::on_new_output_port" << std::endl;
				}
				void node::on_new_single_input_port(typenames::ports::inputs::single & single_input_port)
				{
				}
				void node::on_new_multiple_input_port(typenames::ports::inputs::multiple & multiple_input_port)
				{
				}
				
				port::port(port::parent_type & parent, underlying_type & underlying) : port_base(parent, underlying) {}
				
				namespace ports
				{
					output::output(output::parent_type & parent, output::underlying_type & underlying)
					:
						output_base(parent, underlying),
						input_port_count_(underlying.input_ports().size())
					{
						reset();
					}
					
					input::input(input::parent_type & parent, input::underlying_type & underlying) : input_base(parent, underlying) {}
					
					namespace inputs
					{
						single::single(single::parent_type & parent, single::underlying_type & underlying) : single_base(parent, underlying)
						{
						}
						
						multiple::multiple(multiple::parent_type & parent, multiple::underlying_type & underlying) : multiple_base(parent, underlying)
						{
						}
					}
				}
				
				scheduler::scheduler(underlying::graph & graph) throw(std::exception)
				:
					host::scheduler<graph_type>(graph),
					buffer_pool_instance_(),
					thread_(),
					stop_requested_()
				{
				}

				scheduler::~scheduler() throw()
				{
					stop();
					delete &graph();
				}
				
				namespace
				{		
					class thread
					{
					public:
						inline thread(scheduler & scheduler) : scheduler_(scheduler) {}
						void inline operator()() { scheduler_(); }
					private:
						scheduler & scheduler_;
					};
				}
				
				void scheduler::start() throw(engine::exception)
				{
					if(loggers::information()())
					{
						loggers::information()("starting scheduler thread on graph " + graph().underlying().name() + " ...");
					}
					if(thread_)
					{
						if(loggers::information()())
						{
							loggers::information()("thread is already running");
						}
						return;
					}
					try
					{
						thread_ = new boost::thread(thread(*this));
					}
					catch(boost::thread_resource_error const & e)
					{
						std::ostringstream s; s << universalis::compiler::typenameof(e) << ": " << e.what();
						throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
					}
				}
			
				void scheduler::stop()
				{
					if(loggers::information()())
					{
						loggers::information()("terminating and joining scheduler thread on graph " + graph().underlying().name() + " ...");
					}
					if(!thread_)
					{
						if(loggers::information()())
						{
							loggers::information()("scheduler thread was not running");
						}
						return;
					}
					{
						boost::mutex::scoped_lock lock(mutex_);
						stop_requested_ = true;
					}
					thread_->join();
					if(loggers::information()())
					{
						loggers::information()("scheduler thread on graph " + graph().underlying().name() + " joined");
					}
					delete thread_; thread_ = 0;
				}
			
				bool scheduler::stop_requested()
				{
					boost::mutex::scoped_lock lock(mutex_);
					return stop_requested_;
				}
			
				void scheduler::operator()()
				{
					loggers::information()("scheduler thread started on graph " + graph().underlying().name());
					universalis::processor::exception::new_thread(universalis::compiler::typenameof(*this) + "#" + graph().underlying().name());
					try
					{
						try
						{
							try
							{
								process_loop();
							}
							catch(...)
							{
								loggers::exception()("exception in scheduler thread");
								throw;
							}
						}
						catch(std::exception const & e)
						{
							if(loggers::exception()())
							{
								std::ostringstream s;
								s << "exception: " << universalis::compiler::typenameof(e) << ": " << e.what();
								loggers::exception()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
							}
							throw;
						}
						catch(...)
						{
							if(loggers::exception()())
							{
								std::ostringstream s;
								s << "exception: " << universalis::compiler::exceptions::ellipsis();
								loggers::exception()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
							}
							throw;
						}
					}
					catch(...)
					{
						{
							boost::mutex::scoped_lock lock(mutex_);
							stop_requested_ = false;
						}
						throw;
					}
					loggers::information()("scheduler thread on graph " + graph().underlying().name() + " terminated");
					{
						boost::mutex::scoped_lock lock(mutex_);
						stop_requested_ = false;
					}
				}
				
				void scheduler::allocate() throw(std::exception)
				{
					loggers::trace()("allocating ...", UNIVERSALIS__COMPILER__LOCATION);
					int channels(0);
					for(graph_type::const_iterator i(graph().begin()) ; i != graph().end() ; ++i)
					{
						typenames::node & node(**i);
						node.underlying().start();
						if(!node.output_port_count())
						{
							if(loggers::trace()())
							{
								std::ostringstream s;
								s << "terminal node: " << node.underlying().name();
								loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
							}
							terminal_nodes_.push_back(&node);
						}
						for(typenames::node::output_ports_type::const_iterator i(node.output_ports().begin()) ; i != node.output_ports().end() ; ++i)
						{
							ports::output & output_port(**i);
							channels = std::max(channels, output_port.underlying().channels());
						}
					}
					if(loggers::trace()())
					{
						std::ostringstream s;
						s << "channels: " << channels;
						loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
					}
					buffer_pool_instance_ = new buffer_pool(channels, graph().underlying().events_per_buffer());
				}
				
				void scheduler::free() throw()
				{
					loggers::trace()("freeing ...", UNIVERSALIS__COMPILER__LOCATION);
					delete buffer_pool_instance_; buffer_pool_instance_ = 0;
					terminal_nodes_.clear();
				}

				void scheduler::process_loop()
				{
					try
					{
						allocate();
						while(!stop_requested())
						{
							if(loggers::trace()())
							{
								std::ostringstream s;
								s << "process loop";
								loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
							}
							boost::mutex::scoped_lock lock(graph().underlying().mutex());
							for(graph::const_iterator i(graph().begin()) ; i != graph().end() ; ++i)
							{
								node & node(**i);
								if(node.processed()) node.underlying().reset();
							}
							for(terminal_nodes_type::iterator i(terminal_nodes_.begin()) ; i != terminal_nodes_.end() ; ++i)
							{
								node & node(**i);
								process(node);
							}
						}
					}
					catch(...)
					{
						free();
						throw;
					}
					free();
				}
				
				void scheduler::process(node & node)
				{
					if(node.processed()) return;
					node.mark_as_processed();
					if(loggers::trace()())
					{
						std::ostringstream s;
						s << "scheduling " << node.underlying().qualified_name() << " ###################";
						loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
					}
					{
						// get node input buffers by processing the dependencies of the node
						for(node::single_input_ports_type::const_iterator i(node.single_input_ports().begin()) ; i != node.single_input_ports().end() ; ++i)
						{
							ports::inputs::single & single_input_port(**i);
							if(single_input_port.output_port()) process_node_of_output_port_and_set_buffer_for_input_port(*single_input_port.output_port(), single_input_port);
						}
					}
					if(!node.multiple_input_port()) // the node has no multiple input port: simple case
					{
						set_buffers_for_all_output_ports_of_node_from_the_buffer_pool(node);
						node.underlying().process();
					}
					else if(node.multiple_input_port()->output_ports().size()) // the node has a multiple input port: complex case
					{
						// get first output to process 
						ports::output & first_output_port_to_process(node.multiple_input_port_first_output_port_to_process());
						{
							// process with first input buffer
							process_node_of_output_port_and_set_buffer_for_input_port(first_output_port_to_process, *node.multiple_input_port());
							if(node.multiple_input_port()->underlying().single_connection_is_identity_transform()) // this is the identity transform when we have a single input
							{
								ports::output & output_port(*node.output_ports().front());
								if
								(
									node.multiple_input_port()->buffer() == 1 || // We are the last input port to read the buffer of the output port, so, we can take over its buffer.
									node.multiple_input_port()->output_ports().size() == 1 // We have a single input, so, this is the identity transform, i.e., the buffer will not be modified.
								)
								{
									if(loggers::trace()())
									{
										std::ostringstream s;
										s << node.underlying().qualified_name() << ": copying pointer of input buffer to pointer of output buffer";
										loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
									}
									// copy pointer of input buffer to pointer of output buffer
									set_buffer_for_output_port(output_port, node.multiple_input_port()->buffer());
								}
								else // we have several inputs, so, this cannot by the identity transform, i.e., the buffer would be modified. but its content must be preserved for further reading
								{
									// get buffer for output port
									set_buffer_for_output_port(output_port, buffer_pool_instance()());
									// copy content of input buffer to output buffer
									if(loggers::trace()())
									{
										std::ostringstream s;
										s << node.underlying().qualified_name() << ": copying content of input buffer to output buffer";
										loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
									}
									output_port.buffer().copy(node.multiple_input_port()->buffer(), node.multiple_input_port()->underlying().channels());
								}
							}
							else // this is never the identity transform
							{
								set_buffers_for_all_output_ports_of_node_from_the_buffer_pool(node);
								node.underlying().process_first();
							}
							mark_buffer_as_read_once_more_and_check_whether_to_recycle_it_in_the_pool(first_output_port_to_process, *node.multiple_input_port());
						}
						// process with remaining input buffers
						for(ports::inputs::multiple::output_ports_type::const_iterator i(node.multiple_input_port()->output_ports().begin()) ; i != node.multiple_input_port()->output_ports().end() ; ++i)
						{
							ports::output & output_port(**i);
							if(&output_port == &first_output_port_to_process) continue;
							process_node_of_output_port_and_set_buffer_for_input_port(output_port, *node.multiple_input_port());
							node.underlying().process();
							mark_buffer_as_read_once_more_and_check_whether_to_recycle_it_in_the_pool(output_port, *node.multiple_input_port());
						}
					}
					 // check if the content of the node input ports buffers must be preserved for further reading
					for(typenames::node::single_input_ports_type::const_iterator i(node.single_input_ports().begin()) ; i != node.single_input_ports().end() ; ++i)
					{
						ports::inputs::single & single_input_port(**i);
						if(single_input_port.output_port()) mark_buffer_as_read_once_more_and_check_whether_to_recycle_it_in_the_pool(*single_input_port.output_port(), single_input_port);
					}
					 // check if the content of the node output ports buffers must be preserved for further reading
					for(typenames::node::output_ports_type::const_iterator i(node.output_ports().begin()) ; i != node.output_ports().end() ; ++i)
					{
						ports::output & output_port(**i);
						check_whether_to_recycle_buffer_in_the_pool(output_port);
					}
					if(loggers::trace()())
					{
						std::ostringstream s;
						s << "scheduling of " << node.underlying().qualified_name() << " done ###################";
						loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
					}
				}
				
				/// processes the node of the output port connected to the input port and sets the buffer for the input port
				void inline scheduler::process_node_of_output_port_and_set_buffer_for_input_port(ports::output & output_port, ports::input & input_port)
				{
					process(output_port.parent());
					assert(&output_port.buffer());
					if(loggers::trace()())
					{
						std::ostringstream s;
						s << "back to scheduling of input port " << input_port.underlying().qualified_name() << " ###################";
						loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
					}
					input_port.underlying().buffer(&output_port.underlying().buffer());
				}

				/// set buffers for all output ports of the node from the buffer pool.
				void inline scheduler::set_buffers_for_all_output_ports_of_node_from_the_buffer_pool(node & node)
				{
					for(typenames::node::output_ports_type::const_iterator i(node.output_ports().begin()) ; i != node.output_ports().end() ; ++i)
					{
						ports::output & output_port(**i);
						set_buffer_for_output_port(output_port, buffer_pool_instance()());
					}
				}

				/// sets a buffer for the output port
				void inline scheduler::set_buffer_for_output_port(ports::output & output_port, buffer & buffer)
				{
					output_port.buffer(&buffer);
					output_port.reset();
					buffer += output_port;
				}

				/// decrements the remaining expected read count of the buffer and
				/// checks if the content of the buffer must be preserved for further reading.
	 			void inline scheduler::mark_buffer_as_read_once_more_and_check_whether_to_recycle_it_in_the_pool(ports::output & output_port, ports::input & input_port)
				{
					input_port.underlying().buffer(0);
					--output_port;
					--output_port.buffer();
					check_whether_to_recycle_buffer_in_the_pool(output_port);
				}
				
				/// checks if the content of the buffer must be preserved for further reading and
				/// if not recycle it in the pool.
				void inline scheduler::check_whether_to_recycle_buffer_in_the_pool(ports::output & output_port)
				{
					if(loggers::trace()())
					{
						std::ostringstream s;
						s
							<< "output port " << output_port.underlying().qualified_name()
							<< ": " << static_cast<int>(output_port) << " to go, "
							<< "buffer: " << &output_port.underlying().buffer()
							<< ": " << output_port.buffer() << " to go";
						loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
					}
					if(!output_port)
					{
						if(!output_port.buffer()) (*buffer_pool_instance_)(output_port.buffer()); // recycle the buffer in the pool
						output_port.buffer(0);
					}
				}

				buffer::buffer(int const & channels, int const & events) throw(std::exception)
				:
					underlying::buffer(channels, events),
					reference_count_()
				{
				}

				buffer::~buffer() throw()
				{
					assert(!*this);
				}

				scheduler::buffer_pool::buffer_pool(int const & channels, int const & events) throw(std::exception)
				:
					channels_(channels),
					events_(events)
				{
				}
				
				scheduler::buffer_pool::~buffer_pool() throw()
				{
					for(iterator i(begin()) ; i != end() ; ++i) delete *i;
				}
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
#else // !defined PSYCLE__EXPERIMENTAL
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

namespace psycle
{
	namespace host
	{
		namespace schedulers
		{
			namespace single_threaded
			{
				scheduler::scheduler(engine::graph & graph) throw(std::exception)
				:
					host::scheduler(graph),
					buffer_pool_instance_(),
					thread_(),
					stop_requested_()
				{
				}

				scheduler::~scheduler() throw()
				{
					stop();
				}
				
				namespace
				{		
					class thread
					{
					public:
						inline thread(scheduler & scheduler) : scheduler_(scheduler) {}
						void inline operator()() { scheduler_(); }
					private:
						scheduler & scheduler_;
					};
				}
				
				void scheduler::start() throw(engine::exception)
				{
					if(loggers::information()())
					{
						loggers::information()("starting scheduler thread ...");
					}
					if(thread_)
					{
						if(loggers::information()())
						{
							loggers::information()("thread is already running");
						}
						return;
					}
					try
					{
						thread_ = new boost::thread(thread(*this));
					}
					catch(boost::thread_resource_error const & e)
					{
						std::ostringstream s; s << universalis::compiler::typenameof(e) << ": " << e.what();
						throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
					}
				}
			
				void scheduler::stop()
				{
					if(loggers::information()())
					{
						loggers::information()("terminating and joining scheduler thread on graph " + graph().name() + " ...");
					}
					if(!thread_)
					{
						if(loggers::information()())
						{
							loggers::information()("scheduler thread was not running");
						}
						return;
					}
					{
						boost::mutex::scoped_lock lock(mutex_);
						stop_requested_ = true;
					}
					thread_->join();
					if(loggers::information()())
					{
						loggers::information()("scheduler thread on graph " + graph().name() + " joined");
					}
					delete thread_; thread_ = 0;
				}
			
				bool scheduler::stop_requested()
				{
					boost::mutex::scoped_lock lock(mutex_);
					return stop_requested_;
				}
			
				void scheduler::operator()()
				{
					loggers::information()("scheduler thread started on graph " + graph().name());
					universalis::processor::exception::new_thread(universalis::compiler::typenameof(*this) + "#" + graph().name());
					try
					{
						try
						{
							try
							{
								process_loop();
							}
							catch(...)
							{
								loggers::exception()("exception in scheduler thread");
								throw;
							}
						}
						catch(std::exception const & e)
						{
							if(loggers::exception()())
							{
								std::ostringstream s;
								s << "exception: " << universalis::compiler::typenameof(e) << ": " << e.what();
								loggers::exception()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
							}
							throw;
						}
						catch(...)
						{
							if(loggers::exception()())
							{
								std::ostringstream s;
								s << "exception: " << universalis::compiler::exceptions::ellipsis();
								loggers::exception()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
							}
							throw;
						}
					}
					catch(...)
					{
						{
							boost::mutex::scoped_lock lock(mutex_);
							stop_requested_ = false;
						}
						throw;
					}
					loggers::information()("scheduler thread on graph " + graph().name() + " terminated");
					{
						boost::mutex::scoped_lock lock(mutex_);
						stop_requested_ = false;
					}
				}
				
				namespace
				{
					class node_schedule : public engine::node::schedule
					{
						public: node_schedule::node_schedule(engine::node & node) throw(std::exception)
						:
							multiple_input_port_first_output_port_to_process_(),
							connected_output_ports_(),
							processed_(true) // set to true because reset() is called first in the processing loop
						{
							if(node.multiple_input_port())
							{
								///\todo the best algorithm would be to order the inputs with a recursive evaluation on the graph.
								// find the output port which has the minimum number of connections.
								{
									std::size_t minimum_size(std::numeric_limits<std::size_t>::max());
									engine::ports::inputs::multiple::output_ports_type::const_iterator i(node.multiple_input_port()->output_ports().begin());
									for( ; i != node.multiple_input_port()->output_ports().end() ; ++i)
									{
										engine::ports::output & output_port(**i);
										if(output_port.input_ports().size() < minimum_size)
										{
											minimum_size = output_port.input_ports().size();
											multiple_input_port_first_output_port_to_process_ = &output_port;
											if(minimum_size == 1) break; // It's already an ideal case, we can't find a better one.
										}
									}
									assert(!node.multiple_input_port()->output_ports().size() || multiple_input_port_first_output_port_to_process_);
								}
							}
							// count the number of output ports that are connected.
							for(engine::node::output_ports_type::const_iterator i(node.output_ports().begin()) ; i != node.output_ports().end() ; ++i)
							{
								if((**i).input_ports().size()) ++connected_output_ports_;
							}
						}
							
						public:  engine::ports::output inline & multiple_input_port_first_output_port_to_process() throw() { assert(multiple_input_port_first_output_port_to_process_); return *multiple_input_port_first_output_port_to_process_; }
						private: engine::ports::output        * multiple_input_port_first_output_port_to_process_;

						public:  int inline const & connected_output_ports() const throw() { return connected_output_ports_; }
						private: int                connected_output_ports_;
						
						public: void inline reset() throw() { assert(processed()); processed_= false; }
						public: void inline mark_as_processed() throw() { processed_= true; assert(processed()); }
						public: bool inline const & processed() const throw() { return processed_; }
						private: bool processed_;
					};

					class output_port_schedule : public engine::ports::output::schedule
					{
						public: output_port_schedule::output_port_schedule(engine::ports::output & output_port) throw(std::exception)
						:
							input_ports_(output_port.input_ports().size())
						{
							reset();
						}
							
						public: int inline const & input_ports() const throw() { return input_ports_; }
						private: int input_ports_;
		
						/// convertible to int
						///\returns the reference count.
						public: inline operator int const & () const throw() { return input_ports_remaining_; }
						public: output_port_schedule inline & operator--() throw() { --input_ports_remaining_; assert(*this >= 0); return *this; }
						public: void inline reset() throw() { input_ports_remaining_ = input_ports(); }
						private: int input_ports_remaining_;
					};
				}
				
				void scheduler::allocate() throw(std::exception)
				{
					loggers::trace()("allocating ...", UNIVERSALIS__COMPILER__LOCATION);
					int channels(0);
					for(engine::graph::const_iterator i(graph().begin()) ; i != graph().end() ; ++i)
					{
						engine::node & node(**i);
						node.start();
						node_schedule & node_schedule_instance(*new node_schedule(node));
						node.schedule_instance() = &node_schedule_instance;
						if(!node_schedule_instance.connected_output_ports())
						{
							if(loggers::trace()())
							{
								std::ostringstream s;
								s << "terminal node: " << node.name();
								loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
							}
							terminal_nodes_.push_back(&node);
						}
						for(engine::node::output_ports_type::const_iterator i(node.output_ports().begin()) ; i != node.output_ports().end() ; ++i)
						{
							engine::ports::output & output_port(**i);
							channels = std::max(channels, output_port.channels());
							output_port_schedule & output_port_schedule_instance(*new output_port_schedule(output_port));
							output_port.schedule_instance() = &output_port_schedule_instance;
						}
					}
					if(loggers::trace()())
					{
						std::ostringstream s;
						s << "channels: " << channels;
						loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
					}
					buffer_pool_instance_ = new buffer_pool(channels, graph().events_per_buffer());
				}
				
				void scheduler::free() throw()
				{
					loggers::trace()("freeing ...", UNIVERSALIS__COMPILER__LOCATION);
					for(engine::graph::const_iterator i(graph().begin()) ; i != graph().end() ; ++i)
					{
						engine::node & node(**i);
						node.stop();
						for(engine::node::output_ports_type::const_iterator i(node.output_ports().begin()) ; i != node.output_ports().end() ; ++i)
						{
							engine::ports::output & output_port(**i);
							delete output_port.schedule_instance(); output_port.schedule_instance() = 0;
						}
						delete node.schedule_instance(); node.schedule_instance() = 0;
					}
					delete buffer_pool_instance_; buffer_pool_instance_ = 0;
					terminal_nodes_.clear();
				}

				void scheduler::process_loop()
				{
					try
					{
						allocate();
						while(!stop_requested())
						{
							boost::mutex::scoped_lock lock(graph().mutex());
							for(engine::graph::const_iterator i(graph().begin()) ; i != graph().end() ; ++i)
							{
								engine::node & node(**i);
								node_schedule & node_schedule_instance(*static_cast<node_schedule*>(node.schedule_instance()));
								if(node_schedule_instance.processed())
								{
									node_schedule_instance.reset();
									node.reset();
								}
							}
							for(terminal_nodes_type::iterator i(terminal_nodes_.begin()) ; i != terminal_nodes_.end() ; ++i)
							{
								engine::node & node(**i);
								process(node);
							}
						}
					}
					catch(...)
					{
						free();
						throw;
					}
					free();
				}
				
				void scheduler::process(engine::node & node)
				{
					node_schedule & node_schedule_instance(*static_cast<node_schedule*>(node.schedule_instance()));
					if(node_schedule_instance.processed()) return;
					node_schedule_instance.mark_as_processed();
					if(loggers::trace()())
					{
						std::ostringstream s;
						s << "scheduling " << node.qualified_name() << " ###################";
						loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
					}
					{
						// get node input buffers by processing the dependencies of the node
						for(engine::node::single_input_ports_type::const_iterator i(node.single_input_ports().begin()) ; i != node.single_input_ports().end() ; ++i)
						{
							engine::ports::inputs::single & single_input_port(**i);
							if(single_input_port.output_port()) process_node_of_output_port_and_set_buffer_for_input_port(*single_input_port.output_port(), single_input_port);
						}
					}
					if(!node.multiple_input_port()) // the node has no multiple input port: simple case
					{
						set_buffers_for_all_output_ports_of_node_from_the_buffer_pool(node);
						node.process();
					}
					else if(node.multiple_input_port()->output_ports().size()) // the node has a multiple input port: complex case
					{
						// get first output to process 
						engine::ports::output & first_output_port_to_process(node_schedule_instance.multiple_input_port_first_output_port_to_process());
						{
							// process with first input buffer
							process_node_of_output_port_and_set_buffer_for_input_port(first_output_port_to_process, *node.multiple_input_port());
							if(node.multiple_input_port()->single_connection_is_identity_transform()) // this is the identity transform when we have a single input
							{
								engine::ports::output & output_port(*node.output_ports().front());
								if
								(
									static_cast<buffer&>(node.multiple_input_port()->buffer()) == 1 || // We are the last input port to read the buffer of the output port, so, we can take over its buffer.
									node.multiple_input_port()->output_ports().size() == 1 // We have a single input, so, this is the identity transform, i.e., the buffer will not be modified.
								)
								{
									if(loggers::trace()())
									{
										std::ostringstream s;
										s << node.qualified_name() << ": copying pointer of input buffer to pointer of output buffer";
										loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
									}
									// copy pointer of input buffer to pointer of output buffer
									set_buffer_for_output_port(output_port, node.multiple_input_port()->buffer());
								}
								else // we have several inputs, so, this cannot by the identity transform, i.e., the buffer would be modified. but its content must be preserved for further reading
								{
									// get buffer for output port
									set_buffer_for_output_port(output_port, buffer_pool_instance()());
									// copy content of input buffer to output buffer
									if(loggers::trace()())
									{
										std::ostringstream s;
										s << node.qualified_name() << ": copying content of input buffer to output buffer";
										loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
									}
									output_port.buffer().copy(node.multiple_input_port()->buffer(), node.multiple_input_port()->channels());
								}
							}
							else // this is never the identity transform
							{
								set_buffers_for_all_output_ports_of_node_from_the_buffer_pool(node);
								node.process_first();
							}
							mark_buffer_as_read_once_more_and_check_whether_to_recycle_it_in_the_pool(first_output_port_to_process, *node.multiple_input_port());
						}
						// process with remaining input buffers
						for(engine::ports::inputs::multiple::output_ports_type::const_iterator i(node.multiple_input_port()->output_ports().begin()) ; i != node.multiple_input_port()->output_ports().end() ; ++i)
						{
							engine::ports::output & output_port(**i);
							if(&output_port == &first_output_port_to_process) continue;
							process_node_of_output_port_and_set_buffer_for_input_port(output_port, *node.multiple_input_port());
							node.process();
							mark_buffer_as_read_once_more_and_check_whether_to_recycle_it_in_the_pool(output_port, *node.multiple_input_port());
						}
					}
					 // check if the content of the node input ports buffers must be preserved for further reading
					for(engine::node::single_input_ports_type::const_iterator i(node.single_input_ports().begin()) ; i != node.single_input_ports().end() ; ++i)
					{
						engine::ports::inputs::single & single_input_port(**i);
						if(single_input_port.output_port()) mark_buffer_as_read_once_more_and_check_whether_to_recycle_it_in_the_pool(*single_input_port.output_port(), single_input_port);
					}
					 // check if the content of the node output ports buffers must be preserved for further reading
					for(engine::node::output_ports_type::const_iterator i(node.output_ports().begin()) ; i != node.output_ports().end() ; ++i)
					{
						engine::ports::output & output_port(**i);
						check_whether_to_recycle_buffer_in_the_pool(output_port);
					}
					if(loggers::trace()())
					{
						std::ostringstream s;
						s << "scheduling of " << node.qualified_name() << " done ###################";
						loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
					}
				}
				
				/// processes the node of the output port connected to the input port and sets the buffer for the input port
				void inline scheduler::process_node_of_output_port_and_set_buffer_for_input_port(engine::ports::output & output_port, engine::ports::input & input_port)
				{
					process(output_port.node());
					assert(&output_port.buffer());
					if(loggers::trace()())
					{
						std::ostringstream s;
						s << "back to scheduling of input port " << input_port.qualified_name() << " ###################";
						loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
					}
					input_port.buffer(&output_port.buffer());
				}

				/// set buffers for all output ports of the node from the buffer pool.
				void inline scheduler::set_buffers_for_all_output_ports_of_node_from_the_buffer_pool(engine::node & node)
				{
					for(engine::node::output_ports_type::const_iterator i(node.output_ports().begin()) ; i != node.output_ports().end() ; ++i)
					{
						engine::ports::output & output_port(**i);
						set_buffer_for_output_port(output_port, buffer_pool_instance()());
					}
				}

				/// sets a buffer for the output port
				void inline scheduler::set_buffer_for_output_port(engine::ports::output & output_port, engine::buffer & engine_buffer_instance)
				{
					buffer & buffer_instance(static_cast<buffer&>(engine_buffer_instance));
					output_port.buffer(&buffer_instance);
					output_port_schedule & output_port_schedule_instance(*static_cast<output_port_schedule*>(output_port.schedule_instance()));
					output_port_schedule_instance.reset();
					buffer_instance += output_port_schedule_instance;
				}

				/// decrements the remaining expected read count of the buffer and
				/// checks if the content of the buffer must be preserved for further reading.
	 			void inline scheduler::mark_buffer_as_read_once_more_and_check_whether_to_recycle_it_in_the_pool(engine::ports::output & output_port, engine::ports::input & input_port)
				{
					input_port.buffer(0);
					output_port_schedule & output_port_schedule_instance(*static_cast<output_port_schedule*>(output_port.schedule_instance()));
					--output_port_schedule_instance;
					buffer & buffer_instance(static_cast<buffer&>(output_port.buffer()));
					--buffer_instance;
					check_whether_to_recycle_buffer_in_the_pool(output_port);
				}
				
				/// checks if the content of the buffer must be preserved for further reading and
				/// if not recycle it in the pool.
				void inline scheduler::check_whether_to_recycle_buffer_in_the_pool(engine::ports::output & output_port)
				{
					output_port_schedule & output_port_schedule_instance(*static_cast<output_port_schedule*>(output_port.schedule_instance()));
					if(loggers::trace()())
					{
						std::ostringstream s;
						s
							<< "output port " << output_port.qualified_name() << ": " << output_port_schedule_instance << " to go, "
							<< "buffer: " << &output_port.buffer() << ": " << static_cast<buffer&>(output_port.buffer()) << " to go";
						loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
					}
					if(!output_port_schedule_instance)
					{
						{
							buffer & buffer_instance(static_cast<buffer&>(output_port.buffer()));
							if(!buffer_instance) (*buffer_pool_instance_)(buffer_instance); // recycle the buffer in the pool
						}
						output_port.buffer(0);
					}
				}

				scheduler::buffer::buffer(int const & channels, int const & events) throw(std::exception)
				:
					engine::buffer(channels, events),
					reference_count_()
				{
				}

				scheduler::buffer::~buffer() throw()
				{
					assert(!*this);
				}

				scheduler::buffer_pool::buffer_pool(int const & channels, int const & events) throw(std::exception)
				:
					channels_(channels),
					events_(events)
				{
				}
				
				scheduler::buffer_pool::~buffer_pool() throw()
				{
					for(iterator i(begin()) ; i != end() ; ++i) delete *i;
				}
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
#endif // !defined PSYCLE__EXPERIMENTAL
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
