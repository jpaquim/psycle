// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\interface psycle::host::graph
#ifndef PSYCLE__HOST__GRAPH__INCLUDED
#define PSYCLE__HOST__GRAPH__INCLUDED
#pragma once
#include <psycle/engine/graph.hpp>
#define PSYCLE__DECL  PSYCLE__HOST
#include <psycle/detail/decl.hpp>
namespace psycle { namespace host {

using namespace universalis::stdlib;

class graph;
class node;

/**********************************************************************************************************************/
// graph
class PSYCLE__DECL graph {
	public:
		graph(engine::graph & engine) : engine_(engine) {}

	///\name engine
	///\{
		public:
			engine::graph & engine() const { return engine_; }
			engine::graph & engine() { return engine_; }
		private:
			engine::graph & engine_;
	///\}

	///\name schedule
	///\{
		public:
			void compute_plan();
			void clear_plan();
			
		public:
			/// maximum number of channels needed for buffers
			std::size_t channels() const throw() { return channels_; }
		private:
			std::size_t channels_;
			
		public:
			typedef std::list<node*> terminal_nodes_type;
			/// nodes with no dependency, that are processed first
			terminal_nodes_type const & terminal_nodes() const throw() { return terminal_nodes_; }
		private:
			terminal_nodes_type terminal_nodes_;

		public:
			boost::signal<void (node &)> & io_ready_signal() throw() { return io_ready_signal_; }
		private:
			boost::signal<void (node &)> io_ready_signal_;
	///\}
};

/**********************************************************************************************************************/
// node
class PSYCLE__DECL node {
	node(class graph &, engine::node &);
		
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

}}
#include <psycle/detail/decl.hpp>
#endif
