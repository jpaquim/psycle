// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2009 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>

///\implementation psycle::front_ends::gui::graph
#include <psycle/detail/project.private.hpp>
#include "graph.hpp"
#include "lock.hpp"
#include <boost/bind.hpp>
#include <gdkmm/color.h>
#include <gdkmm/colormap.h>
#include <gtkmm/stock.h>
#include <gdk/gdkkeysyms.h>
namespace psycle { namespace front_ends { namespace gui {

/**********************************************************************************************************************/
// graph

graph::graph(underlying_type & underlying, host::plugin_resolver & resolver)
:
	bases::graph(underlying),
	resolver_(resolver),
	scheduler_(*this),
	start_("play"),
	canvas_(*this),
	adjustment_(1.00, 0.05, 5.00, 0.001, 0.1, 0.1),
	spin_(adjustment_, 1.0, 2)
{
	loggers::trace()("new gui graph");
	pack_start(start(), Gtk::PACK_SHRINK);
	pack_start(spin_, Gtk::PACK_SHRINK);
	scroll_.add(canvas());
	pack_start(scroll_);
	show_all_children();
	start().signal_toggled().connect(sigc::mem_fun(*this, &graph::on_start));
	adjustment_.signal_value_changed().connect(sigc::mem_fun(*this, &graph::on_zoom));
}

void graph::after_construction() {
	bases::graph::after_construction();
	resolver()("output", *this, "output");
}

graph::~graph() {}

void graph::on_start() {
	loggers::trace()("graph::on_start");
	scheduler().started(start().get_active());
}

void graph::on_size_allocate(Gtk::Allocation & allocation) {
	base::on_size_allocate(allocation);
}

void graph::on_zoom() {
	loggers::trace()("graph::on_zoom");
	canvas().set_pixels_per_unit(adjustment_.get_value());
}

/**********************************************************************************************************************/
// node

node::node(class graph & graph, node::underlying_type & underlying, real const & x, real const & y)
:
	bases::node(graph, underlying),
	Gnome::Canvas::Group(graph, x, y),
	contraption_(*this, 0, 0, 0x60606000, underlying.name() + "\n" + underlying.plugin_library_reference().name())
{
	loggers::trace()("node::new (gui)");
	menu_ = new Gtk::Menu;
	Gtk::MenuItem * item;
	item = new Gtk::MenuItem("Delete");
	item->signal_activate().connect(sigc::mem_fun(*this, &node::on_delete));
	menu_->append(*item); // menu_->items().push_back(*item);
	item->show();
	/*contraption_.*/signal_event().connect(sigc::mem_fun(*this, &node::on_canvas_event));
}

void node::after_construction() {
	loggers::trace()("node::after_construction (gui)");
	bases::node::after_construction();
	int const ports(single_input_ports().size() + output_ports().size() + (multiple_input_port() ? 1 : 0));
	real const angle_step(engine::math::pi * 2 / ports);
	real const radius(60);
	real angle(0);
	for(output_ports_type::const_iterator i(output_ports().begin()), e(output_ports().end()); i != e; ++i) {
		ports::output & output_port(**i);
		Gtk::manage(&output_port);
		real const x(radius * std::cos(angle)), y(radius * std::sin(angle));
		output_port.contraption().property_parent().get_value()->move(x, y);
		output_port.contraption().signal_move()(output_port.contraption());
		angle += angle_step;
	}
	if(multiple_input_port()) {
		ports::inputs::multiple & multiple_input_port(*this->multiple_input_port());
		Gtk::manage(&multiple_input_port);
		real const x(radius * std::cos(angle)), y(radius * std::sin(angle));
		multiple_input_port.contraption().property_parent().get_value()->move(x, y);
		multiple_input_port.contraption().signal_move()(multiple_input_port.contraption());
		angle += angle_step;
	}
	for(single_input_ports_type::const_iterator i(single_input_ports().begin()), e(single_input_ports().end()); i != e; ++i) {
		ports::inputs::single & single_input_port(**i);
		Gtk::manage(&single_input_port);
		real const x(radius * std::cos(angle)), y(radius * std::sin(angle));
		single_input_port.contraption().property_parent().get_value()->move(x, y);
		single_input_port.contraption().signal_move()(single_input_port.contraption());
		angle += angle_step;
	}
	contraption().raise_to_top();
}

void node::on_delete() {
	loggers::trace()("node::on_delete (gui)");
	underlying().free_heap();
}

bool node::on_canvas_event(GdkEvent * event) {
	switch(event->type) {
		case GDK_ENTER_NOTIFY: {
		}
		break;
		case GDK_LEAVE_NOTIFY: {
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
					loggers::trace()("node::on_canvas_event: menu");
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
		default: ;
	}
	//loggers::trace()("node::on_canvas_event: false");
	return false;
}

/**********************************************************************************************************************/
//port

port::port(class node & node, port::underlying_type & underlying, real const & x, real const & y, color const & color)
:
	bases::port(node, underlying),
	Gnome::Canvas::Group(node, x, y),
	contraption_(*this, 0, 0, color, underlying.name()),
	line_(contraption())
{
	line().property_fill_color_rgba() = contraption().fill_color();
	line().property_width_pixels() = 3;
	line().property_last_arrowhead() = false;
	line().lower_to_bottom();
	/*contraption_.*/signal_event().connect(sigc::mem_fun(*this, &port::on_canvas_event));
	contraption().signal_select().connect(boost::bind(&port::on_select, this, _1));
	contraption().signal_move()  .connect(boost::bind(&port::on_move  , this, _1));
	on_move(contraption());
}

void port::on_select(class contraption & contraption) {
	node().graph().canvas().selected_port(*this);
}

void port::on_move(class contraption & contraption) {
	Gnome::Canvas::Points points(2);
	points[0] = Gnome::Art::Point(0, 0);
	real node_x(node().property_x()), node_y(node().property_y());
	w2i(node_x, node_y);
	points[1] = Gnome::Art::Point(node_x, node_y);
	line().property_points() = points;
}

bool port::on_canvas_event(GdkEvent * event) {
	real x(event->button.x), y(event->button.y);
	switch(event->type) {
		case GDK_ENTER_NOTIFY: {
		}
		break;
		case GDK_LEAVE_NOTIFY: {
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
			}
		}
		break;
		case GDK_KEY_RELEASE: {
		}
		break;
		default: ;
	}
	//loggers::trace()("port::on_canvas_event: false");
	return false;
}

namespace ports {

	/**********************************************************************************************************************/
	// output
	
	output::output(class node & node, output::underlying_type & underlying, real const & x, real const & y)
	:
		bases::ports::output(node, underlying, x, y, boost::cref(color(0xa0600000)))
	{
		Gnome::Canvas::Line & line = *new Gnome::Canvas::Line(contraption());
		Gtk::manage(&line);
		line.property_fill_color_rgba() = color(0xa060a0b0);
		line.property_width_pixels() = 3;
		line.property_last_arrowhead() = false;
		line.lower_to_bottom();

		Gnome::Canvas::Points points(2);
		points[0] = Gnome::Art::Point(0, 0);
		points[1] = Gnome::Art::Point(100, 0);
		line.property_points() = points;
		line.show();

		lines_.push_back(&line);
	}

	input::input(class node & node, input::underlying_type & underlying, real const & x, real const & y, color const & color)
	:
		bases::ports::input(node, underlying, x, y, color)
	{}
	
	namespace inputs {
	
		/**********************************************************************************************************************/
		// single
		
		single::single(class node & node, single::underlying_type & underlying, real const & x, real const & y)
		:
			bases::ports::inputs::single(node, underlying, x, y, boost::cref(color(0x00a00000)))
		{}

		/**********************************************************************************************************************/
		// multiple
		
		multiple::multiple(class node & node, multiple::underlying_type & underlying, real const & x, real const & y)
		:
			bases::ports::inputs::multiple(node, underlying, x, y, boost::cref(color(0x0060a000)))
		{}
	}
}

/**********************************************************************************************************************/
// canvas

canvas::canvas(class graph & graph)
:
	group_(*root()),
	graph_(graph),
	selected_port_(),
	selected_port_2_(),
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
		underlying = &static_cast<node::underlying_type&>(graph().resolver()(type, graph(), name.str()));
	} catch(std::exception const & e) {
		loggers::exception()(e.what(), UNIVERSALIS__COMPILER__LOCATION);
	}
	if(underlying) {
		node & underlying_wrapper(graph().underlying_wrapper(*underlying));
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

void canvas::selected_port(port & port) {
	if(!selected_port_) {
		if(loggers::trace()()) loggers::trace()("connection from");
		selected_port_ = &port;
		ports::output * output_port(dynamic_cast<ports::output*>(&port));
		selected_port_is_output_ = output_port;
	} else {
		if(loggers::trace()()) loggers::trace()("connection to");
		selected_port_2_ = &port;
	}
}

bool canvas::on_event(GdkEvent * event) {
	real x(event->button.x), y(event->button.y);
	switch(event->type) {
		case GDK_ENTER_NOTIFY: {
			loggers::trace()("canvas::on_event: grab focus");
			grab_focus(); // grabs the keyboard focus so that items may receive keyboard events
		}
		break;
		case GDK_LEAVE_NOTIFY: {
			loggers::trace()("canvas::on_event: keyboard ungrab");
			// doesn't seem to do anything
			this->get_display()->keyboard_ungrab(event->crossing.time);
		}
		break;
		case GDK_KEY_PRESS: {
			if(event->key.keyval == GDK_c) {
				loggers::trace()("canvas::on_event: scroll region");
				set_scroll_region_from_bounds();
			}
		}
		break;
		case GDK_BUTTON_PRESS: {
			switch(event->button.button) {
				case 1: {
				}
				break;
				case 2: {
				}
				break;
				case 3: {
					loggers::trace()("canvas::on_event: menu");
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
			} else if(event->motion.state & (GDK_SHIFT_MASK | GDK_CONTROL_MASK)) {
				if(selected_port_) {
					window_to_world(x, y, x, y);
					real port_x(selected_port_->property_x()), port_y(selected_port_->property_y());
					selected_port_->property_parent().get_value()->i2w(port_x, port_y);
					Gnome::Canvas::Points points(2);
					points[selected_port_is_output_ ? 0 : 1] = Gnome::Art::Point(port_x, port_y);
					points[selected_port_is_output_ ? 1 : 0] = Gnome::Art::Point(x, y);
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
			if(
				event->key.keyval == GDK_Shift_L ||
				event->key.keyval == GDK_Shift_R ||
				event->key.keyval == GDK_Control_L ||
				event->key.keyval == GDK_Control_R
			) {
				if(selected_port_2_) {
					if(loggers::trace()()) loggers::trace()("connection");
					//selected_port_is_output_
					ports::output * output_port(dynamic_cast<ports::output*>(selected_port_));
					ports::input * input_port(dynamic_cast<ports::input*>(selected_port_2_));
					if(!(input_port && output_port)) {
						output_port = dynamic_cast<ports::output*>(selected_port_);
						input_port = dynamic_cast<ports::input*>(selected_port_2_);
					}
					if(input_port && output_port) {
						if(loggers::trace()()) {
							std::stringstream s;
							s
								<< "connecting " << input_port->underlying().qualified_name()
								<< " to "  << output_port->underlying().qualified_name();
							loggers::trace()(s.str());
						}
						static_cast<engine::ports::input&>(input_port->underlying())
							.connect(
								static_cast<engine::ports::output&>(output_port->underlying())
							);
					} else {
						if(loggers::trace()()) loggers::trace()("not connecting");
					}
					selected_port_ = selected_port_2_ = 0;
				} else {
					selected_port_ = 0;
					line().hide();
				}
			}
			if(event->key.state & (GDK_SHIFT_MASK | GDK_CONTROL_MASK)) {
			}
		}
		break;
		default: ;
	}
	if(base::on_event(event)) {
		//loggers::trace()("canvas::on_event: base true");
		return true;
	} else {
		//loggers::trace()("canvas::on_event: false");
		return false;
	}
}
}}}
