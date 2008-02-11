/* -*- mode:c++, indent-tabs-mode:t -*- */
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

///\\implementation psycle::plugins::bipolar_filter
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <psycle/detail/project.private.hpp>
#include "bipolar_filter.hpp"
namespace psycle { namespace plugins {

bipolar_filter::bipolar_filter(engine::plugin_library_reference & plugin_library_reference, engine::graph & graph, std::string const & name, real const & logical_zero)
:
	node(plugin_library_reference, graph, name),
	logical_zero_(logical_zero)
{
	engine::ports::inputs::multiple::create_on_heap(*this, "in", /*single_connection_is_identity_transform*/ boost::cref(true));
	engine::ports::output::create_on_heap(*this, "out");
}

void bipolar_filter::channel_change_notification_from_port(const engine::port & port) throw(engine::exception) {
	if(&port == output_ports()[0]) multiple_input_port()->propagate_channels(port.channels());
	else if(&port == multiple_input_port()) output_ports()[0]->propagate_channels(port.channels());
	assert(multiple_input_port()->channels() == output_ports()[0]->channels());
}

void bipolar_filter::seconds_per_event_change_notification_from_port(const engine::port & port) {
	if(&port == output_ports()[0]) multiple_input_port()->propagate_seconds_per_event(port.seconds_per_event());
	else if(&port == multiple_input_port()) output_ports()[0]->propagate_seconds_per_event(port.seconds_per_event());
	assert(engine::math::roughly_equals(multiple_input_port()->seconds_per_event(), output_ports()[0]->seconds_per_event()));
}

void bipolar_filter::do_process_first() throw(engine::exception) {
	if(!multiple_input_port()->output_ports().size()) return;
	if(!output_ports()[0]->input_ports().size()) return;
	assert(&multiple_input_port()->buffer());
	assert(&output_ports()[0]->buffer());
	engine::buffer & in(multiple_input_port()->buffer());
	engine::buffer & out(output_ports()[0]->buffer());
	assert(out.channels() == in.channels());
	for(std::size_t channel(0); channel < in.channels(); ++channel)
		for(std::size_t event(0); event < in.events() && in[channel][event].index() < in.events(); ++event) {
			// not needed because in and out have the same buffer: out[channel][event].index(event);
			out[channel][event].sample() = logical_zero_;
		}
	do_process();
}

void bipolar_filter::do_process() throw(engine::exception) {
	if(!*output_ports()[0]) return;
	if(!*multiple_input_port()) return;
	engine::buffer & in(multiple_input_port()->buffer());
	engine::buffer & out(output_ports()[0]->buffer());
	for(std::size_t channel(0) ; channel < in.channels() ; ++channel)
		out[channel].flag(in[channel].flag());
}

}}

