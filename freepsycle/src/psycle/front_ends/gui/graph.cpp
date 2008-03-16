// -*- mode:c++; indent-tabs-mode:t -*-
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2008 psycledelics http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>
///\implementation psycle::front_ends::gui::graph
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <psycle/detail/project.private.hpp>
#include "graph.hpp"
#include "lock.hpp"
#include <boost/bind.hpp>
#include <gdkmm/color.h>
#include <gdkmm/colormap.h>
#include <gtkmm/stock.h>
namespace psycle { namespace front_ends { namespace gui {

/**********************************************************************************************************************/
// graph

graph::graph(underlying_type & underlying, host::plugin_resolver & resolver)
:
	graph_type(underlying),
	resolver_(resolver),
	scheduler_(*this),
	start_("play"),
	canvas_(*this),
	adjustment_(1.00, 0.05, 5.00, 0.001, 0.1, 0.1),
	spin_(adjustment_, 1.0, 2)
{
std::clog << "new gui graph\n";
	//pack_start(start(), Gtk::PACK_SHRINK);
	//pack_start(spin_, Gtk::PACK_SHRINK);
	scroll_.add(canvas_instance());
	pack_start(scroll_);
	show_all_children();
	start().signal_toggled().connect(sigc::mem_fun(*this, &graph::on_start));
	adjustment_.signal_value_changed().connect(sigc::mem_fun(*this, &graph::on_zoom));
}

void graph::after_construction() {
	graph_base::after_construction();
	resolver()("output", *this, "output");
}

graph::~graph() {}

void graph::on_start() {
	scheduler().started(start().get_active());
}

void graph::on_size_allocate(Gtk::Allocation & allocation) {
	base::on_size_allocate(allocation);
}

void graph::on_zoom() {
	canvas_instance().set_pixels_per_unit(adjustment_.get_value());
}

/**********************************************************************************************************************/
// node

node::node(node::parent_type & parent, node::underlying_type & underlying, real const & x, real const & y)
:
	node_type(parent, underlying),
	Gnome::Canvas::Group(parent, x, y),
	contraption_(*this, 0, 0, 0x60606000, underlying.name() + "\n" + underlying.plugin_library_reference().name())
{
std::clog << "new gui node\n";
	menu_ = new Gtk::Menu;
	Gtk::MenuItem * item;
	item = new Gtk::MenuItem("Delete");
	item->signal_activate().connect(sigc::mem_fun(*this, &node::on_delete));
	menu_->append(*item); // menu_->items().push_back(*item);
	item->show();
	/*contraption_.*/signal_event().connect(sigc::mem_fun(*this, &node::on_event_));
}

void node::after_construction() {
	node_type::after_construction();
	int const ports(single_input_ports().size() + output_ports().size() + (multiple_input_port() ? 1 : 0));
	real const angle_step(engine::math::pi * 2 / ports);
	real const radius(60);
	real angle(0);
	for(output_ports_type::const_iterator i(output_ports().begin()) ; i != output_ports().end() ; ++i) {
		typenames::ports::output & output_port(**i);
		Gtk::manage(&output_port);
		real const x(radius * std::cos(angle)), y(radius * std::sin(angle));
		output_port.contraption_instance().property_parent().get_value()->move(x, y);
		output_port.contraption_instance().signal_move()(output_port.contraption_instance());
		angle += angle_step;
	}
	if(multiple_input_port()) {
		typenames::ports::inputs::multiple & multiple_input_port(*this->multiple_input_port());
		Gtk::manage(&multiple_input_port);
		real const x(radius * std::cos(angle)), y(radius * std::sin(angle));
		multiple_input_port.contraption_instance().property_parent().get_value()->move(x, y);
		multiple_input_port.contraption_instance().signal_move()(multiple_input_port.contraption_instance());
		angle += angle_step;
	}
	for(single_input_ports_type::const_iterator i(single_input_ports().begin()) ; i != single_input_ports().end() ; ++i) {
		typenames::ports::inputs::single & single_input_port(**i);
		Gtk::manage(&single_input_port);
		real const x(radius * std::cos(angle)), y(radius * std::sin(angle));
		single_input_port.contraption_instance().property_parent().get_value()->move(x, y);
		single_input_port.contraption_instance().signal_move()(single_input_port.contraption_instance());
		angle += angle_step;
	}
	contraption_instance().raise_to_top();
}

void node::on_delete() {
std::clog << "node::on_delete\n";
	underlying().free_heap();
}

bool node::on_event_(GdkEvent * event) {
std::clog << "node::on_event\n";
	switch(event->type) {
		case GDK_ENTER_NOTIFY: {
		}
		break;
		case GDK_KEY_PRESS: {
		}
		break;
		case GDK_BUTTON_PRESS: {
			switch(event->button.button) {
				case 1: {
				}
				break;
				case 2: {
std::clog << "node::on_event: bp2\n";
					menu_->popup(event->button.button, event->button.time);
					return true;
				}
				break;
				case 3: {
				}
				break;
				default: ;
			}
		}
		break;
		case GDK_MOTION_NOTIFY: {
		}
		break;
		case GDK_BUTTON_RELEASE: {
			switch(event->button.button) {
				default: ;
			}
		}
		break;
		case GDK_KEY_RELEASE: {
		}
		break;
		case GDK_LEAVE_NOTIFY: {
		}
		break;
		default: ;
	}
	//return Gnome::Canvas::Group::on_event(event);
std::clog << "node::on_event: false\n";
	return false;
}

/**********************************************************************************************************************/
//port

port::port(port::parent_type & parent, port::underlying_type & underlying, real const & x, real const & y, color const & color)
:
	port_type(parent, underlying),
	Gnome::Canvas::Group(parent, x, y),
	contraption_(*this, 0, 0, color, underlying.name()),
	line_(contraption_instance())
{
	line().property_fill_color_rgba() = contraption_instance().fill_color();
	line().property_width_pixels() = 3;
	line().property_last_arrowhead() = false;
	line().lower_to_bottom();
	contraption_instance().signal_select().connect(boost::bind(&port::on_select, this, _1));
	contraption_instance().signal_move()  .connect(boost::bind(&port::on_move  , this, _1));
	on_move(contraption_instance());
}

void port::on_select(contraption & contraption) {
	if(!parent().parent().canvas_instance().selected_port())
		parent().parent().canvas_instance().selected_port(*this);
}

void port::on_move(contraption & contraption) {
	Gnome::Canvas::Points points(2);
	points[0] = Gnome::Art::Point(0, 0);
	real node_x(parent().property_x()), node_y(parent().property_y());
	w2i(node_x, node_y);
	points[1] = Gnome::Art::Point(node_x, node_y);
	line().property_points() = points;
}

bool port::on_event(GdkEvent * event) {
	if(Gnome::Canvas::Group::on_event(event)) return true;
	#if 0
	real x(event->button.x), y(event->button.y);
	switch(event->type) {
		case GDK_ENTER_NOTIFY: {
		}
		break;
		case GDK_KEY_PRESS: {
		}
		break;
		case GDK_BUTTON_PRESS: {
			switch(event->button.button) {
			}
		}
		break;
		case GDK_MOTION_NOTIFY: {
		}
		break;
		case GDK_BUTTON_RELEASE: {
			switch(event->button.button) {
			}
		}
		break;
		case GDK_KEY_RELEASE: {
		}
		break;
		case GDK_LEAVE_NOTIFY: {
		}
		break;
		default: ;
	}
	#endif
	return false;
}

namespace ports {

	/**********************************************************************************************************************/
	// output
	
	output::output(output::parent_type & parent, output::underlying_type & underlying, real const & x, real const & y)
	:
		output_type(parent, underlying, x, y, boost::cref(color(0xa0600000)))
	{}

	input::input(input::parent_type & parent, input::underlying_type & underlying, real const & x, real const & y, color const & color)
	:
		input_type(parent, underlying, x, y, color)
	{}
	
	namespace inputs {
	
		/**********************************************************************************************************************/
		// single
		
		single::single(single::parent_type & parent, single::underlying_type & underlying, real const & x, real const & y)
		:
			single_type(parent, underlying, x, y, boost::cref(color(0x00a00000)))
		{}

		/**********************************************************************************************************************/
		// multiple
		
		multiple::multiple(multiple::parent_type & parent, multiple::underlying_type & underlying, real const & x, real const & y)
		:
			multiple_type(parent, underlying, x, y, boost::cref(color(0x0060a000)))
		{}
	}
}

/**********************************************************************************************************************/
// canvas

canvas::canvas(graph & graph)
:
	group_(*root()),
	graph_(graph),
	selected_port_(),
	line_(*root()),
	x_(), y_()
{
	//set_center_scroll_region(false);
	set_scroll_region(-200, -200, +200, +200);
	
	line().lower_to_bottom();
	line().property_fill_color_rgba() = 0x0000ffa0;
	line().property_last_arrowhead() = true;
	line().property_width_pixels() = 2;
	line().property_arrow_shape_a() = 20;
	line().property_arrow_shape_b() = 25;
	line().property_arrow_shape_c() = 8;
	
	action_group_ = Gtk::ActionGroup::create();
	add_node_type("sequence");
	add_node_type("sine");
	add_node_type("decay");
	add_node_type("additioner");
	add_node_type("multiplier");
	add_node_type("output");

	ui_manager_ = Gtk::UIManager::create();
	ui_manager_->set_add_tearoffs(true);
	ui_manager_->insert_action_group(action_group_);
	//xxx.add_accel_group(ui_manager_->get_accel_group());
	{
		std::ostringstream s;
		s << "<ui><popup name='popup-menu'>";
		{
			typedef std::list<Glib::RefPtr<Gtk::Action const> > action_list;
			action_list const & actions(action_group_->get_actions());
			for(action_list::const_iterator i(actions.begin()), e(actions.end()); i != e; ++i) {
				if(loggers::information()) {
					std::ostringstream s; s << "adding ui action: " << (*i)->get_name();
					loggers::information()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
				}
				s << "<menuitem action='" << (*i)->get_name() << "'/>";
			}
		}
		s << "</popup></ui>";
		ui_manager_->add_ui_from_string(s.str());
	}
	//ui_manager_->add_ui(ui_manager_->new_merge_id(), "/popup-menu", "new-" + type + "-node", "new-" + type + "-node");
	menu_ = dynamic_cast<Gtk::Menu*>(ui_manager_->get_widget("/popup-menu"));
}

void canvas::add_node_type(std::string const & type) {
	action_group_->add(
		Gtk::Action::create("new-" + type + "-node", Gtk::Stock::NEW, "New " + type, "Create a new " + type + " node"),
		sigc::bind(sigc::mem_fun(*this, &canvas::on_new_node), type)
	);
}

void canvas::on_new_node(std::string const & type) {
	static int id(0);
	std::ostringstream name; name << type << id++;
	node::underlying_type * underlying(0);
	try {
		underlying = &static_cast<node::underlying_type&>(graph_instance().resolver()(type, graph_instance(), name.str()));
	} catch(std::exception const & e) {
		loggers::exception()(e.what(), UNIVERSALIS__COMPILER__LOCATION);
	}
	if(underlying) {
		node & underlying_wrapper(graph_instance().underlying_wrapper(*underlying));
		Gtk::manage(&underlying_wrapper);
		underlying_wrapper.property_x() = x_;
		underlying_wrapper.property_y() = y_;
		//set_scroll_region_from_bounds();
	}
}

void canvas::set_scroll_region_from_bounds() {
	real bounds_x_min, bounds_y_min, bounds_x_max, bounds_y_max;
	root()->get_bounds(bounds_x_min, bounds_y_min, bounds_x_max, bounds_y_max);
	real scroll_x_min, scroll_y_min, scroll_x_max, scroll_y_max;
	//get_scroll_region(scroll_x_min, scroll_y_min, scroll_x_max, scroll_y_max);
	//set_scroll_region(std::min(bounds_x_min, scroll_x_min), std::min(bounds_y_min, scroll_y_min), std::max(bounds_x_max, scroll_x_max), std::max(bounds_y_max, scroll_y_max));
	set_scroll_region(bounds_x_min, bounds_y_min, bounds_x_max, bounds_y_max);
}

bool canvas::on_event(GdkEvent * event) {
	if(base::on_event(event)) return true;
	real x(event->button.x), y(event->button.y);
	switch(event->type) {
		case GDK_ENTER_NOTIFY: {
		}
		break;
		#if 0
		case GDK_KEY_PRESS: {
			if(event->key.state & GDK_SHIFT_MASK) {
				//dragging_ = true;
				//grab(GDK_KEY_RELEASE_MASK | GDK_POINTER_MOTION_MASK | GDK_BUTTON_RELEASE_MASK, Gdk::Cursor(Gdk::TCROSS), event->key.time);
			}
		}
		break;
		#endif
		case GDK_BUTTON_PRESS: {
			switch(event->button.button) {
				case 1: {
				}
				break;
				case 2: {
					set_scroll_region_from_bounds();
					//return true;
				}
				break;
				case 3: {
					window_to_world(x, y, x_, y_);
					menu_->popup(event->button.button, event->button.time);
					return true;
				}
				break;
				default: ;
			}
		}
		break;
		case GDK_MOTION_NOTIFY: {
			if(event->motion.state & GDK_BUTTON1_MASK) {
			}
			else if(event->motion.state & GDK_SHIFT_MASK) {
				if(selected_port()) {
					window_to_world(x, y, x, y);
					real port_x(selected_port()->property_x()), port_y(selected_port()->property_y());
					selected_port()->property_parent().get_value()->i2w(port_x, port_y);
					Gnome::Canvas::Points points(2);
					points[0] = Gnome::Art::Point(port_x, port_y);
					points[1] = Gnome::Art::Point(x, y);
					line().property_points() = points;
					line().show();
					//return true;
				}
			}
		}
		break;
		case GDK_BUTTON_RELEASE: {
			switch(event->button.button) {
			}
		}
		break;
		case GDK_KEY_RELEASE: {
			//if(event->key.state & GDK_CONTROL_MASK)
			{
				selected_port_ = 0;
				line().hide();
			}
			#if 0
			if(event->key.state & GDK_SHIFT_MASK) {
				dragging_ = false;
				ungrab(event->key.time);
			}
			if(other) {
				ports::output * output_port(dynamic_cast<ports::output*>(this));
				ports::input * input_port(dynamic_cast<ports::input*>(other));
				if(!(input_port && output_port)) {
					output_port = dynamic_cast<ports::output*>(this);
					input_port = dynamic_cast<ports::input*>(other);
				}
				if(input_port && output_port) {
					static_cast<engine::ports::input&>(other->underlying_instance())
						.connect(
							static_cast<engine::ports::output&>(this->underlying_instance())
						);
				}
				other = 0;
			}
			#endif
		}
		break;
		case GDK_LEAVE_NOTIFY: {
		}
		break;
		default: ;
	}
	return false;
}
}}}
