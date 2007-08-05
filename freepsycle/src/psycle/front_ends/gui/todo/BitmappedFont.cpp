///\file
///\brief implementation file for ...
#include <stdexcept>
#include <sstream>
#include "BitmappedFont.hpp"
namespace psycle
{
	BitmappedFont::BitmappedFont
	(
		const std::string& fontname,
		const unsigned int& zoom,
		const Glib::RefPtr<Gdk::Drawable>& drawable,
		const Glib::RefPtr<Gdk::GC>& gc
	) throw(std::exception)
	:
		fontname(fontname),
		zoom(zoom),
		mask_data(0)
	{
		initialise_font(drawable, gc);
	}
	
	BitmappedFont::~BitmappedFont() throw()
	{
		delete[] mask_data;
	}
	
	void BitmappedFont::initialise_font(const Glib::RefPtr<Gdk::Drawable> & drawable, const Glib::RefPtr<Gdk::GC> & gc) throw(std::exception)
	{
		try
		{
			font_pixbuf = Gdk::Pixbuf::create_from_file(fontname);
		}
		catch(const Glib::FileError & e)
		{
			std::ostringstream ss; ss << "could not create pixbuf from file " << fontname << std::endl << typeid(e).name() << std::endl << e.what();
			throw std::runtime_error(ss.str());
		}
		catch(const Gdk::PixbufError & e)
		{
			std::ostringstream ss; ss << "could not create pixbuf from file " << fontname << std::endl << typeid(e).name() << std::endl << e.what();
			throw std::runtime_error(ss.str());
		}
		font_pixbuf = font_pixbuf->scale_simple(font_pixbuf->get_width() * zoom, font_pixbuf->get_height() * zoom, Gdk::INTERP_NEAREST);
		pixbuf_width = font_pixbuf->get_width();
		char_height = font_pixbuf->get_height();
		//pixel_count = pixbuf_width * char_height;
		//mask_data = new char[pixel_count];
		//memset(mask_data, '\0', sizeof(char) * pixel_count);
		font_pixmap = Gdk::Pixmap::create(drawable, pixbuf_width, char_height);
		//font_mask = Gdk::Bitmap::create(drawable, mask_data, pixbuf_width, char_height);
		//font_pixbuf->render_threshold_alpha(font_mask, 0, 0, 0, 0, -1, -1, 1);
		gc_mask = Gdk::GC::create(font_pixmap);
		//gc_mask->set_clip_mask(font_mask);
		font_pixmap->draw_pixbuf(gc_mask, font_pixbuf, 0, 0, 0, 0, pixbuf_width, char_height, Gdk::RGB_DITHER_NONE, 0, 0);
		calculate_offsets_and_widths();
		x_spacing = zoom;
		y_spacing = zoom;
	}
	
	void BitmappedFont::calculate_offsets_and_widths()
	{
		guint32 spacer_colour;
		int current_x_pos = 0;
		int current_char = 32;
		Glib::RefPtr<Gdk::Image> font_image = font_pixmap->get_image(0, 0, pixbuf_width, char_height);
		spacer_colour = font_image->get_pixel(current_x_pos,0);
		while(current_char < 127)
		{
			while(font_image->get_pixel(current_x_pos,0) == spacer_colour)
				++current_x_pos;
			char_x_offset[current_char] = current_x_pos;
			while(font_image->get_pixel(current_x_pos,0) != spacer_colour)
				++current_x_pos;
			char_width[current_char] = (current_x_pos - char_x_offset[current_char]);
			++current_char;
		  }
	}
	
	int BitmappedFont::get_height()
	{
	    return char_height;
	}
	
	int BitmappedFont::get_width(const std::string& text)
	{
		int width = 0;
		int current_char_pos = 0;
		int text_length = text.length();
		while(current_char_pos < text_length)
			width += char_width[text[current_char_pos]] + x_spacing;
		if(text_length > 1)
			width -= x_spacing;
		return width;
	}
	
	Gdk::Rectangle& BitmappedFont::print
	(
		const std::string & text,
		const int & x,
		const int & y,
		const int & printwidth,
		const Glib::RefPtr<Gdk::Drawable> & drawable,
		const Glib::RefPtr<Gdk::GC> & gc,
		Gdk::Rectangle & update_rect
	) throw(std::exception)
	{
		int current_x = x;                      //the x position of the current char to be rendered
		int current_y = y;                      // the y         "              "
		int current_char_pos = 0;               //the position of the current char in the text string
		char current_char;                      // duh
		int text_length = text.length();        //copied locally to save cpu
		update_rect.set_x(x);                   //left of area the are that the text will be rendered to
		update_rect.set_y(y);                   //top     "                 "
		int word_width;                         //width of the current word
		int word_char_pos;                      //the position of the current char in the current word
		char word_char;                         //the current char in the current word
		bool new_word = false;                   //bool to determine  if we've encountered a new word
		while(current_char_pos < text_length)
		{
			current_char = text[current_char_pos];
			if(new_word == true)               //figure out if the word runs over the edge of the printwidth, if so move the cursor
			{                                   //onto the next line
				new_word = false;
				word_char_pos = current_char_pos;
				word_char = current_char;
				word_width = 0;
				while((word_char != ' ') && (word_char_pos < text_length))  //determine word length
				{
					word_char = text[word_char_pos];
					word_width += char_width[word_char] + x_spacing;
					word_char_pos++;
				}
				if(word_char_pos != current_char_pos)
					word_width -= x_spacing;
				if((current_x + word_width) > (printwidth + x))                //if the word runs over the edge of the printwidth
				{                                                               //move onto a new line
					if((current_char == 32) && (current_char_pos < (text_length - 1)))        //don't print a space at the beginning
						current_char_pos++;                                                   //of a new line
					if((word_width <= printwidth) && (current_y == y))                        //only set the update rect size once
						update_rect.set_width(printwidth);
					else if(word_width > printwidth)                                        //unless the word width is bigger than
						update_rect.set_width(word_width);                                    //printwidth
					current_x = x;
					current_y += char_height + y_spacing;
				}
			}
			
			//drawable->draw_drawable(gc_mask, font_pixmap, char_x_offset[current_char], 0, current_x, current_y, char_width[current_char], char_height);
			drawable->draw_pixbuf(gc_mask, font_pixbuf, char_x_offset[current_char], 0, current_x, current_y, char_width[current_char], char_height, Gdk::RGB_DITHER_NONE, 0, 0);
			current_x += char_width[current_char] + x_spacing;
			if(current_char == ' ')
				new_word = true;
			++current_char_pos;
		}
		if(current_y == y)                                     //if text hasnt run to a newline then set the width of the update rect
			update_rect.set_width(current_x - x_spacing);
		update_rect.set_height(current_y + char_height);
		return update_rect;
	}
}
