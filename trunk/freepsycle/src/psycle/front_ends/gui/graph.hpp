// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2009 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>

///\interface psycle::front_ends::gui::graph
#pragma once
#include "forward_declarations.hpp"
#include "color.hpp"
#include "contraption.hpp"
#include <psycle/engine.hpp>
#include <psycle/host.hpp>
#include <universalis/compiler/cast.hpp>
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
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK PSYCLE__FRONT_ENDS__GUI
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace psycle { namespace front_ends { namespace gui {

typedef
	Gnome::Canvas::
		//Canvas
		CanvasAA
			canvas_base;

class UNIVERSALIS__COMPILER__DYNAMIC_LINK canvas : public canvas_base {
	public:
		typedef canvas_base base;
		canvas(typenames::graph &);

	protected:
		bool on_event(GdkEvent *) /*override*/;

	private:
		Gnome::Canvas::Group group_;
		
	public:
		typenames::graph inline & graph() { return graph_; }
		inline operator typenames::graph & () { return graph(); }
	private:
		typenames::graph & graph_;
	
	///\name drawing of a new connection between ports
	///\{
		public:
			void inline selected_port(port & port);
		private:
			port * selected_port_, * selected_port_2_;
			bool selected_port_is_output_;

		protected:
			Gnome::Canvas::Line inline & line() { return line_; }
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

class UNIVERSALIS__COMPILER__DYNAMIC_LINK graph
:
	public typenames::typenames::bases::graph,
	public Gtk::VBox
{
	public:
		typedef Gtk::VBox base;

	protected: friend class virtual_factory_access;
		graph(underlying_type &, host::plugin_resolver & resolver);
		void after_construction() /*override*/;
		virtual ~graph();

	public:
		host::plugin_resolver inline & resolver() { return resolver_; }
	private:
		host::plugin_resolver & resolver_;

	protected:
		host::schedulers::single_threaded::scheduler inline & scheduler() throw() { return scheduler_; }
	private:
		host::schedulers::single_threaded::scheduler scheduler_;

	protected:
		void on_start();
		Gtk::ToggleButton inline & start() throw() { return start_; }
	private:
		Gtk::ToggleButton start_;

	public:
		typenames::canvas inline & canvas() throw() { return canvas_; }
		inline operator typenames::canvas & () throw() { return canvas(); }
		inline operator Gnome::Canvas::Group & () throw() { return *canvas().root(); }
		inline operator Gnome::Canvas::Group * () throw() { return canvas().root(); }
	protected:
		void on_size_allocate(Gtk::Allocation & allocation) /*override*/;
		void on_zoom();
	private:
		typenames::canvas canvas_;
		Gtk::ScrolledWindow scroll_;
		Gtk::Adjustment adjustment_;
		Gtk::SpinButton spin_;

	private:
		Gtk::VBox box_;
};

class UNIVERSALIS__COMPILER__DYNAMIC_LINK port
:
	public typenames::typenames::bases::port,
	public Gnome::Canvas::Group
{
	protected: friend class virtual_factory_access;
		port(parent_type &, underlying_type &, real const & x, real const & y, color const &);
		virtual inline ~port() {}

	protected: friend class node;
		typenames::contraption inline & contraption() throw() { return contraption_; }
	private:
		typenames::contraption contraption_;
	
	protected:
		Gnome::Canvas::Line inline & line() throw() { return line_; }
	private:
		Gnome::Canvas::Line line_;

	protected:
		void on_select(typenames::contraption &);
		void on_move(typenames::contraption &);
	private:
		bool on_canvas_event(GdkEvent *);
};

namespace ports {
	class UNIVERSALIS__COMPILER__DYNAMIC_LINK output : public typenames::typenames::bases::ports::output {
		protected: friend class virtual_factory_access;
			output(parent_type &, underlying_type &, real const & x = 0, real const & y = 0);
		
		protected:
			std::vector<Gnome::Canvas::Line*> inline & lines() throw() { return lines_; }
		private:
			std::vector<Gnome::Canvas::Line*> lines_;
	};
	class UNIVERSALIS__COMPILER__DYNAMIC_LINK input : public typenames::typenames::bases::ports::input {
		protected: friend class virtual_factory_access;
			input(parent_type &, underlying_type &, real const & x, real const & y, color const &);
	};
	namespace inputs {
		class UNIVERSALIS__COMPILER__DYNAMIC_LINK single : public typenames::typenames::bases::ports::inputs::single {
			protected: friend class virtual_factory_access;
				single(parent_type &, underlying_type &, real const & x = 0, real const & y = 0);
		};
		class UNIVERSALIS__COMPILER__DYNAMIC_LINK multiple : public typenames::typenames::bases::ports::inputs::multiple {
			protected: friend class virtual_factory_access;
				multiple(parent_type &, underlying_type &, real const & x = 0, real const & y = 0);
		};
	}
}

class UNIVERSALIS__COMPILER__DYNAMIC_LINK node
:
	public typenames::typenames::bases::node,
	public Gnome::Canvas::Group
{
	protected: friend class virtual_factory_access;
		//node(parent_type & parent, underlying_type & underlying);
		node(parent_type &, underlying_type &, real const & x = 0, real const & y = 0);
		void after_construction() /*override*/;
		
	public: // protected:
		~node() throw() {}

	protected:
		typenames::contraption inline & contraption() throw() { return contraption_; }
	private:
		typenames::contraption contraption_;
	
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
#include <universalis/compiler/dynamic_link/end.hpp>
