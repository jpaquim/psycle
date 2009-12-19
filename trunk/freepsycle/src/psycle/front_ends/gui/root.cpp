// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2009 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>

///\implementation psycle::front_ends::gui::root
#include <psycle/detail/project.private.hpp>
#include "root.hpp"
#include <psycle/paths.hpp>
#include <gdkmm/screen.h>
namespace psycle { namespace front_ends { namespace gui {

root::root(underlying::graph & g)
:
	playing_(),
	button_("play"),
	label_("stopped"),
	graph_(gui::graph::create_on_heap(g, resolver())),
	sched_frame_("scheduler"),
	graph_frame_(g.name())
{
	set_size_request(1000, 700);
	#if !defined DIVERSALIS__COMPILER__FEATURE__NOT_CONCRETE // parsing problems
		set_title(paths::package::name() + " " + paths::package::version::string());
	#endif
	set_border_width(4);

	button().signal_clicked().connect(sigc::mem_fun(*this, &root::on_button_clicked));
	h_box_.pack_start(button(), Gtk::PACK_SHRINK);
	h_box_.pack_start(label(), Gtk::PACK_EXPAND_PADDING, 4);
	sched_frame_.add(h_box_);
	v_box_.pack_start(sched_frame_, Gtk::PACK_SHRINK);

	graph_frame_.add(graph_instance());
	v_box_.pack_start(graph_frame_);
	add(v_box_);

	show_all_children();
	//set_resize_mode(Gtk::RESIZE_IMMEDIATE);
}

root::~root() {
 	graph_.free_heap();
}

void root::on_button_clicked() {
	playing_ = !playing_;
	label().set_text(playing_ ? "playing" : "stopped");
}

}}}
