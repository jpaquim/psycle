// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2005 Psycledelics http://psycle.pastnotecut.org : Johan Boule

///\file
///\brief \implementation psycle::front_ends::gui::graph
#include PACKAGENERIC__PRE_COMPILED
#include PACKAGENERIC
#include <psycle/detail/project.private.hpp>
#include "graph.hpp"
#include "lock.hpp"
#include <boost/bind.hpp>
#include <gdkmm/color.h>
#include <gdkmm/colormap.h>
namespace psycle
{
	namespace front_ends
	{
		namespace gui
		{
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
//				pack_start(start(), Gtk::PACK_SHRINK);
//				pack_start(spin_, Gtk::PACK_SHRINK);
				scroll_.add(canvas_instance());
				pack_start(scroll_);
				show_all_children();

				start().signal_toggled().connect(sigc::mem_fun(*this, &graph::on_start));

				adjustment_.signal_value_changed().connect(sigc::mem_fun(*this, &graph::on_zoom));
			}

			graph::~graph() throw()
			{
			}
			
			void graph::on_start()
			{
				scheduler().started(start().get_active());
			}
			
			void graph::on_size_allocate(Gtk::Allocation & allocation)
			{
				base::on_size_allocate(allocation);
			}

			void graph::on_zoom()
			{
				canvas_instance().set_pixels_per_unit(adjustment_.get_value());
			}

			contraption::contraption(Gnome::Canvas::Group & parent, real const & x, real const & y, color const & color, std::string const & text)
			:
				Gnome::Canvas::Group(parent, x, y),
				rectangle_(*this),
				text_shadow_(*this),
				text_highlight_(*this),
				fill_color_(color.a(0x80)),
				outline_color_(color.a(0xd0)),
				highlight_fill_color_(color.a(0xb0)),
				highlight_outline_color_(color.a(0xf0)),
				dragging_(false)
			{
				Pango::FontDescription static * font_description(0);
				if(!font_description)
				{
					if(loggers::trace()())
					{
						loggers::trace()("pango: font description: creating ...", UNIVERSALIS__COMPILER__LOCATION);
					}
					font_description = new Pango::FontDescription;
					font_description->set_family("helvetica");
					#if !defined OPERATING_SYSTEM__MICROSOFT
						font_description->set_weight(Pango::WEIGHT_BOLD);
					#else
						font_description->set_weight(Pango::WEIGHT_ULTRABOLD);
						// for some fonts, microsoft's operating system seems not to handle scalar weights, but only 400 (Pango::WEIGHT_NORMAL), and 800 (Pango::WEIGHT_ULTRABOLD).
						// related bug?
						// http://bugzilla.gnome.org/show_bug.cgi?id=110521
						// pango versions prior to 1.6 have a bug in their implementation for microsoft's operating system
						// which makes requests of bold or italic fonts fail, and the rendering to become very slow.
					#endif
					font_description->set_size(Pango::SCALE * 8); // in points / Pango::SCALE
					if(loggers::trace()())
					{
						loggers::trace()("pango: font description: " + font_description->to_string(), UNIVERSALIS__COMPILER__LOCATION);
					}
				}

				text_highlight().property_font_desc() = *font_description;
				text_highlight().property_fill_color() = "white";
				text_highlight().property_justification() = Gtk::JUSTIFY_CENTER;
				text_highlight().property_anchor() = Gtk::ANCHOR_CENTER;
				text_highlight().property_x() = 0;
				text_highlight().property_y() = 0;

				text_shadow().property_font_desc() = text_highlight().property_font_desc().get_value();
				text_shadow().property_fill_color() = "black";
				text_shadow().property_justification() = text_highlight().property_justification().get_value();
				text_shadow().property_anchor() = text_highlight().property_anchor().get_value();
				text_shadow().property_x() = text_highlight().property_x() + 1;
				text_shadow().property_y() = text_highlight().property_y() + 1;

				rectangle().property_fill_color_rgba() = fill_color();
				rectangle().property_outline_color_rgba() = outline_color();
				
				this->text(text);
			}
			
			void contraption::text(std::string const & text)
			{
				text_highlight().property_text() = text;
				text_shadow().property_text() = text_highlight().property_text().get_value();

				real text_highlight_bounds_x_min, text_highlight_bounds_y_min, text_highlight_bounds_x_max, text_highlight_bounds_y_max;
				text_highlight().get_bounds(text_highlight_bounds_x_min, text_highlight_bounds_y_min, text_highlight_bounds_x_max, text_highlight_bounds_y_max);

				real text_shadow_bounds_x_min, text_shadow_bounds_y_min, text_shadow_bounds_x_max, text_shadow_bounds_y_max;
				text_shadow().get_bounds(text_shadow_bounds_x_min, text_shadow_bounds_y_min, text_shadow_bounds_x_max, text_shadow_bounds_y_max);

				rectangle().property_x1() = std::min(text_highlight_bounds_x_min, text_shadow_bounds_x_min) - 1;
				rectangle().property_y1() = std::min(text_highlight_bounds_y_min, text_shadow_bounds_y_min);// - 1;
				rectangle().property_x2() = std::max(text_highlight_bounds_x_max, text_shadow_bounds_x_max) + 1;
				rectangle().property_y2() = std::max(text_highlight_bounds_y_max, text_shadow_bounds_y_max);// + 1;
			}

			void contraption::dragging_start(real const & x, real const & y, time const & time)
			{
				assert(!dragging_);
				dragging_ = true;
				dragging_x_ = x;
				dragging_y_ = y;
				grab(GDK_POINTER_MOTION_MASK | GDK_BUTTON_RELEASE_MASK, Gdk::Cursor(Gdk::FLEUR), time);
			}

			void contraption::dragging(real const & x, real const & y)
			{
				assert(dragging_);
				property_parent().get_value()->move(x - dragging_x_, y - dragging_y_);
				dragging_x_ = x;
				dragging_y_ = y;
				signal_move()(*this);
			}
			
			void contraption::dragging_stop(time const & time)
			{
				assert(dragging_);
				dragging_ = false;
				ungrab(time);
			}

			bool contraption::on_event(GdkEvent * event)
			{
				if(Gnome::Canvas::Group::on_event(event)) return true;
				real x(event->button.x), y(event->button.y);
				switch(event->type)
				{
					case GDK_ENTER_NOTIFY:
					{
						rectangle().property_fill_color_rgba() = highlight_fill_color();
						rectangle().property_outline_color_rgba() = highlight_outline_color();
						signal_enter()(*this);
						//return true;
					}
					break;
					case GDK_KEY_PRESS:
					{
						//if(event->key.state & GDK_SHIFT_MASK)
						{
							signal_select()(*this);
							//return true;
						}
					}
					break;
					case GDK_BUTTON_PRESS:
					{
						switch(event->button.button)
						{
							case 1:
							{
								dragging_start(x, y, event->button.time);
								return true;
							}
							break;
							case 2:
							{
								if(event->button.state & GDK_SHIFT_MASK)
								{
									property_parent().get_value()->lower_to_bottom();
									//return true;
								}
								else
								{
									property_parent().get_value()->lower(1);
									//return true;
								}
							}
							break;
							case 3:
							{
								if(event->button.state & GDK_SHIFT_MASK)
								{
									property_parent().get_value()->raise_to_top();
									//return true;
								}
								else
								{
									property_parent().get_value()->raise(1);
									//return true;
								}
							}
							break;
							default: ;
						}
					}
					break;
					case GDK_MOTION_NOTIFY:
					{
						if(dragging() && event->motion.state & GDK_BUTTON1_MASK)
						{
							dragging(x, y);
							return true;
						}
					}
					break;
					case GDK_BUTTON_RELEASE:
					{
						switch(event->button.button)
						{
							case 1:
							{
								if(dragging())
								{
									dragging_stop(event->button.time);
									return true;
								}
							}
							break;
						}
					}
					break;
					case GDK_KEY_RELEASE:
					{
					}
					break;
					case GDK_LEAVE_NOTIFY:
					{
						if(!(event->crossing.state & GDK_BUTTON1_MASK))
						{
							rectangle().property_fill_color_rgba() = fill_color();
							rectangle().property_outline_color_rgba() = outline_color();
							signal_leave()(*this);
							//return true;
						}
					}
					break;
					default: ;
				}
				return false;
			}

			node::node(node::parent_type & parent, node::underlying_type & underlying, real const & x, real const & y)
			:
				node_type(parent, underlying),
				Gnome::Canvas::Group(parent, x, y),
				contraption_(*this, 0, 0, 0x60606000, underlying.name() + "\n" + underlying.plugin_library_reference_instance().name())
			{
			}

			void node::init()
			{
				node_type::init();
				int const ports(single_input_ports().size() + output_ports().size() + (multiple_input_port() ? 1 : 0));
				real const angle_step(engine::math::pi * 2 / ports);
				real const radius(60);
				real angle(0);
				for(output_ports_type::const_iterator i(output_ports().begin()) ; i != output_ports().end() ; ++i)
				{
					typenames::ports::output & output_port(**i);
					Gtk::manage(&output_port);
					real const x(radius * std::cos(angle)), y(radius * std::sin(angle));
					output_port.contraption_instance().property_parent().get_value()->move(x, y);
					output_port.contraption_instance().signal_move()(output_port.contraption_instance());
					angle += angle_step;
				}
				if(multiple_input_port())
				{
					typenames::ports::inputs::multiple & multiple_input_port(*this->multiple_input_port());
					Gtk::manage(&multiple_input_port);
					real const x(radius * std::cos(angle)), y(radius * std::sin(angle));
					multiple_input_port.contraption_instance().property_parent().get_value()->move(x, y);
					multiple_input_port.contraption_instance().signal_move()(multiple_input_port.contraption_instance());
					angle += angle_step;
				}
				for(single_input_ports_type::const_iterator i(single_input_ports().begin()) ; i != single_input_ports().end() ; ++i)
				{
					typenames::ports::inputs::single & single_input_port(**i);
					Gtk::manage(&single_input_port);
					real const x(radius * std::cos(angle)), y(radius * std::sin(angle));
					single_input_port.contraption_instance().property_parent().get_value()->move(x, y);
					single_input_port.contraption_instance().signal_move()(single_input_port.contraption_instance());
					angle += angle_step;
				}
				contraption_instance().raise_to_top();
			}

			bool node::on_event(GdkEvent * event)
			{
				if(Gnome::Canvas::Group::on_event(event)) return true;
				switch(event->type)
				{
					case GDK_ENTER_NOTIFY:
					{
					}
					break;
					case GDK_KEY_PRESS:
					{
					}
					break;
					case GDK_BUTTON_PRESS:
					{
						switch(event->button.button)
						{
						}
					}
					break;
					case GDK_MOTION_NOTIFY:
					{
					}
					break;
					case GDK_BUTTON_RELEASE:
					{
						switch(event->button.button)
						{
						}
					}
					break;
					case GDK_KEY_RELEASE:
					{
					}
					break;
					case GDK_LEAVE_NOTIFY:
					{
					}
					break;
					default: ;
				}
				return false;
			}

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

			void port::on_select(contraption & contraption)
			{
				if(!parent().parent().canvas_instance().selected_port())
					parent().parent().canvas_instance().selected_port(*this);
			}
			
			void port::on_move(contraption & contraption)
			{
				Gnome::Canvas::Points points(2);
				points[0] = Gnome::Art::Point(0, 0);
				real node_x(parent().property_x()), node_y(parent().property_y());
				w2i(node_x, node_y);
				points[1] = Gnome::Art::Point(node_x, node_y);
				line().property_points() = points;
			}

			bool port::on_event(GdkEvent * event)
			{
				if(Gnome::Canvas::Group::on_event(event)) return true;
				real x(event->button.x), y(event->button.y);
				switch(event->type)
				{
					case GDK_ENTER_NOTIFY:
					{
					}
					break;
					case GDK_KEY_PRESS:
					{
					}
					break;
					case GDK_BUTTON_PRESS:
					{
						switch(event->button.button)
						{
						}
					}
					break;
					case GDK_MOTION_NOTIFY:
					{
					}
					break;
					case GDK_BUTTON_RELEASE:
					{
						switch(event->button.button)
						{
						}
					}
					break;
					case GDK_KEY_RELEASE:
					{
					}
					break;
					case GDK_LEAVE_NOTIFY:
					{
					}
					break;
					default: ;
				}
				return false;
			}

			namespace ports
			{
				output::output(output::parent_type & parent, output::underlying_type & underlying, real const & x, real const & y)
				:
					output_type(parent, underlying, x, y, boost::cref(color(0xa0600000)))
				{
				}

				input::input(input::parent_type & parent, input::underlying_type & underlying, real const & x, real const & y, color const & color)
				:
					input_type(parent, underlying, x, y, color)
				{
				}
				
				namespace inputs
				{
					single::single(single::parent_type & parent, single::underlying_type & underlying, real const & x, real const & y)
					:
						single_type(parent, underlying, x, y, boost::cref(color(0x00a00000)))
					{
					}

					multiple::multiple(multiple::parent_type & parent, multiple::underlying_type & underlying, real const & x, real const & y)
					:
						multiple_type(parent, underlying, x, y, boost::cref(color(0x0060a000)))
					{
					}
				}
			}

			canvas::canvas(graph & graph)
			:
				group_(*root()),
				graph_(graph),
				selected_port_(0),
				line_(*root())
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
			}
			
			bool canvas::on_event(GdkEvent * event)
			{
				if(base::on_event(event)) return true;
				real x(event->button.x), y(event->button.y);
				switch(event->type)
				{
					case GDK_ENTER_NOTIFY:
					{
					}
					break;
					#if 0
					case GDK_KEY_PRESS:
					{
						if(event->key.state & GDK_SHIFT_MASK)
						{
						//	dragging_ = true;
						//	grab(GDK_KEY_RELEASE_MASK | GDK_POINTER_MOTION_MASK | GDK_BUTTON_RELEASE_MASK, Gdk::Cursor(Gdk::TCROSS), event->key.time);
						}
					}
					break;
					#endif
					case GDK_BUTTON_PRESS:
					{
						switch(event->button.button)
						{
							case 1:
							{
							}
							break;
							case 2:
							{
								window_to_world(x, y, x, y);
								node & node_instance(*Gtk::manage(&node::create(graph_instance(), graph_instance().resolver()("sine", graph_instance(), "node"), x, y)));
								//return true;
							}
							break;
							case 3:
							{
								real bounds_x_min, bounds_y_min, bounds_x_max, bounds_y_max;
								root()->get_bounds(bounds_x_min, bounds_y_min, bounds_x_max, bounds_y_max);
								real scroll_x_min, scroll_y_min, scroll_x_max, scroll_y_max;
								//get_scroll_region(scroll_x_min, scroll_y_min, scroll_x_max, scroll_y_max);
								//set_scroll_region(std::min(bounds_x_min, scroll_x_min), std::min(bounds_y_min, scroll_y_min), std::max(bounds_x_max, scroll_x_max), std::max(bounds_y_max, scroll_y_max));
								set_scroll_region(bounds_x_min, bounds_y_min, bounds_x_max, bounds_y_max);
								//return true;
							}
							break;
						}
					}
					break;
					case GDK_MOTION_NOTIFY:
					{
						if(event->motion.state & GDK_BUTTON1_MASK)
						{
						}
						else if(event->motion.state & GDK_SHIFT_MASK)
						{
							if(selected_port())
							{
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
					case GDK_BUTTON_RELEASE:
					{
						switch(event->button.button)
						{
						}
					}
					break;
					case GDK_KEY_RELEASE:
					{
						//if(event->key.state & GDK_CONTROL_MASK)
						{
							selected_port_ = 0;
							line().hide();
						}
						#if 0
						if(event->key.state & GDK_SHIFT_MASK)
						{
							dragging_ = false;
							ungrab(event->key.time);
						}
						if(other)
						{
							ports::output * output_port(dynamic_cast<ports::output*>(this));
							ports::input * input_port(dynamic_cast<ports::input*>(other));
							if(!(input_port && output_port))
							{
								output_port = dynamic_cast<ports::output*>(this);
								input_port = dynamic_cast<ports::input*>(other);
							}
							if(input_port && output_port)
							{
								static_cast<engine::ports::input&>(other->underlying_instance())
									.connect
									(
										static_cast<engine::ports::output&>(this->underlying_instance())
									);
							}
							other = 0;
						}
						#endif
					}
					break;
					case GDK_LEAVE_NOTIFY:
					{
					}
					break;
					default: ;
				}
				return false;
			}
		}
	}
}
