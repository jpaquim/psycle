// -*- C++ -*-
/*
 * pixmap.cc:
 * Simple off-screen OpenGL rendering example.
 *
 * written by Naofumi Yasufuku  <naofumi@users.sourceforge.net>
 */

#include <iostream>
#include <cstdlib>

#include <gtkmm.h>

#include <gtkglmm.h>

#ifdef G_OS_WIN32
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>


///////////////////////////////////////////////////////////////////////////////
//
// OpenGL frame buffer configuration utilities.
//
///////////////////////////////////////////////////////////////////////////////

struct GLConfigUtil
{
  static void print_gl_attrib(const Glib::RefPtr<const Gdk::GL::Config>& glconfig,
                              const char* attrib_str,
                              int attrib,
                              bool is_boolean);

  static void examine_gl_attrib(const Glib::RefPtr<const Gdk::GL::Config>& glconfig);
};

//
// Print a configuration attribute.
//
void GLConfigUtil::print_gl_attrib(const Glib::RefPtr<const Gdk::GL::Config>& glconfig,
                                   const char* attrib_str,
                                   int attrib,
                                   bool is_boolean)
{
  int value;

  if (glconfig->get_attrib(attrib, value))
    {
      std::cout << attrib_str << " = ";
      if (is_boolean)
        std::cout << (value == true ? "true" : "false") << std::endl;
      else
        std::cout << value << std::endl;
    }
  else
    {
      std::cout << "*** Cannot get "
                << attrib_str
                << " attribute value\n";
    }
}

//
// Print configuration attributes.
//
void GLConfigUtil::examine_gl_attrib(const Glib::RefPtr<const Gdk::GL::Config>& glconfig)
{
  std::cout << "\nOpenGL visual configurations :\n\n";

  std::cout << "glconfig->is_rgba() = "
            << (glconfig->is_rgba() ? "true" : "false")
            << std::endl;
  std::cout << "glconfig->is_double_buffered() = "
            << (glconfig->is_double_buffered() ? "true" : "false")
            << std::endl;
  std::cout << "glconfig->is_stereo() = "
            << (glconfig->is_stereo() ? "true" : "false")
            << std::endl;
  std::cout << "glconfig->has_alpha() = "
            << (glconfig->has_alpha() ? "true" : "false")
            << std::endl;
  std::cout << "glconfig->has_depth_buffer() = "
            << (glconfig->has_depth_buffer() ? "true" : "false")
            << std::endl;
  std::cout << "glconfig->has_stencil_buffer() = "
            << (glconfig->has_stencil_buffer() ? "true" : "false")
            << std::endl;
  std::cout << "glconfig->has_accum_buffer() = "
            << (glconfig->has_accum_buffer() ? "true" : "false")
            << std::endl;

  std::cout << std::endl;

  print_gl_attrib(glconfig, "Gdk::GL::USE_GL",           Gdk::GL::USE_GL,           true);
  print_gl_attrib(glconfig, "Gdk::GL::BUFFER_SIZE",      Gdk::GL::BUFFER_SIZE,      false);
  print_gl_attrib(glconfig, "Gdk::GL::LEVEL",            Gdk::GL::LEVEL,            false);
  print_gl_attrib(glconfig, "Gdk::GL::RGBA",             Gdk::GL::RGBA,             true);
  print_gl_attrib(glconfig, "Gdk::GL::DOUBLEBUFFER",     Gdk::GL::DOUBLEBUFFER,     true);
  print_gl_attrib(glconfig, "Gdk::GL::STEREO",           Gdk::GL::STEREO,           true);
  print_gl_attrib(glconfig, "Gdk::GL::AUX_BUFFERS",      Gdk::GL::AUX_BUFFERS,      false);
  print_gl_attrib(glconfig, "Gdk::GL::RED_SIZE",         Gdk::GL::RED_SIZE,         false);
  print_gl_attrib(glconfig, "Gdk::GL::GREEN_SIZE",       Gdk::GL::GREEN_SIZE,       false);
  print_gl_attrib(glconfig, "Gdk::GL::BLUE_SIZE",        Gdk::GL::BLUE_SIZE,        false);
  print_gl_attrib(glconfig, "Gdk::GL::ALPHA_SIZE",       Gdk::GL::ALPHA_SIZE,       false);
  print_gl_attrib(glconfig, "Gdk::GL::DEPTH_SIZE",       Gdk::GL::DEPTH_SIZE,       false);
  print_gl_attrib(glconfig, "Gdk::GL::STENCIL_SIZE",     Gdk::GL::STENCIL_SIZE,     false);
  print_gl_attrib(glconfig, "Gdk::GL::ACCUM_RED_SIZE",   Gdk::GL::ACCUM_RED_SIZE,   false);
  print_gl_attrib(glconfig, "Gdk::GL::ACCUM_GREEN_SIZE", Gdk::GL::ACCUM_GREEN_SIZE, false);
  print_gl_attrib(glconfig, "Gdk::GL::ACCUM_BLUE_SIZE",  Gdk::GL::ACCUM_BLUE_SIZE,  false);
  print_gl_attrib(glconfig, "Gdk::GL::ACCUM_ALPHA_SIZE", Gdk::GL::ACCUM_ALPHA_SIZE, false);

  std::cout << std::endl;
}


///////////////////////////////////////////////////////////////////////////////
//
// Simple OpenGL scene using GL::Pixmap.
//
///////////////////////////////////////////////////////////////////////////////

class PixmapGLScene : public Gtk::DrawingArea
{
public:
  PixmapGLScene();
  virtual ~PixmapGLScene();

protected:
  // init OpenGL context
  void init_gl();

protected:
  virtual bool on_configure_event(GdkEventConfigure* event);
  virtual bool on_expose_event(GdkEventExpose* event);

protected:
  // OpenGL rendering stuff:
  Glib::RefPtr<Gdk::GL::Config> m_GLConfig;
  Glib::RefPtr<Gdk::GL::Context> m_GLContext;
  Glib::RefPtr<Gdk::Pixmap> m_Pixmap;
};

PixmapGLScene::PixmapGLScene()
  : m_GLConfig(0), m_GLContext(0), m_Pixmap(0)
{
  //
  // Configure OpenGL-capable visual.
  //

  // Try single-buffered visual
  m_GLConfig = Gdk::GL::Config::create(Gdk::GL::MODE_RGB    |
                                       Gdk::GL::MODE_DEPTH  |
                                       Gdk::GL::MODE_SINGLE);
  if (!m_GLConfig)
    {
      std::cerr << "*** Cannot find any OpenGL-capable visual.\n";
      std::exit(1);
    }

  // print frame buffer attributes.
  GLConfigUtil::examine_gl_attrib(m_GLConfig);

  //
  // Set OpenGL-capable colormap.
  //

  set_colormap(m_GLConfig->get_colormap());
}

PixmapGLScene::~PixmapGLScene()
{
}

void PixmapGLScene::init_gl()
{
  GLUquadricObj* qobj = gluNewQuadric();
  gluQuadricDrawStyle(qobj, GLU_FILL);
  glNewList(1, GL_COMPILE);
  gluSphere(qobj, 1.0, 20, 20);
  glEndList();

  static GLfloat light_diffuse[] = {1.0, 0.0, 0.0, 1.0};
  static GLfloat light_position[] = {1.0, 1.0, 1.0, 0.0};
  glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
  glLightfv(GL_LIGHT0, GL_POSITION, light_position);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_DEPTH_TEST);

  glClearColor(1.0, 1.0, 1.0, 1.0);
  glClearDepth(1.0);

  glViewport(0, 0, get_width(), get_height());

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(40.0, 1.0, 1.0, 10.0);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluLookAt(0.0, 0.0, 3.0,
            0.0, 0.0, 0.0,
            0.0, 1.0, 0.0);
  glTranslatef(0.0, 0.0, -3.0);
}

bool PixmapGLScene::on_configure_event(GdkEventConfigure* event)
{
  //
  // Create an OpenGL off-screen rendering area.
  //

  m_Pixmap = Gdk::Pixmap::create(get_window(),
                                 get_width(), get_height(),
                                 m_GLConfig->get_depth());

  //
  // Set OpenGL-capability to the pixmap (invoke extension method).
  //

  Glib::RefPtr<Gdk::GL::Pixmap> glpixmap =
    Gdk::GL::ext(m_Pixmap).set_gl_capability(m_GLConfig);

  //
  // Create OpenGL rendering context (not direct).
  //

  if (!m_GLContext)
    m_GLContext = Gdk::GL::Context::create(glpixmap, false);

  //
  // GL calls.
  //

  // *** OpenGL BEGIN ***
  if (!glpixmap->gl_begin(m_GLContext))
    return false;

  static bool is_initialized = false;
  if (!is_initialized)
    {
      init_gl();
      is_initialized = true;
    }

  glViewport(0, 0, get_width(), get_height());

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glCallList(1);

  glFlush();

  glpixmap->gl_end();
  // *** OpenGL END ***

  return true;
}

bool PixmapGLScene::on_expose_event(GdkEventExpose* event)
{
  if (!m_Pixmap)
    return false;

  get_window()->draw_drawable(get_style()->get_fg_gc(get_state()),
                              m_Pixmap,
                              event->area.x, event->area.y,
                              event->area.x, event->area.y,
                              event->area.width, event->area.height);

  return true;
}


///////////////////////////////////////////////////////////////////////////////
//
// The application class.
//
///////////////////////////////////////////////////////////////////////////////

class Pixmap : public Gtk::Window
{
public:
  Pixmap();
  virtual ~Pixmap();

protected:
  // signal handlers:
  void on_button_quit_clicked();

protected:
  // member widgets:
  Gtk::VBox m_VBox;
  PixmapGLScene m_PixmapGLScene;
  Gtk::Button m_ButtonQuit;
};

Pixmap::Pixmap()
  : m_VBox(false, 0), m_ButtonQuit("Quit")
{
  //
  // Top-level window.
  //

  set_title("Pixmap");

  add(m_VBox);

  //
  // Simple OpenGL scene using GL::Pixmap.
  //

  m_PixmapGLScene.set_size_request(200, 200);

  m_VBox.pack_start(m_PixmapGLScene);

  //
  // Simple quit button.
  //

  m_ButtonQuit.signal_clicked().connect(
    sigc::mem_fun(*this, &Pixmap::on_button_quit_clicked));

  m_VBox.pack_start(m_ButtonQuit, Gtk::PACK_SHRINK, 0);

  //
  // Show window.
  //

  show_all();
}

Pixmap::~Pixmap()
{}

void Pixmap::on_button_quit_clicked()
{
  Gtk::Main::quit();
}


///////////////////////////////////////////////////////////////////////////////
//
// Main.
//
///////////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
  Gtk::Main kit(argc, argv);

  //
  // Init gtkglextmm.
  //

  Gtk::GL::init(argc, argv);

  //
  // Query OpenGL extension version.
  //

  int major, minor;
  Gdk::GL::query_version(major, minor);
  std::cout << "OpenGL extension version - "
            << major << "." << minor << std::endl;

  //
  // Instantiate and run the application.
  //

  Pixmap pixmap;

  kit.run(pixmap);

  return 0;
}
