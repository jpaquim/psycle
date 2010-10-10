// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2009 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>

///\interface psycle::front_ends::gui::root
#ifndef PSYCLE__FRONT_ENDS__GUI__ROOT__INCLUDED
#define PSYCLE__FRONT_ENDS__GUI__ROOT__INCLUDED
#pragma once
#include <psycle/engine/engine.hpp>
#include "graph.hpp"
#include <gtkmm/window.h>
#include <gtkmm/frame.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
namespace psycle { namespace front_ends { namespace gui {

class root : public Gtk::Window {
	public:
		root(underlying::graph &);
		virtual ~root();
	
	protected:
		host::plugin_resolver & resolver() { return resolver_; }
	private:
		host::plugin_resolver   resolver_;

	protected:
		virtual void on_button_clicked();
		Gtk::Button const & button() const { return button_; }
		Gtk::Button       & button()       { return button_; }
	private:
		Gtk::Button         button_;
	
	protected:
		Gtk::Label const & label() const { return label_; }
		Gtk::Label       & label()       { return label_; }
	private:
		Gtk::Label         label_;
	
	protected:
		graph const & graph_instance() const { return graph_; }
		graph       & graph_instance()       { return graph_; }
	private:
		graph       & graph_;
		
	private:
		bool playing_;
		Gtk::Frame sched_frame_, graph_frame_;
		Gtk::VBox v_box_;
		Gtk::HBox h_box_;
};
}}}
#endif
