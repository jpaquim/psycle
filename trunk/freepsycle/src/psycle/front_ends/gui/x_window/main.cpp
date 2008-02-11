// -*- mode:c++; indent-tabs-mode:t -*-
#include <psycle/detail/project.private.hpp>
#include <diversalis/operating_system.hpp>
#if !defined DIVERSALIS__OPERATING_SYSTEM__X_WINDOW
	#error "This source file is for the X Window System only ; try to enable the use of crossplatform libraries in configure's options."
#endif
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <typeinfo>
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <sstream>
#include <universalis/compiler/typenameof.hpp>
#include <universalis/compiler/exceptions/ellipsis.hpp>
namespace psycle
{
	namespace front_ends
	{
		namespace gui
		{
			namespace x_window
			{
				int handler(::Display * display);
				int main(int const argument_count, char const * const arguments[]);
			}
		}
	}
}

int main(int const argument_count, char const * const arguments[])
{
	try
	{
		try
		{
			return psycle::front_ends::gui::x_window::main(argument_count, arguments);
		}
		catch(std::exception const & e)
		{
			std::cerr << "main: exception: " << universalis::compiler::typenameof(e) << ": " << e.what() << std::endl;
			throw;
		}
		catch(...)
		{
			std::cerr << "main: exception: " << universalis::compiler::exceptions::ellipsis() << std::endl;
			throw;
		}
	}
	catch(...)
	{
		std::string s; std::getline(std::cin, s);
		throw;
	}
}

namespace psycle
{
	namespace front_ends
	{
		namespace gui
		{
			namespace x_window
			{
				int handler(::Display * display)
				{
					std::cerr << "x fatal io error handler (this is normal): display: " << DisplayString(display) << std::endl;
					std::exit(1);
				}
			
				int main(int const argument_count, char const * const arguments[])
				{
					::XSetIOErrorHandler(handler);
			
					std::string const display_name(::XDisplayName(argument_count > 1 ? arguments[1] : 0));
			
					std::cout << "opening display named: " << display_name << std::endl;
			
					// Open the display
					::Display & display(*::XOpenDisplay(display_name.c_str()));
					if(!&display)
					{
						std::ostringstream s; s << "could not open display: " << display_name;
						throw std::runtime_error(s.str());
					}
			
					std::cout << ": information on display: " << DisplayString(&display) << std::endl;
					std::cout << ":\t protocol: version: " << ProtocolVersion(&display) << ", revision: " << ProtocolRevision(&display) << std::endl;
					std::cout << ":\t server: vendor: " << ServerVendor(&display) << ", release: " << VendorRelease(&display) << std::endl;
					{
						std::cout << ":\t screens:" << std::endl;
						std::cout << ":\t \t default: " << DefaultScreen(&display) << std::endl;
						for(int s(0) ; s < ScreenCount(&display) ; ++s)
						{
							std::cout << ":\t \t " << s << ':' << std::endl;
							std::cout << ":\t \t \t width:  " << std::setw(4) << DisplayWidth(&display, s)  << " pixels, " << std::setw(4) << DisplayWidthMM(&display, s)  << " millimeters" << std::endl;
							std::cout << ":\t \t \t height: " << std::setw(4) << DisplayHeight(&display, s) << " pixels, " << std::setw(4) << DisplayHeightMM(&display, s) << " millimeters" << std::endl;
							std::cout << ":\t \t \t available depths:";
							int depth_count;
							const int * const depths(::XListDepths(&display, DefaultScreen(&display), &depth_count));
							if(!depths)
							{
								std::ostringstream s; s << "could not retrieve depth list for display: " << DisplayString(&display) << ", screen: " << s;
								throw std::runtime_error(s.str());
							}
							for(int i(0) ; i < depth_count ; ++i) std::cout << ' ' << depths[i];
							std::cout << std::endl;
							::XFree(const_cast<int * const>(depths));
							std::cout << ":\t \t \t default depth of the root window: " << DefaultDepth(&display, s) << std::endl;
							std::cout << ":\t \t \t planes on the root window: " << DisplayPlanes(&display, s) << std::endl;
						}
					}
					{
						int pixmap_format_count;
						::XPixmapFormatValues const * const pixmap_formats(::XListPixmapFormats(&display, &pixmap_format_count));
						if(!pixmap_formats)
						{
							std::ostringstream s; s << "could not retrieve pixmap format list for display: " << DisplayString(&display);
							throw std::runtime_error(s.str());
						}
						std::cout << ":\t pixmap formats:" << std::endl;
						for(int f(0) ; f < pixmap_format_count ; ++f)
						{
							std::cout << ":\t \t " << f;
							std::cout << ": depth: " << std::setw(2) << pixmap_formats[f].depth;
							std::cout << ", bits per pixel: " << std::setw(2) << pixmap_formats[f].bits_per_pixel;
							std::cout << ", scanline pad: " << std::setw(2) << pixmap_formats[f].scanline_pad;
							std::cout << std::endl;
						}
						::XFree(const_cast<XPixmapFormatValues * const>(pixmap_formats));
					}
			
					std::cout.flush();
			
					// Load the font to use.  See Sections 10.2 & 6.5.1
					std::string const font_name("fixed");
					::XFontStruct & font(*::XLoadQueryFont(&display, font_name.c_str()));
					if(!&font)
					{
						std::ostringstream s; s << ": could not load font: " << font_name;
						throw std::runtime_error(s.str());
					}
					int const font_height(font.max_bounds.ascent + font.max_bounds.descent);
			
					// Get some colors
					unsigned int const black_color(BlackPixel(&display, DefaultScreen(&display)));
					unsigned int const white_color(WhitePixel(&display, DefaultScreen(&display)));
			
					int const border(1);
					int const pad(1);
			
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
					std::string const text("This is just a test. Use the GTKMM/GNOMEMM front-end instead.");
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
			
					// Deal with providing the window with an initial position & size.
					// Fill out the XSizeHints struct to inform the window manager. See Sections 9.1.6 & 10.3.
					::XSizeHints size_hints; // Size hints for window manager
					size_hints.flags = PPosition | PSize;
					size_hints.height = font_height + pad * 2;
					size_hints.width = ::XTextWidth(&font, text.c_str(), text.size()) + pad * 2;
					size_hints.x = (DisplayWidth(&display, DefaultScreen(&display)) - size_hints.width) / 2;
					size_hints.y = (DisplayHeight(&display, DefaultScreen(&display)) - size_hints.height) / 2;
			
					// Create the window with the information in the XSizeHints, the
					// border width, and the border & background pixels. See Section 3.3.
					::Window window
						(
							::XCreateSimpleWindow
							(
								&display,
								DefaultRootWindow(&display),
								size_hints.x, size_hints.y, size_hints.width, size_hints.height,
								border,
								white_color, // border color
								white_color // background color
							)
						);
			
					// This structure forms the WM_HINTS property of the window,
					// letting the window manager know how to handle this window.
					// See Section 9.1 of the Xlib manual.
					::XWMHints window_manager_hints =
					{
						InputHint | StateHint, // flags
						false, // input
						NormalState, // initial_state
						0, // icon pixmap
						0, // icon window
						0, 0, // icon location
						0, // icon mask
						0, // Window group
					};
			
					// Set the standard properties for the window managers. See Section 9.1.
					::XSetStandardProperties(&display, window, text.c_str(), text.c_str(), None, const_cast<char**>(arguments), argument_count, &size_hints);
					::XSetWMHints(&display, window, &window_manager_hints);
			
					// Ensure that the window's colormap field points to the default
					// colormap, so that the window manager knows the correct colormap to
					// use for the window. See Section 3.2.9.
					::XSetWindowAttributes window_attributes_set;
					window_attributes_set.colormap = DefaultColormap(&display, DefaultScreen(&display));
					window_attributes_set.bit_gravity = CenterGravity; // Also, set the window's Bit Gravity to reduce Expose events.
					::XChangeWindowAttributes(&display, window, CWColormap | CWBitGravity, &window_attributes_set);
			
					// Create the Graphics Context for writing the text.  See Section 5.3.
					XGCValues gc_values;
					gc_values.font = font.fid;
					gc_values.foreground = black_color;
					gc_values.background = white_color;
					::GC gc(::XCreateGC(&display, window, GCFont | GCForeground | GCBackground, &gc_values));
			
					// Tell the GC we draw using the black color
					::XSetForeground(&display, gc, black_color);
			
					// Specify the event types we're interested in - only Exposures. See Sections 8.5 & 8.4.5.1
					// We want to get MapNotify events also
					::XSelectInput(&display, window, ExposureMask | StructureNotifyMask);
			
					// "Map" the window (that is, make it appear on the screen)
					::XMapWindow(&display, window);
			
					// Wait for the MapNotify event
					for(;;)
					{
						::XEvent event;
						// Get the next event
						::XNextEvent(&display, &event);
						if(event.type == MapNotify) break;
					}
			
					std::cout << ": received map notification event" << std::endl;
			
					// Loop forever, examining each event.
					for(;;)
					{
						::XEvent event;
						// Get the next event
						::XNextEvent(&display, &event);
			
						//std::cout << ": display: " << DisplayString(&display) << ", received event type: " << event.type << std::endl;
			
						// On the last of each group of Expose events, repaint the entire window. See Section 8.4.5.1.
						if(event.type == Expose && event.xexpose.count == 0)
						{
							// Remove any other pending Expose events from the queue to avoid multiple repaints. See Section 8.7.
							while(::XCheckTypedEvent(&display, Expose, &event));
			
							// Find out how big the window is now, so that we can center the text in it.
							::XWindowAttributes window_attributes;
							if(!::XGetWindowAttributes(&display, window, &window_attributes))
							{
								std::ostringstream s; s << ": could not get window attributes";
								throw std::runtime_error(s.str());
							}
							
							int const x((window_attributes.width - ::XTextWidth(&font, text.c_str(), text.size())) / 2);
							int const y((window_attributes.height + font.max_bounds.ascent - font.max_bounds.descent) / 2);
			
							// Fill the window with the background color.
							::XClearWindow(&display, window);
			
							// Paint the centered string.
							::XDrawString(&display, window, gc, x, y, text.c_str(), text.size());
							
							// Draw some lines
							//::XDrawLine(&display, window, gc, 0, 0, window_attributes.width, window_attributes.height);
							//::XDrawLine(&display, window, gc, 0, window_attributes.width, 0, window_attributes.height);
			
							// Send the requests to the server
							//::XFlush(&display);
						}
					}
					return 0;
				}
			}
		}
	}
}
