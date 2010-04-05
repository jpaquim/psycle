/*!
  \file fliptest.c
  \author Nafees Bin Zafar <nafees@d2.com>

  $Id: fliptest.c,v 1.1.1.1 2003/02/20 08:51:07 nafees Exp $

  \brief Test of OpenGL glDrawPixels speed for use in flipbooks.

  This program simulates a flipbook viewer drawing a sequence of
  images out of memory. It reports the frames per second it achieves
  on stdout.  Real flipbook programs can be written to reliably get
  any speed up to the speed this program reports.

  To run it you provide it with three arguments: width, height, and
  'd' for doublebuffer or 's' for singlebuffer:

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

#include <stdio.h>
#include <stdlib.h>

/*======================================================================*/
#ifdef _WIN32

#include <windows.h>
#include <GL/gl.h>

double elapsed_time(void) {
  static long prevclock;
  long newclock = GetTickCount();
  double elapsed = (newclock-prevclock)/1000.0;
  prevclock = newclock;
  return elapsed;
}

HWND window;
HDC dc;
HGLRC context;

static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg) {
  case WM_QUIT:
  case WM_CLOSE:
    exit(0);
  }
  return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void make_window(int width, int height, int doublebuf) {
  int pixelFormat, i;
  PIXELFORMATDESCRIPTOR chosen_pfd;
  const char* class_name = "YUCK";
  WNDCLASSEX wc;
  wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS;
  wc.lpfnWndProc = (WNDPROC)WndProc;
  wc.cbClsExtra = wc.cbWndExtra = 0;
  wc.hInstance = GetModuleHandle(NULL);
  wc.hIcon = wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = NULL;
  wc.lpszMenuName = NULL;
  wc.lpszClassName = class_name;
  wc.cbSize = sizeof(WNDCLASSEX);
  RegisterClassEx(&wc);

  window = CreateWindow(class_name, /* class */
			"fliptest",
			WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_BORDER | WS_SYSMENU,
			CW_USEDEFAULT, CW_USEDEFAULT, width, height,
			NULL, /* parent */
			NULL, /* menu */
			GetModuleHandle(NULL),
			NULL /* creation parameters */
			);
  ShowWindow(window, SW_SHOWNORMAL);
  dc = GetDC(window);
  pixelFormat = 0;
  for (i = 1; ; i++) {
    PIXELFORMATDESCRIPTOR pfd;
    if (!DescribePixelFormat(dc, i, sizeof(pfd), &pfd)) break;
    // continue if it does not satisfy our requirements:
    if (~pfd.dwFlags & (PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL)) continue;
    if (pfd.iPixelType != 0) continue;
    if ((!doublebuf) != (!(pfd.dwFlags & PFD_DOUBLEBUFFER))) continue;
    // see if better than the one we have already:
    if (pixelFormat) {
      // offering overlay is better:
      if (!(chosen_pfd.bReserved & 15) && (pfd.bReserved & 15)) {}
      // otherwise more bit planes is better:
      else if (chosen_pfd.cColorBits < pfd.cColorBits) {}
      else continue;
    }
    pixelFormat = i;
    chosen_pfd = pfd;
  }
  if (!pixelFormat) {
    printf("No such visual\n");
    exit(1);
  }
  SetPixelFormat(dc, pixelFormat, &chosen_pfd);
  context = wglCreateContext(dc);
  wglMakeCurrent(dc, context);
}

void do_system_stuff(void) {
  MSG msg;
  while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
}

void swap_buffers(void) {
  SwapBuffers(dc);
}

const GLenum format = GL_RGBA;

/*======================================================================*/
#else /* Unix/X version */

#include <GL/gl.h>
#include <GL/glx.h>
#include <sys/time.h>
#ifdef __sgi
#include <invent.h>
#endif

double elapsed_time(void) {
  static struct timeval prevclock;
  struct timeval newclock;
  double elapsed;
  gettimeofday(&newclock, NULL);
  elapsed = newclock.tv_sec - prevclock.tv_sec +
    (newclock.tv_usec - prevclock.tv_usec)/1000000.0;
  prevclock.tv_sec = newclock.tv_sec;
  prevclock.tv_usec = newclock.tv_usec;
  return elapsed;
}

/* color singlebuffer */
int clist[] = {
    GLX_RGBA,
    GLX_GREEN_SIZE,1,
    None
};

/* color doublebuffer */
int dlist[] = {
    GLX_RGBA,
    GLX_GREEN_SIZE,1,
    GLX_DOUBLEBUFFER,
    None
};

Display *dpy;
XVisualInfo *vis;
Window window;
GLenum format = GL_RGBA;

/* This creates the OpenGL window and makes the current OpenGL context
   draw into it. */
Window make_window(int width, int height, int doublebuf) {
  XSetWindowAttributes attr;
  Colormap cmap;
  GLXContext context;

  dpy = XOpenDisplay(0);
  if (!dpy) {
    printf("Can't open display\n");
    exit(1);
  }
  vis =  glXChooseVisual(dpy,DefaultScreen(dpy),doublebuf ? dlist : clist);
  if (!vis) {
    printf("No such visual\n");
    exit(1);
  }
/*   printf("depth is %d\n",vis->depth); */

  cmap = XCreateColormap(dpy,RootWindow(dpy,vis->screen),vis->visual,AllocNone);
  attr.border_pixel = 0;
  attr.colormap = cmap;
  attr.background_pixel = WhitePixel(dpy,DefaultScreen(dpy));
  window = XCreateWindow(dpy, RootWindow(dpy,DefaultScreen(dpy)),
			 0, 0, width, height, 0,
			 vis->depth, InputOutput, vis->visual,
			 CWBorderPixel|CWColormap, &attr);
  XMapRaised(dpy,window);

  context = glXCreateContext(dpy, vis, 0, 1);
  glXMakeCurrent(dpy, window, context);

#if defined(__sgi) && defined(GL_ABGR_EXT)
  { /* figure out what pixel order is faster for the graphics card: */
    for (;;) {
      struct inventory_s* s = getinvent();
      if (!s) break;
      if (s->inv_class == INV_GRAPHICS) {
	if (s->inv_type < INV_MGRAS) format = GL_ABGR_EXT;
	break;
      }
    }
  }
#endif
  return window;
}

void do_system_stuff(void) {
  /* simulate something looking for X events */
  XEvent event;
  XCheckWindowEvent(dpy, window, -1, &event);
}

void swap_buffers(void) {
  glXSwapBuffers(dpy, window);
}

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

/*======================================================================*/

#define NUMIMAGES 25

int main(int argc, char **argv) {

  int width;
  int height;
  int doublebuf;
  int i;
  unsigned* images;

  if (argc > 1) width = atoi(argv[1]);
  if (argc > 2) height = atoi(argv[2]);
  if (argc > 3) doublebuf = argv[3][0] == 'd';

  if (argc != 4 || width < 100 || height < 100) {
    fprintf(stderr,"Usage: %s <width> <height> <d or s for double or single buffer>\n", argv[0]);
    exit(1);
  }

  {// build the images
    int i,x,y;
    unsigned * p;
    images = (unsigned*) malloc(width*height*NUMIMAGES*4);
    p = images;
    for (i = 0; i < NUMIMAGES; i++) {
		int bar = width * i / NUMIMAGES;
		int barw = 3 * width / NUMIMAGES;
		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				if (x >= bar && x <= bar+barw) *p++ = 0xffffffff;
				else *p++ = 0;
			}
		}
    }
  }

  make_window(width, height, doublebuf);

  printf( "GL Vendor = %s\n", glGetString(GL_VENDOR));
  printf( "GL Renderer = %s\n", glGetString(GL_RENDERER));
  printf( "GL Version = %s\n", glGetString(GL_VERSION));
  printf( "GL Extensions = %s\n", glGetString(GL_EXTENSIONS));
  
  if (doublebuf) glDrawBuffer(GL_BACK);

  { // set orthographic coordinate system with 0,0 in lower-left corner
    GLint v[2];
    glGetIntegerv(GL_MAX_VIEWPORT_DIMS, v);
    glLoadIdentity();
    glViewport(width-v[0], height-v[1], v[0], v[1]);
    glOrtho(width-v[0], width, height-v[1], height, -1, 1);
  }

  elapsed_time();

  for (i = 0; ; i++) {
    do_system_stuff();
    if ((i%100)==99) {
      printf("\rFPS = %g   ", 100.0/elapsed_time());
      fflush(stdout);
    }
    disable_for_draw_pixels();
    glRasterPos2i(0,0);
    glDrawPixels(width, height, format, GL_UNSIGNED_BYTE, images + (i % NUMIMAGES) * width * height);
    if(doublebuf) swap_buffers(); else glFlush();
  }

  return 0;
}

