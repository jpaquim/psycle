// -*- mode:c++; indent-tabs-mode:t -*-
///\file
///\brief interface file for ...
#pragma once
#include <exception>
#include <string>
#include <gdkmm/pixbuf.h>
#include <gdkmm/pixmap.h>
#include <gdkmm/drawable.h>
#include <gdkmm/gc.h>
namespace psycle
{
	class BitmappedFont
	{
	public:
		BitmappedFont
		(
			const std::string & fontname,
			const unsigned int & zoom,
			const Glib::RefPtr<Gdk::Drawable> & drawable,
			const Glib::RefPtr<Gdk::GC> & gc
		) throw(std::exception);
		virtual ~BitmappedFont() throw();
		Gdk::Rectangle & print
		(
			const std::string & text,
			const int & x,
			const int & y,
			const int & printwidth,
			const Glib::RefPtr<Gdk::Drawable> & drawable,
			const Glib::RefPtr<Gdk::GC> & gc,
			Gdk::Rectangle & update_rect
		) throw(std::exception);
		int get_width(const std::string & text);
		int get_height();

	private:
		void initialise_font(const Glib::RefPtr<Gdk::Drawable> & drawable, const Glib::RefPtr<Gdk::GC> & gc) throw(std::exception);
		void calculate_offsets_and_widths();

	protected:
		Glib::RefPtr<Gdk::Pixbuf> font_pixbuf;
		Glib::RefPtr<Gdk::Bitmap> font_mask;
		char * mask_data;
		Glib::RefPtr<Gdk::GC> gc_mask;
		Glib::RefPtr<Gdk::Pixmap> font_pixmap;
		std::string fontname;
		/// the offsets (starting location) of each char
		int char_x_offset[128]; 
		/// the widths of each char
		int char_width[128];    
		/// the height of the font image (and a char)
		int char_height;        
		/// the width of the font image
		int pixbuf_width;       
		/// the space between each letter horizontally
		int pixel_count;
		///the space between each letter vertically
		int x_spacing;          
		int y_spacing;          
		/// the font size is multiplied by zoom
		unsigned int zoom;      
	};
}
