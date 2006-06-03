// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2006 Johan Boule <bohan@jabber.org>
// Copyright (C) 2004-2006 Psycledelics http://psycle.pastnotecut.org

///\interface psycle::host::schedulers::single_threaded
#pragma once
#include "../scheduler.hpp"
#include <psycle/generic/wrappers.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <list>
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PACKAGENERIC__MODULE__SOURCE__PSYCLE__HOST__SCHEDULERS__SINGLE_THREADED
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace psycle
{
	namespace host
	{
		namespace schedulers
		{
			/// a scheduler using only one thread
			namespace single_threaded
			{
				namespace underlying = host::underlying;

				class graph;
				class node;
				class port;
				namespace ports
				{
					class output;
					class input;
					namespace inputs
					{
						class single;
						class multiple;
					}
				}

				namespace typenames
				{
					using namespace single_threaded;
					class typenames : public generic::typenames<graph, node, port, ports::output, ports::input, ports::inputs::single, ports::inputs::multiple, underlying::typenames::typenames> {};
				}

				/// underlying::buffer with a reference counter.
				class buffer : public underlying::buffer
				{
					public:
						/// creates a buffer with an initial reference count set to 0.
						buffer(int const & channels, int const & events) throw(std::exception);
						/// deletes the buffer
						///\pre the reference count must be 0.
						virtual ~buffer() throw();
						/// convertible to int
						///\returns the reference count.
						inline operator int const & () const throw() { return reference_count_; }
						/// increments the reference count.
						inline buffer & operator+=(int const & more) throw() { reference_count_ += more; return *this; }
						/// decrements the reference count by 1.
						inline buffer & operator--() throw() { --reference_count_; assert(*this >= 0); return *this; }
					private:
						int reference_count_;
				};
				
				typedef generic::wrappers::graph<typenames::typenames> graph_base;
				class UNIVERSALIS__COMPILER__DYNAMIC_LINK graph : public graph_base
				{
					protected: friend class factory;
						graph(underlying_type &);
						void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES after_construction();

					///\name signal slots
					///\{
						private:
							void on_new_node(typenames::node &);
					///\}
				};

				typedef generic::wrappers::port<typenames::typenames> port_base;
				class UNIVERSALIS__COMPILER__DYNAMIC_LINK port : public port_base
				{
					protected: friend class factory;
						port(parent_type &, underlying_type &);

					///\name buffer
					///\{	
						public:
							/// assigns a buffer to this port (or unassigns if 0) only if the given buffer is different.
							void inline buffer(typenames::buffer * const buffer) { underlying().buffer(buffer); }
							/// the buffer to read or write data from or to (buffers are shared accross several ports).
							typenames::buffer inline & buffer() const throw() { return static_cast<typenames::buffer &>(underlying().buffer()); }
					///\}
				};
				
				namespace ports
				{
					typedef generic::wrappers::ports::output<typenames::typenames> output_base;
					class UNIVERSALIS__COMPILER__DYNAMIC_LINK output : public output_base
					{
						protected: friend class factory;
							output(parent_type &, underlying_type &);
						
						///\name schedule
						///\{	
							public:  int inline const & input_port_count() const throw() { return input_port_count_; }
							private: int                input_port_count_;
			
							public:
								/// convertible to int
								///\returns the reference count.
								inline operator int const & () const throw() { return input_ports_remaining_; }
								output inline & operator--() throw() { --input_ports_remaining_; assert(*this >= 0); return *this; }
								void inline reset() throw() { input_ports_remaining_ = input_port_count(); }
							private:
								int input_ports_remaining_;
						///\}
					};

					typedef generic::wrappers::ports::input<typenames::typenames> input_base;
					class UNIVERSALIS__COMPILER__DYNAMIC_LINK input : public input_base
					{
						protected: friend class factory;
							input(parent_type &, underlying_type &);
					};

					namespace inputs
					{
						typedef generic::wrappers::ports::inputs::single<typenames::typenames> single_base;
						class UNIVERSALIS__COMPILER__DYNAMIC_LINK single : public single_base
						{
							protected: friend class factory;
								single(parent_type &, underlying_type &);
						};

						typedef generic::wrappers::ports::inputs::multiple<typenames::typenames> multiple_base;
						class UNIVERSALIS__COMPILER__DYNAMIC_LINK multiple : public multiple_base
						{
							protected: friend class factory;
								multiple(parent_type &, underlying_type &);
						};
					}
				}

				typedef generic::wrappers::node<typenames::typenames> node_base;
				class UNIVERSALIS__COMPILER__DYNAMIC_LINK node : public node_base
				{
					protected: friend class factory;
						node(parent_type &, underlying_type &);
						void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES after_construction(); friend class graph; // init code moved to graph since it deals with connections
						
					///\name signal slots
					///\{					
						private:
							void on_new_output_port(typenames::ports::output &);
							void on_new_single_input_port(typenames::ports::inputs::single &);
							void on_new_multiple_input_port(typenames::ports::inputs::multiple &);
					///\}
					
					///\name schedule
					///\{
						public:  ports::output inline & multiple_input_port_first_output_port_to_process() throw() { assert(multiple_input_port_first_output_port_to_process_); return *multiple_input_port_first_output_port_to_process_; }
						private: ports::output        * multiple_input_port_first_output_port_to_process_;

						public:  int inline const & output_port_count() const throw() { return output_port_count_; }
						private: int                output_port_count_;
						
						public:  void inline UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES reset() throw() { assert(processed()); processed(false); underlying().reset(); }
						public:  void inline mark_as_processed() throw() { processed(true); }
						public:  void inline         processed(bool processed) throw() { assert(this->processed() != processed); this->processed_ = processed; assert(this->processed() == processed); }
						public:  bool inline const & processed() const throw() { return processed_; }
						private: bool                processed_;
					///\}
				};
				
				/// a scheduler using only one thread
				class UNIVERSALIS__COMPILER__DYNAMIC_LINK scheduler : public host::scheduler<graph>
				{
					public:
						scheduler(graph::underlying_type &) throw(std::exception);
						virtual ~scheduler() throw();
						void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES start() throw(underlying::exception);
						void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES stop();
						void operator()();
					private:
						/// Flyweight pattern [Gamma95].
						/// a pool of buffers that can be used for input and output ports of the nodes of the graph.
						class buffer_pool : protected std::list<buffer*>
						{
							public:
								buffer_pool(int const & channels, int const & events) throw(std::exception);
								virtual ~buffer_pool() throw();
								/// gets a buffer from the pool.
								buffer inline & operator()()
								{
									if(false && loggers::trace()())
									{
										std::ostringstream s;
										s << "buffer requested, pool size before: " << size();
										loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
									}
									if(empty())
									{
										return *new buffer(channels_, events_);
									}
									else
									{
										buffer & result(*back());
										assert("reference count is zero: " && !result);
										pop_back();
										return result;
									}
								}
								/// recycles a buffer in the pool.
								void inline operator()(buffer & buffer)
								{
									assert(&buffer);
									assert("reference count is zero: " && !buffer);
									assert(buffer.size() >= this->channels_);
									assert(buffer.events() >= this->events_);
									if(false && loggers::trace()())
									{
										std::ostringstream s;
										s << "buffer " << &buffer << " given back, pool size before: " << size();
										loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
									}
									push_back(&buffer);
								}
							private:
								int channels_, events_;
						} * buffer_pool_instance_;
						buffer_pool inline & buffer_pool_instance() throw() { return *buffer_pool_instance_; }
						boost::thread * thread_;
						boost::mutex mutable mutex_;
						bool stop_requested_;
						bool stop_requested();
						void process_loop();
						void process(node &);
						void process_node_of_output_port_and_set_buffer_for_input_port(ports::output &, ports::input &);
						void set_buffer_for_output_port(ports::output &, buffer &);
						void set_buffers_for_all_output_ports_of_node_from_the_buffer_pool(node &);
						void mark_buffer_as_read_once_more_and_check_whether_to_recycle_it_in_the_pool(ports::output &, ports::input &);
						void check_whether_to_recycle_buffer_in_the_pool(ports::output &);
						typedef std::list<node*> terminal_nodes_type;
						terminal_nodes_type terminal_nodes_;
						void allocate() throw(std::exception);
						void free() throw();
				};
			}
		}
	}
}
#include <universalis/compiler/dynamic_link/end.hpp>
