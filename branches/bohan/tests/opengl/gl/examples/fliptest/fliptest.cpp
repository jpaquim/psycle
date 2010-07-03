/**

	\file fliptest.c
	\author Nafees Bin Zafar <nafees@d2.com>

	$Id: fliptest.c,v 1.1.1.1 2003/02/20 08:51:07 nafees Exp $

	\brief Test of OpenGL glDrawPixels speed for use in flipbooks.

**/

/*

	This program simulates a flipbook viewer drawing a sequence of
	images out of memory. It reports the frames per second it achieves
	on stdout.  Real flipbook programs can be written to reliably get
	any speed up to the speed this program reports.

	To run it you provide it with three arguments: width, height, and
	'd' for double buffer or 's' for single buffer:

	fliptest 640 480 d  # typical video playback
	fliptest 1024 554 d # typical 1/2 rez film with letterbox
	fliptest 1024 778 d # typical 1/2 rez full-ap film
	fliptest 2048 1107 d # typical full-rez film with letterbox
	fliptest 2048 1556 d # full rez full-ap film
	fliptest 1828 1371 d # full rez acadamy frame
	fliptest 1828 685 d  # anamorphic acadamy images, 1/2 rez vertically

	The vertical refresh is the max achievable performance. It should
	work no matter where the window is placed or if portions are clipped
	off by the edges of the screen. Lowest acceptable values are 24 for
	the large ones and 30 for the video.

	This can also be used to detect tearing of the image due to lack of
	synchronization between the double buffer swaps and the screen. This
	should never produce these artifacts when double buffering is on, no
	matter where the window is placed on the screen.  However particular
	OpenGL implementations often require a driver or environment setting
	to enable redraw sync.

	For NVidia's OpenGL implementation, under linux, the environment
	variable __GL_SYNC_TO_VBLANK must be set to 1.

	If tearing is prevented in single buffer mode that is nice, but not
	required.

	Speeds higher than the vertical refresh rate are NOT wanted. For
	double buffered at least, if this runs faster it is a system error.
	This is also undesirable because a lot of time will be wasted
	drawing images the user does not see.

	Compilation instructions:

	Linux:
	cc -O2 fliptest.c -o fliptest -L/usr/X11R6/lib -lGL -lX11 -lXext

	Irix:
	cc -o fliptest fliptest.c -lGL -lX11 -lXext

	NT:
	cl -O2 fliptest.c OpenGL32.lib gdi32.lib user32.lib

*/

#include <iostream>
#include <sstream>

#ifdef _WIN32
	#include <windows.h>
#endif

#ifdef __APPLE__
	#include <OpenGL/OpenGL.h>
	//#include <GLUT/glut.h>
#else
	#include <GL/gl.h>
	//#include <GL/glut.h>
	#define GLX_GLXEXT_PROTOTYPES
	#include <GL/glx.h>
	#include <GL/glxext.h>
#endif

void make_window(unsigned int width, unsigned int height, bool double_buf);
void swap_buffers();
void enable_vsync();
void do_system_stuff();
double elapsed_time();
GLenum format = GL_RGBA;

#ifdef _WIN32
	/******************************************************************************/

	HWND window;
	HDC dc;
	HGLRC context;

	LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		switch(uMsg) {
			case WM_QUIT:
			case WM_CLOSE:
				std::exit(0);
		}
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	void make_window(unsigned int width, unsigned int height, bool double_buf) {
		char const * class_name = "YUCK";
	
		WNDCLASSEX wc;
		wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS;
		wc.lpfnWndProc = reinterpret_cast<WNDPROC>(WndProc);
		wc.cbClsExtra = wc.cbWndExtra = 0;
		wc.hInstance = GetModuleHandle(0);
		wc.hIcon = wc.hIconSm = LoadIcon(0, IDI_APPLICATION);
		wc.hCursor = LoadCursor(0, IDC_ARROW);
		wc.hbrBackground = 0;
		wc.lpszMenuName = 0;
		wc.lpszClassName = class_name;
		wc.cbSize = sizeof wc;
		RegisterClassEx(&wc);

		window = CreateWindow(
			class_name /* class */,
			"fliptest",
			WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_BORDER | WS_SYSMENU,
			CW_USEDEFAULT, CW_USEDEFAULT, width, height,
			0 /* parent */,
			0 /* menu */,
			GetModuleHandle(0),
			0 /* creation parameters */
		);
		ShowWindow(window, SW_SHOWNORMAL);
		dc = GetDC(window);
		int pixelFormat = 0;
		PIXELFORMATDESCRIPTOR chosen_pfd;
		for(int i = 1; ; ++i) {
			PIXELFORMATDESCRIPTOR pfd;
			if(!DescribePixelFormat(dc, i, sizeof pfd, &pfd)) break;
			// continue if it does not satisfy our requirements:
			if(~pfd.dwFlags & (PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL)) continue;
			if(pfd.iPixelType != 0) continue;
			if(!double_buf != !(pfd.dwFlags & PFD_DOUBLEBUFFER)) continue;
			// see if better than the one we have already:
			if(pixelFormat) {
				// offering overlay is better:
				if(!(chosen_pfd.bReserved & 15) && (pfd.bReserved & 15)) {}
				// otherwise more bit planes is better:
				else if(chosen_pfd.cColorBits < pfd.cColorBits) {}
				else continue;
			}
			pixelFormat = i;
			chosen_pfd = pfd;
		}
		if(!pixelFormat) {
			std::cerr <<  "ms-windows: no such visual\n";
			std::exit(1);
		}
		SetPixelFormat(dc, pixelFormat, &chosen_pfd);
		context = wglCreateContext(dc);
		wglMakeCurrent(dc, context);
	}

	void enable_vsync() {
		// WGL_EXT_swap_control
		//wglGetSwapIntervalEXT(1);
	}
	
	void swap_buffers() {
		SwapBuffers(dc);
	}

	void do_system_stuff() {
		MSG msg;
		while(PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	double elapsed_time(void) {
		static long int prevclock;
		long int newclock = GetTickCount();
		double elapsed = (newclock - prevclock) / 1000.0;
		prevclock = newclock;
		return elapsed;
	}

#elif __APPLE__
	/******************************************************************************/
	// TODO
	
	void make_window(unsigned int width, unsigned int height, bool double_buf) {
	}
	
	void enable_vsync() {
		// NSOpenGLCPSwapInterval
	}

	void swap_buffers() {
	}
	
	void do_system_stuff() {
	}
	
	double elapsed_time(void) {
	}
	
#elif __unix__ // X-window & unix version
	/******************************************************************************/

	#include <GL/glx.h>
	#include <sys/time.h>
	#ifdef __sgi
		#include <invent.h>
	#endif

	double elapsed_time(void) {
		static timeval prevclock;
		timeval newclock;
		gettimeofday(&newclock, 0);
		double elapsed = newclock.tv_sec - prevclock.tv_sec + (newclock.tv_usec - prevclock.tv_usec) / 1000000.0;
		prevclock.tv_sec = newclock.tv_sec;
		prevclock.tv_usec = newclock.tv_usec;
		return elapsed;
	}

	// color single buffer
	int clist[] = {
		GLX_RGBA,
		GLX_GREEN_SIZE, 1,
		None
	};

	// color double buffer
	int dlist[] = {
		GLX_RGBA,
		GLX_GREEN_SIZE, 1,
		GLX_DOUBLEBUFFER,
		None
	};

	Display * dpy;
	XVisualInfo * vis;
	Window window;

	// This creates the OpenGL window and makes the current OpenGL context draw into it.
	void make_window(unsigned int width, unsigned int height, bool double_buf) {
		dpy = XOpenDisplay(0);
		if(!dpy) {
			std::cerr << "x-window: could not open display\n";
			std::exit(1);
		}
		vis = glXChooseVisual(dpy, DefaultScreen(dpy), double_buf ? dlist : clist);
		if(!vis) {
			std::cerr << "x-window: no such visual\n";
			std::exit(1);
		}
		std::clog << "x-window: visual depth: " << vis->depth << '\n';

		XSetWindowAttributes attr;
		attr.border_pixel = 0;
		attr.colormap = XCreateColormap(dpy, RootWindow(dpy, vis->screen), vis->visual, AllocNone);
		attr.background_pixel = WhitePixel(dpy, DefaultScreen(dpy));
		window = XCreateWindow(
			dpy, RootWindow(dpy, DefaultScreen(dpy)),
			0, 0, width, height, 0,
			vis->depth, InputOutput, vis->visual,
			CWBorderPixel | CWColormap, &attr
		);
		XMapRaised(dpy, window);

		GLXContext context = glXCreateContext(dpy, vis, 0, 1);
		glXMakeCurrent(dpy, window, context);

		#if defined __sgi && defined GL_ABGR_EXT
			{ // figure out what pixel order is faster for the graphics card
				for(;;) {
					inventory_s * s = getinvent();
					if(!s) break;
					if(s->inv_class == INV_GRAPHICS) {
						if(s->inv_type < INV_MGRAS) format = GL_ABGR_EXT;
						break;
					}
				}
			}
		#endif
	}

	void enable_vsync() {
		#if defined GLX_MESA_swap_control
			//glXSwapIntervalMESA(1);
		#elif defined GLX_SGI_swap_control
			glXSwapIntervalSGI(1);
		#endif
	}

	void swap_buffers(void) {
		glXSwapBuffers(dpy, window);
	}

	void do_system_stuff(void) {
		// simulate something looking for X events
		XEvent event;
		XCheckWindowEvent(dpy, window, -1, &event);
	}

#else
	/******************************************************************************/
	#error unimplemented
#endif

void disable_for_draw_pixels() {
	// Disable stuff that's likely to slow down glDrawPixels.
	// (Omit as much of	this as possible, when you know in advance that the OpenGL state will already be set correctly.)
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_DITHER);
	glDisable(GL_FOG);
	glDisable(GL_LIGHTING);
	glDisable(GL_LOGIC_OP);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_TEXTURE_1D);
	glDisable(GL_TEXTURE_2D);
	glPixelTransferi(GL_MAP_COLOR, GL_FALSE);
	glPixelTransferi(GL_RED_SCALE, 1);
	glPixelTransferi(GL_RED_BIAS, 0);
	glPixelTransferi(GL_GREEN_SCALE, 1);
	glPixelTransferi(GL_GREEN_BIAS, 0);
	glPixelTransferi(GL_BLUE_SCALE, 1);
	glPixelTransferi(GL_BLUE_BIAS, 0);
	glPixelTransferi(GL_ALPHA_SCALE, 1);
	glPixelTransferi(GL_ALPHA_BIAS, 0);

	// Disable extensions that could slow down glDrawPixels.
	// (Actually, you should check for the presence of the proper extension before making these calls.
	// I've omitted that code for simplicity.)
	#ifdef GL_EXT_convolution
		glDisable(GL_CONVOLUTION_1D_EXT);
		glDisable(GL_CONVOLUTION_2D_EXT);
		glDisable(GL_SEPARABLE_2D_EXT);
	#endif
	#ifdef GL_EXT_histogram
		glDisable(GL_HISTOGRAM_EXT);
		glDisable(GL_MINMAX_EXT);
	#endif
	#ifdef GL_EXT_texture3D
		glDisable(GL_TEXTURE_3D_EXT);
	#endif
}

/******************************************************************************/

unsigned int const image_count = 25;

int main(int argc, char ** argv) {
	unsigned int width;
	unsigned int height;
	bool double_buf;

	if(argc > 1) { std::stringstream s; s << argv[1]; s >> width; }
	if(argc > 2) { std::stringstream s; s << argv[2]; s >> height; }
	if(argc > 3) { std::ostringstream s; s << argv[3]; double_buf = s.str()[0] == 'd'; }

	if(argc != 4 || width < 100 || height < 100) {
		std::cerr << "usage: " << argv[0] << " <width> <height> <d or s for double or single buffer>\n";
		std::exit(1);
	}
	
	std::clog <<
		"arg: width: " << width << "\n"
		"arg: height: " << height << "\n"
		"arg: double buffered: " << double_buf << '\n';

	unsigned int * const images = new unsigned int[width * height * image_count];
	{ // build the images
		unsigned int * p = images;
		for(unsigned int i = 0; i < image_count; ++i) {
			unsigned int bar = width * i / image_count;
			unsigned int barw = 3 * width / image_count;
			for(unsigned int y = 0; y < height; ++y) {
				for(unsigned int x = 0; x < width; ++x) {
					unsigned int const color = x >= bar && x <= bar + barw ? 0xffffffff : 0;
					*p++ = color;
				}
			}
		}
	}

	make_window(width, height, double_buf);

	std::cout <<
		"opengl: vendor: " << glGetString(GL_VENDOR) << "\n"
		"opengl: renderer: " << glGetString(GL_RENDERER) << "\n"
		"opengl: version: " << glGetString(GL_VERSION) << "\n"
		"opengl: extensions: " << glGetString(GL_EXTENSIONS) << '\n';
		
	// TODO glXQueryExtensionsString may provide more extensions

	if(double_buf) glDrawBuffer(GL_BACK);

	{ // set orthographic coordinate system with (0, 0) in lower-left corner
		GLint v[2];
		glGetIntegerv(GL_MAX_VIEWPORT_DIMS, v);
		int const dx = static_cast<int>(width) - v[0];
		int const dy = static_cast<int>(height) - v[1];
		glLoadIdentity();
		glViewport(dx, dy, v[0], v[1]);
		glOrtho(dx, width, dy, height, -1, 1);
	}

	elapsed_time();

	for(unsigned int i = 0; ; ++i) {
		do_system_stuff();
		unsigned int const frames = 1000;
		if(i % frames == frames - 1) {
			std::cout << "\rframes/s: " << frames / elapsed_time() << "    " << std::flush;
		}
		disable_for_draw_pixels();
		glRasterPos2i(0, 0);
		glDrawPixels(width, height, format, GL_UNSIGNED_BYTE, images + (i % image_count) * width * height);
		if(double_buf) swap_buffers(); else glFlush();
	}
	
	return 0;
}

