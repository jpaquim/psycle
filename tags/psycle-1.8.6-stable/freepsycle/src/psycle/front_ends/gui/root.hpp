// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org : johan boule

///\interface psycle::front_ends::gui::root
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
		host::plugin_resolver & resolver() throw() { return resolver_; }
	private:
		host::plugin_resolver   resolver_;

	protected:
		void virtual on_button_clicked();
		Gtk::Button const & button() const throw() { return button_; }
		Gtk::Button       & button()       throw() { return button_; }
	private:
		Gtk::Button         button_;
	
	protected:
		Gtk::Label const & label() const throw() { return label_; }
		Gtk::Label       & label()       throw() { return label_; }
	private:
		Gtk::Label         label_;
	
	protected:
		graph const & graph_instance() const throw() { return graph_; }
		graph       & graph_instance()       throw() { return graph_; }
	private:
		graph       & graph_;
		
	private:
		Gtk::Frame hello_frame_, graph_frame_;
		Gtk::VBox v_box_;
		Gtk::HBox h_box_;
};
}}}

