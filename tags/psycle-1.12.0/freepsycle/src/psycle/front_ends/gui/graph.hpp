// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2011 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>

#pragma once
#include "forward_declarations.hpp"
#include "color.hpp"
#include "contraption.hpp"
#include <psycle/engine.hpp>
#include <psycle/host.hpp>
#include <psycle/generic/cast.hpp>
#include <boost/signal.hpp>
#include <gtkmm/adjustment.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/box.h>
#include <gtkmm/togglebutton.h>
#include <gtkmm/progressbar.h>
#include <gtkmm/actiongroup.h>
#include <gtkmm/uimanager.h>
#include <libgnomecanvasmm/canvas.h>
#include <libgnomecanvasmm/group.h>
#include <libgnomecanvasmm/rect.h>
#include <libgnomecanvasmm/line.h>
#include <libgnomecanvasmm/text.h>
#include <vector>
#define PSYCLE__DECL PSYCLE__FRONT_ENDS__GUI
#include <psycle/detail/decl.hpp>
namespace psycle { namespace front_ends { namespace gui {

typedef
	Gnome::Canvas::
		//Canvas
		CanvasAA
			canvas_base;

class PSYCLE__DECL canvas : public canvas_base {
	public:
		typedef canvas_base base;
		canvas(class graph &);

	protected:
		bool on_event(GdkEvent *) /*override*/;

	private:
		Gnome::Canvas::Group group_;

	public:
		class graph & graph() { return graph_; }
		operator class graph & () { return graph(); }
	private:
		class graph & graph_;
	
	///\name drawing of a new connection between ports
	///\{
		public:
			void selected_port(port & port);
		private:
			port * selected_port_, * selected_port_2_;
			bool selected_port_is_output_;

		protected:
			Gnome::Canvas::Line & line() { return line_; }
		private:
			Gnome::Canvas::Line line_;
	///\}
	
	///\name menu
	///\{
		private:
			Glib::RefPtr<Gtk::ActionGroup> action_group_;
			void add_node_type(std::string const & type);
			void on_new_node(std::string const & type);
			Glib::RefPtr<Gtk::UIManager> ui_manager_;
			Gtk::Menu * menu_;
			real x_, y_;
	///\}
		
	private:
		void set_scroll_region_from_bounds();
};

class PSYCLE__DECL graph
:
	public bases::graph,
	public Gtk::VBox
{
	public:
		typedef Gtk::VBox base;

	protected: friend class virtual_factory_access;
		graph(underlying_type &, host::plugin_resolver & resolver);
		void after_construction() /*override*/;
		virtual ~graph();

	public:
		host::plugin_resolver & resolver() { return resolver_; }
	private:
		host::plugin_resolver & resolver_;

	protected:
		host::schedulers::single_threaded::scheduler & scheduler() throw() { return scheduler_; }
	private:
		host::schedulers::single_threaded::scheduler scheduler_;

	protected:
		void on_start();
		Gtk::ToggleButton & start() throw() { return start_; }
	private:
		Gtk::ToggleButton start_;

	public:
		class canvas & canvas() throw() { return canvas_; }
		operator class canvas & () throw() { return canvas_; }
		operator Gnome::Canvas::Group & () throw() { return *canvas_.root(); }
		operator Gnome::Canvas::Group * () throw() { return canvas_.root(); }
	protected:
		void on_size_allocate(Gtk::Allocation & allocation) /*override*/;
		void on_zoom();
	private:
		class canvas canvas_;
		Gtk::ScrolledWindow scroll_;
		Gtk::Adjustment adjustment_;
		Gtk::SpinButton spin_;

	private:
		Gtk::VBox box_;
};

class PSYCLE__DECL port
:
	public bases::port,
	public Gnome::Canvas::Group
{
	protected: friend class virtual_factory_access;
		port(class node &, underlying_type &, real const & x, real const & y, color const &);
		virtual ~port() {}

	protected: friend class node;
		class contraption & contraption() throw() { return contraption_; }
	private:
		class contraption contraption_;
	
	protected:
		Gnome::Canvas::Line & line() throw() { return line_; }
	private:
		Gnome::Canvas::Line line_;

	protected:
		void on_select(class contraption &);
		void on_move(class contraption &);
	private:
		bool on_canvas_event(GdkEvent *);
};

namespace ports {
	class PSYCLE__DECL output : public bases::ports::output {
		protected: friend class virtual_factory_access;
			output(class node &, underlying_type &, real const & x = 0, real const & y = 0);
		
		protected:
			std::vector<Gnome::Canvas::Line*> & lines() throw() { return lines_; }
		private:
			std::vector<Gnome::Canvas::Line*> lines_;
	};
	class PSYCLE__DECL input : public bases::ports::input {
		protected: friend class virtual_factory_access;
			input(class node &, underlying_type &, real const & x, real const & y, color const &);
	};
	namespace inputs {
		class PSYCLE__DECL single : public bases::ports::inputs::single {
			protected: friend class virtual_factory_access;
				single(class node &, underlying_type &, real const & x = 0, real const & y = 0);
		};
		class PSYCLE__DECL multiple : public bases::ports::inputs::multiple {
			protected: friend class virtual_factory_access;
				multiple(class node &, underlying_type &, real const & x = 0, real const & y = 0);
		};
	}
}

class PSYCLE__DECL node
:
	public bases::node,
	public Gnome::Canvas::Group
{
	protected: friend class virtual_factory_access;
		//node(parent_type & parent, underlying_type & underlying);
		node(class graph &, underlying_type &, real const & x = 0, real const & y = 0);
		void after_construction() /*override*/;
		
	public: // protected:
		~node() throw() {}

	protected:
		class contraption & contraption() throw() { return contraption_; }
	private:
		class contraption contraption_;
	
	private:
		bool on_canvas_event(GdkEvent *);
		
	///\name menu
	///\{
		private:
			Gtk::Menu * menu_;
			void on_delete();
	///\}
};

}}}
#include <psycle/detail/decl.hpp>
