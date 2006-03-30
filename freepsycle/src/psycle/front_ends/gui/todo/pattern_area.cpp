///\file
///\brief implementation file for ...
#include <sstream>
#include "pattern_area.hpp"
namespace psycle
{
	pattern_area::pattern_area() throw(std::exception)
	:
		the_font(0)
	{
		// get_window() would return 0 because the Gdk::Window has not yet been realized
		pat1 = new Pattern("Pattern 1");
		pat1->add_event(0, *new Event(0));
		pat1->add_event(0, *new Event(1000));
		std::cout << pat1->event_count();
		bpm = 128;
	}
	
	pattern_area::~pattern_area() throw()
	{
		delete the_font;
	}
	
	void pattern_area::draw_pattern(const int& line_start, const int& line_end, const int& col_start, const int& col_end, const int& x, const int& y)
	{
		draw_borders(x, x + (col_end - col_start) * channel_width, y, y + (line_end - line_start + 1) * line_height);
	}
	
	void pattern_area::clear()
	{
		pixmap_pattern_area->draw_rectangle(get_style()->get_black_gc(), true, 0, 0, pattern_area_width - 1, pattern_area_height - 1);
	}
	
	void pattern_area::draw_borders(const int& left, int right, const int& top, int bottom)
	{
		// make sure the drawing doesnt go off the edge of the drawingarea
		if(bottom > pattern_area_height) bottom = pattern_area_height;
		if(right > pattern_area_width) right = pattern_area_width;
		for(int current_x(left) ; current_x < right ; current_x += channel_width)
		{
			pixmap_pattern_area->draw_line(gc_, current_x, top, current_x, bottom);
		}
	}
	
	void pattern_area::draw_line_numbers()
	{
		{
			int current_y(0), line_number(0);
			while(current_y < pattern_area_height - 1)
			{
				std::stringstream ss; ss << line_number; std::string s; ss >> s;
				Gdk::Rectangle update_rect;
				update_rect = the_font->print(s, 4, current_y + 2, 300, pixmap_pattern_area, gc_, update_rect);
				++line_number;
				current_y += line_height;
			}
		}
	}
	
	void pattern_area::draw_highlights()
	{
		for(int current_y(0) ; current_y < pattern_area_height - 1 ; current_y += line_height * 4)
		{
			std::cout << " " << current_y;
			pixmap_pattern_area->draw_rectangle(gc_, true, line_number_width, current_y, pattern_area_width, line_height);
			//invalidate(Gdk::Rectangle(0, 0, pattern_area_width, pattern_area_height));
		}
	}
	
	
	void pattern_area::on_realize() throw(std::exception)
	{
		Gtk::DrawingArea::on_realize();
		the_font = new BitmappedFont("font1b.png", 1, get_window(), get_style()->get_black_gc());
		window = get_window();
		gc_ = Gdk::GC::create(window);
		window->clear();
		set_size_request(500,500);
		highlight_colour = Gdk::Color::Color();
		border_colour = Gdk::Color::Color();
		highlight_colour.set_rgb(10000, 10000, 20000);
		border_colour.set_rgb(15000, 15000, 25000);
		gc_->set_rgb_fg_color(highlight_colour);
		//gc_->set_rgb_bg_color(highlight_colour);
		line_height = 14;
		line_number_width = 23;
		channel_width = 88;
		get_size_request(pattern_area_width, pattern_area_height);
		add_events(Gdk::BUTTON_PRESS_MASK | Gdk::KEY_PRESS_MASK);
		signal_button_press_event().connect( sigc::mem_fun(*this, &pattern_area::button_press) );
		pixmap_pattern_area = Gdk::Pixmap::create(get_window(), pattern_area_width - 1, pattern_area_height - 1, -1);
		clear();
		draw_pattern(0, 63, 0, 15, 0, 0);
		//draw_highlights();
		//gc_->set_rgb_fg_color(border_colour);
		//draw_borders();
		//draw_line_numbers();
		Gdk::Rectangle update_rect;
		//update_rect = the_font->print(std::string("C-1 80 0c20 "), 26, 2, 300, pixmap_pattern_area, gc_, update_rect);
		update_rect = Gdk::Rectangle(0, 0, pattern_area_width - 1, pattern_area_height - 1);
		invalidate(update_rect);
	}
	
	bool pattern_area::on_expose_event(GdkEventExpose * event)
	{
		// This is where we draw on the window
		window->draw_drawable
		(
			gc_, pixmap_pattern_area,
			// Only copy the area that was exposed:
			event->area.x, event->area.y,
			event->area.x, event->area.y,
			event->area.width, event->area.height
		);
		return true;
	}
	
	void pattern_area::invalidate(const Gdk::Rectangle& update_rect)
	{
		window->invalidate_rect(update_rect, false);
	}
	
	bool pattern_area::button_press(GdkEventButton* event)
	{
		return true;
	}
}
