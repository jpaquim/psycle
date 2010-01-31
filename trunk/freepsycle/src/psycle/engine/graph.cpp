// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2009 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>

///\implementation psycle::engine::graph
#include <psycle/detail/project.private.hpp>
#include "graph.hpp"
#include "buffer.hpp"
#include "reference_counter.hpp"
#include <universalis/compiler/typenameof.hpp>
//#include <boost/foreach.hpp> // boost foreach is in v 1.34
#include <map>
#include <iomanip>
namespace psycle { namespace engine {

/**********************************************************************************************************************/
// graph

graph::graph(graph::name_type const & name)
:
	named(name)
{
	if(loggers::information()()) {
		std::ostringstream s;
		s << qualified_name() << ": new engine graph";
		loggers::information()(s.str());
	}
	events_per_buffer_ = 1024;
}

graph::name_type graph::qualified_name() const {
	return name();
}

graph::~graph() {
	if(loggers::information()()) {
		std::ostringstream s;
		s << qualified_name() << ": deleting engine graph";
		loggers::information()(s.str());
	}

	while(!empty()) {
		node & node(**begin());
		erase(*begin()); // work around to bypass the compiler's no-aliasing optimisation

		if(loggers::information()()) {
			std::ostringstream s;
			s << node.qualified_name() << ": deleting engine node instance of " << universalis::compiler::typenameof(node) << " from loaded library " << node.plugin_library_reference().name();
			loggers::information()(s.str());
		}

		reference_counter & reference_counter(node.plugin_library_reference());
		node.free_heap();
		--reference_counter;
	}
	assert(empty());
}
	
void graph::dump(std::ostream & out, std::size_t tabulations) const {
	for(std::size_t t(0) ; t < tabulations ; ++t) out << '\t';
	out << name() << '\n';
	for(const_iterator i = begin() ; i != end() ; ++i) (**i).dump(out, tabulations + 1);
}

std::ostream & operator<<(std::ostream & out, graph const & graph) {
	graph.dump(out);
	return out;
}

/**********************************************************************************************************************/
// node

node::node(class plugin_library_reference & plugin_library_reference, class graph & graph, node::name_type const & name)
:
	bases::node(graph),
	named(name),
	plugin_library_reference_(plugin_library_reference),
	opened_(),
	started_()
{
	if(loggers::trace()()) {
		std::ostringstream s;
		s << qualified_name() << ": new engine node";
		loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
}

node::~node() {
	if(loggers::trace()()) {
		std::ostringstream s;
		s << qualified_name() << ": deleting engine node";
		loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
	if(multiple_input_port()) multiple_input_port()->free_heap();
	for(single_input_ports_type::const_iterator i(single_input_ports().begin()) ; i != single_input_ports().end() ; ++i) (**i).free_heap();
	for(output_ports_type::const_iterator i(output_ports().begin()) ; i != output_ports().end() ; ++i) (**i).free_heap();
}

node::name_type node::qualified_name() const {
	return graph().qualified_name() + '.' + name();
}

void node::io_ready(bool io_ready) {
	bool was_io_ready;
	{ scoped_lock lock(mutex_);
		was_io_ready = io_ready_;
		io_ready_ = io_ready;
	}
	if(!was_io_ready && io_ready) io_ready_signal_(*this);
}

ports::input * const node::input_port(name_type const & name) const {
	if(multiple_input_port() && multiple_input_port()->name() == name) return multiple_input_port();
	for(single_input_ports_type::const_iterator i(single_input_ports().begin()) ; i != single_input_ports().end() ; ++i) if((**i).name() == name) return *i;
	if(loggers::warning()()) {
		std::ostringstream s;
		s << qualified_name() << ": input port not found: " << name;
		loggers::warning()(s.str());
	}
	return 0;
}

ports::output * const node::output_port(name_type const & name) const {
	for(output_ports_type::const_iterator i(output_ports().begin()) ; i != output_ports().end() ; ++i) if((**i).name() == name) return *i;
	if(loggers::warning()()) {
		std::ostringstream s;
		s << qualified_name() << ": output port not found: " << name;
		loggers::warning()(s.str());
	}
	return 0;
}

void node::do_open() throw(std::exception) {
	if(loggers::trace()()) {
		std::ostringstream s;
		s << qualified_name() << ": opening";
		loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
}

void node::do_start() throw(std::exception) {
	if(loggers::trace()()) {
		std::ostringstream s;
		s << qualified_name() << ": starting";
		loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
	io_ready_ = true;
}

void node::do_stop() throw(std::exception) {
	if(loggers::trace()()) {
		std::ostringstream s;
		s << qualified_name() << ": stopping";
		loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
}

void node::do_close() throw(std::exception) {
	if(loggers::trace()()) {
		std::ostringstream s;
		s << qualified_name() << ": closing";
		loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
}

void node::quaquaversal_propagation_of_seconds_per_event_change_notification_from_port(port const & port) {
#if 0 // boost foreach is in v 1.34
	if(std::find(single_input_ports().begin(), single_input_ports().end(), &port) != single_input_ports().end()) {
		BOOST_FOREACH(ports::inputs::single * const input_port, single_input_ports())
			if(&port != input_port) input_port->propagate_seconds_per_event(port.seconds_per_event());
		BOOST_FOREACH(ports::output * const output_port, output_ports())
			output_port->propagate_seconds_per_event(port.seconds_per_event());
		if(multiple_input_port()) multiple_input_port()->propagate_seconds_per_event(port.seconds_per_event());
		return;
	}
	if(std::find(output_ports().begin(), output_ports().end(), &port) != output_ports().end()) {
		BOOST_FOREACH(ports::inputs::single * const input_port, single_input_ports())
			input_port->propagate_seconds_per_event(port.seconds_per_event());
		BOOST_FOREACH(ports::output * const output_port, output_ports())
			if(&port != output_port) output_port->propagate_seconds_per_event(port.seconds_per_event());
		if(multiple_input_port()) multiple_input_port()->propagate_seconds_per_event(port.seconds_per_event());
		return;
	}
	if(
		multiple_input_port() &&
		std::find(
			multiple_input_port()->output_ports().begin(), multiple_input_port()->output_ports().end(), &port
		) != multiple_input_port()->output_ports().end()
	) {
		BOOST_FOREACH(ports::inputs::single * const input_port, single_input_ports())
			input_port->propagate_seconds_per_event(port.seconds_per_event());
		BOOST_FOREACH(ports::output * const output_port, output_ports())
			output_port->propagate_seconds_per_event(port.seconds_per_event());
		BOOST_FOREACH(ports::output * const output_port, multiple_input_port()->output_ports())
			if(&port != output_port) output_port->propagate_seconds_per_event(port.seconds_per_event());
	}
#else
	for(single_input_ports_type::const_iterator
		i(single_input_ports().begin()), e(single_input_ports().end()); i != e; ++i
	) if(&port == *i) {
		for(single_input_ports_type::const_iterator
			j(single_input_ports().begin()), e(single_input_ports().end()); j != e; ++j
		) if(j != i) (**j).propagate_seconds_per_event(port.seconds_per_event());
		for(output_ports_type::const_iterator
			j(output_ports().begin()), e(output_ports().end()); j != e; ++j
		) (**j).propagate_seconds_per_event(port.seconds_per_event());
		if(multiple_input_port()) for(ports::inputs::multiple::output_ports_type::const_iterator
			j(multiple_input_port()->output_ports().begin()), e(multiple_input_port()->output_ports().end()); j != e; ++j
		) (**j).propagate_seconds_per_event(port.seconds_per_event());
		return;
	}
	for(output_ports_type::const_iterator
		i(output_ports().begin()), e(output_ports().end()); i != e; ++i
	) if(&port == *i) {
		for(single_input_ports_type::const_iterator
			j(single_input_ports().begin()), e(single_input_ports().end()); j != e; ++j
		) (**j).propagate_seconds_per_event(port.seconds_per_event());
		for(output_ports_type::const_iterator
			j(output_ports().begin()), e(output_ports().end()); j != e; ++j
		) if(j != i) (**j).propagate_seconds_per_event(port.seconds_per_event());
		if(multiple_input_port()) for(ports::inputs::multiple::output_ports_type::const_iterator
			j(multiple_input_port()->output_ports().begin()), e(multiple_input_port()->output_ports().end()); j != e; ++j
		) (**j).propagate_seconds_per_event(port.seconds_per_event());
		return;
	}
	if(multiple_input_port()) for(ports::inputs::multiple::output_ports_type::const_iterator
		i(multiple_input_port()->output_ports().begin()), e(multiple_input_port()->output_ports().end()); i != e; ++i
	) if(&port == *i) {
		for(single_input_ports_type::const_iterator
			j(single_input_ports().begin()), e(single_input_ports().end()); j != e; ++j
		) (**j).propagate_seconds_per_event(port.seconds_per_event());
		for(output_ports_type::const_iterator
			j(output_ports().begin()), e(output_ports().end()); j != e; ++j
		) (**j).propagate_seconds_per_event(port.seconds_per_event());
		for(ports::inputs::multiple::output_ports_type::const_iterator
			j(multiple_input_port()->output_ports().begin()), e(multiple_input_port()->output_ports().end()); j != e; ++j
		) if(j != i) (**j).propagate_seconds_per_event(port.seconds_per_event());
	}
#endif
}

void node::dump(std::ostream & out, std::size_t tabulations) const {
	for(std::size_t t(0) ; t < tabulations ; ++t) out << '\t';
	out
		<< name()
		<< " (" << universalis::compiler::typenameof(*this)
		<< ", lib " << plugin_library_reference().name()
		<< ")\n";
	if(multiple_input_port()) multiple_input_port()->dump(out, tabulations + 1);
	for(single_input_ports_type::const_iterator
		i(single_input_ports().begin()), e(single_input_ports().end()); i != e; ++i
	) (**i).dump(out, tabulations + 1);
	for(output_ports_type::const_iterator
		i(output_ports().begin()), e(output_ports().end()); i != e; ++i
	) (**i).dump(out, tabulations + 1);;
}

std::ostream & operator<<(std::ostream & out, const node & node) {
	node.dump(out);
	return out;
}

/**********************************************************************************************************************/
// port

port::port(class node & node, name_type const & name, std::size_t channels)
:
	bases::port(node),
	named(name),
	buffer_(0),
	seconds_per_event_(0)
{
	if(loggers::trace()()) {
		std::ostringstream s;
		s << this->qualified_name() << ": new engine port";
		loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
	if(!channels) {
		channels_ = 0;
		channels_immutable_ = false;
	} else {
		channels_immutable_ = false;
		this->do_channels(channels);
		channels_immutable_ = true;
		if(loggers::information()()) {
			std::ostringstream s;
			s << this->qualified_name() << ": port channels is immutable from now on";
			loggers::information()(s.str());
		}
	}
}

port::~port() {
	if(loggers::trace()()) {
		std::ostringstream s;
		s << this->qualified_name() << ": deleting engine port";
		loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
}

port::name_type port::qualified_name() const {
	return node().qualified_name() + '.' + name();
}

port::name_type port::semi_qualified_name() const {
	return node().name() + '.' + name();
}

void port::buffer(class buffer * const buffer) {
	if(false && loggers::trace()()) {
		std::ostringstream s;
		s << "assigning buffer " << buffer << " to port " << qualified_name();
		loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
	if(this->buffer_ == buffer) {
		if(false && loggers::trace()()) {
			std::ostringstream s;
			s << "already assigned the same buffer";
			loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
		return;
	}
	this->buffer_ = buffer;
}

void port::channels(std::size_t channels) throw(exception) {
	propagate_channels_to_node(channels);
	do_propagate_channels(); // polymorphic virtual call
}

void port::seconds_per_event(real const & seconds_per_event) {
	if(loggers::information()()) {
		std::ostringstream s;
		s << qualified_name() << ": port events per second changing to " << 1 / seconds_per_event;
		loggers::information()(s.str());
	}
	propagate_seconds_per_event_to_node(seconds_per_event);
	do_propagate_seconds_per_event(); // polymorphic virtual call
}

void port::propagate_channels(std::size_t channels) throw(exception) {
	if(this->channels() == channels) return;
	channels_transaction(channels);
	do_propagate_channels(); // polymorphic virtual call
}

void port::propagate_seconds_per_event(real const & seconds_per_event) {
	if(this->seconds_per_event() == seconds_per_event) return;
	this->seconds_per_event(seconds_per_event);
	do_propagate_seconds_per_event(); // polymorphic virtual call
}

void port::connect(port & port) throw(exception) {
	if(loggers::trace()()) {
		std::ostringstream s;
		s << this->qualified_name() << " port connecting to port " << port.qualified_name();
		loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
	// channels negociation
	{
		if(this->channels())
			if(port.channels())
				if(this->channels() != port.channels())
					throw exceptions::runtime_error("the channel counts of the two ports are not the same", UNIVERSALIS__COMPILER__LOCATION);
				else { /* nothing to do */ }
			else port.propagate_channels_to_node(this->channels());
		else if(port.channels()) this->propagate_channels_to_node(port.channels());
		else { /* nothing can be done for now */ }
	}
	// seconds per event negociation
	{
		if(this->seconds_per_event())
			if(port.seconds_per_event())
				if(this->seconds_per_event() != port.seconds_per_event())
					throw exceptions::runtime_error("the events per second of the two ports are not the same", UNIVERSALIS__COMPILER__LOCATION);
				else { /* nothing to do */ }
			else port.propagate_seconds_per_event_to_node(this->seconds_per_event());
		else if(port.seconds_per_event())
			this->propagate_seconds_per_event_to_node(port.seconds_per_event());
		else { /* nothing can be done for now */ }
	}
}

void port::propagate_channels_to_node(std::size_t channels) throw(exception) {
	channels_transaction(channels);
	node().channel_change_notification_from_port(*this);
}

void port::propagate_seconds_per_event_to_node(real const & seconds_per_event) {
	this->seconds_per_event_ = seconds_per_event;
	node().seconds_per_event_change_notification_from_port(*this);
}

void port::channels_transaction(std::size_t channels) throw(exception) {
	std::size_t const rollback(this->channels());
	try {
		this->do_channels(channels);
	} catch(...) {
		if(!channels_immutable()) this->channels(rollback);
		throw;
	}
}

void port::do_channels(std::size_t channels) throw(exception) {
	if(loggers::information()()) {
		std::ostringstream s;
		s << qualified_name() << ": port channels changing to " << channels;
		loggers::information()(s.str());
	}
	if(channels_immutable()) throw exceptions::runtime_error("channel count of port " + qualified_name() + " is immutable", UNIVERSALIS__COMPILER__LOCATION);
	this->channels_ = channels;
	if(buffer_ && buffer().channels() < channels) buffer().channels(channels);
}

void port::dump(std::ostream & out, /*const std::string & kind,*/ std::size_t tabulations) const {
	for(std::size_t t(0) ; t < tabulations; ++t) out << '\t';
	if     (typeid(*this) == typeid(ports::inputs::single  )) out << "in  ";
	else if(typeid(*this) == typeid(ports::inputs::multiple)) out << "in* ";
	else if(typeid(*this) == typeid(ports::output          )) out << "out ";
	else out << universalis::compiler::typenameof(*this) << ' ';
	out
		<< std::setw(16) << name()
		<< "\tchns: " << channels()
		<< "\tevts/s: " << std::setw(6) << events_per_second()
		<< "\tconns: ";
}

std::ostream & operator<<(std::ostream & out, port const & port) {
	port.dump(out);
	return out;
}

namespace ports {

	/**********************************************************************************************************************/
	// output

	output::output(class node & node, name_type const & name, std::size_t channels)
	:
		bases::ports::output(node, name, channels)
	{
		if(loggers::trace()()) {
			std::ostringstream s;
			s << qualified_name() << ": new engine port output";
			loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
	}

	output::~output() {
		if(loggers::trace()()) {
			std::ostringstream s;
			s << qualified_name() << ": deleting engine port output";
			loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
	}

	void output::do_propagate_channels() throw(exception) {
		for(input_ports_type::const_iterator i(input_ports().begin()) ; i != input_ports().end() ; ++i)
			(**i).propagate_channels_to_node(this->channels());
	}

	void output::do_propagate_seconds_per_event() {
		for(input_ports_type::const_iterator i(input_ports().begin()) ; i != input_ports().end() ; ++i)
			(**i).propagate_seconds_per_event(this->seconds_per_event());
	}

	void output::dump(std::ostream & out, std::size_t tabulations) const {
		port::dump(out, /*"out",*/ tabulations);
		for(input_ports_type::const_iterator i(input_ports().begin()) ; i != input_ports().end() ; ++i)
			out << ' ' << (**i).semi_qualified_name();
		out << '\n';
	}

	/**********************************************************************************************************************/
	// input
	
	input::input(class node & node, name_type const & name, std::size_t channels)
	:
		bases::ports::input(node, name, channels)
	{
		if(loggers::trace()()) {
			std::ostringstream s;
			s << qualified_name() << ": new engine port input";
			loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
	}

	input::~input() {
		if(loggers::trace()()) {
			std::ostringstream s;
			s << qualified_name() << ": deleting engine port input";
			loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
		//disconnect_all();
	}

	void input::connect(ports::output & output_port) throw(exception) {
		if(loggers::information()()) {
			std::ostringstream s;
			s << output_port.qualified_name() << " output port connecting to input port " << this->qualified_name();
			loggers::information()(s.str());
		}
		bases::ports::input::connect(output_port);
	}

	void input::disconnect(ports::output & output_port) {
		if(loggers::information()()) {
			std::ostringstream s;
			s << output_port.qualified_name() << " output port disconnecting from input port " << this->qualified_name();
			loggers::information()(s.str());
		}
		bases::ports::input::disconnect(output_port);
	}
	
	namespace inputs {
	
		/**********************************************************************************************************************/
		// single
		
		single::single(class node & node, name_type const & name, std::size_t channels)
		:
			bases::ports::inputs::single(node, name, channels)
		{
			if(loggers::trace()()) {
				std::ostringstream s;
				s << this->qualified_name() << ": new engine port input single";
				loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
		}

		single::~single() {
			if(loggers::trace()()) {
				std::ostringstream s;
				s << this->qualified_name() << ": deleting engine port input single";
				loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
		}

		void single::do_propagate_channels() throw(exception) {
			if(output_port()) output_port()->propagate_channels_to_node(this->channels());
		}

		void single::do_propagate_seconds_per_event() {
			if(output_port()) output_port()->propagate_seconds_per_event_to_node(this->seconds_per_event());
		}

		void single::dump(std::ostream & out, std::size_t tabulations) const {
			port::dump(out, /*"in",*/ tabulations);
			if(output_port()) out << ' ' << output_port()->semi_qualified_name();
			out << '\n';
		}

		/**********************************************************************************************************************/
		// multiple

		multiple::multiple(class node & node, name_type const & name, bool single_connection_is_identity_transform, std::size_t channels)
		:
			bases::ports::inputs::multiple(node, name, channels),
			single_connection_is_identity_transform_(single_connection_is_identity_transform)
		{
			if(loggers::trace()()) {
				std::ostringstream s;
				s << qualified_name() << ": new engine port input multiple";
				loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
		}

		multiple::~multiple() {
			if(loggers::trace()()) {
				std::ostringstream s;
				s << qualified_name() << ": deleting engine port input multiple";
				loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
		}

		void multiple::do_propagate_channels() throw(exception) {
			for(output_ports_type::const_iterator i(output_ports().begin()); i != output_ports().end() ; ++i)
				(**i).propagate_channels_to_node(this->channels());
		}

		void multiple::do_propagate_seconds_per_event() {
			for(output_ports_type::const_iterator i(output_ports().begin()); i != output_ports().end() ; ++i)
				(**i).propagate_seconds_per_event_to_node(this->seconds_per_event());
		}

		void multiple::dump(std::ostream & out, std::size_t tabulations) const {
			port::dump(out, /*"in",*/ tabulations);
			for(std::vector<output*>::const_iterator i = output_ports().begin() ; i != output_ports().end() ; ++i)
				out << ' ' << (**i).semi_qualified_name();
			out << '\n';
		}
	}
}
}}