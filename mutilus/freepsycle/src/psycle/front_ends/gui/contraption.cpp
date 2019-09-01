// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2007 psycledelics http://psycle.pastnotecut.org : johan boule

///\file
///\brief \implementation psycle::front_ends::gui::contraption
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <psycle/detail/project.private.hpp>
#include "contraption.hpp"
#include <universalis/operating_system/loggers.hpp>
namespace psycle
{
	namespace front_ends
	{
		namespace gui
		{
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
					#if !defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
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
		}
	}
}
