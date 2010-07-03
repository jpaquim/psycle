// -*- C++ -*-
/*
 * font.cc:
 * Simple bitmap font rendering example.
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
// Simple OpenGL scene using bitmap font.
//
///////////////////////////////////////////////////////////////////////////////

class FontGLScene : public Gtk::DrawingArea,
                    public Gtk::GL::Widget<FontGLScene>
{
public:
  FontGLScene();
  virtual ~FontGLScene();

protected:
  virtual void on_realize();
  virtual bool on_configure_event(GdkEventConfigure* event);
  virtual bool on_expose_event(GdkEventExpose* event);

protected:
  // font rendering stuff:
  static const Glib::ustring m_FontString;
  GLuint m_FontListBase;
  int m_FontHeight;
};

const Glib::ustring FontGLScene::m_FontString = "courier 12";

FontGLScene::FontGLScene()
  : m_FontListBase(0), m_FontHeight(0)
{
  //
  // Configure OpenGL-capable visual.
  //

  Glib::RefPtr<Gdk::GL::Config> glconfig;

  // Try double-buffered visual
  glconfig = Gdk::GL::Config::create(Gdk::GL::MODE_RGB    |
                                     Gdk::GL::MODE_DEPTH  |
                                     Gdk::GL::MODE_DOUBLE);
  if (!glconfig)
    {
      std::cerr << "*** Cannot find the double-buffered visual.\n"
                << "*** Trying single-buffered visual.\n";

      // Try single-buffered visual
      glconfig = Gdk::GL::Config::create(Gdk::GL::MODE_RGB   |
                                         Gdk::GL::MODE_DEPTH);
      if (!glconfig)
        {
          std::cerr << "*** Cannot find any OpenGL-capable visual.\n";
          std::exit(1);
        }
    }

  // print frame buffer attributes.
  GLConfigUtil::examine_gl_attrib(glconfig);

  //
  // Set OpenGL-capability to the widget.
  //

  set_gl_capability(glconfig);
}

FontGLScene::~FontGLScene()
{
}

void FontGLScene::on_realize()
{
  // We need to call the base on_realize()
  Gtk::DrawingArea::on_realize();

  //
  // Get GL::Window.
  //

  Glib::RefPtr<Gdk::GL::Window> glwindow = get_gl_window();

  //
  // GL calls.
  //

  // *** OpenGL BEGIN ***
  if (!glwindow->gl_begin(get_gl_context()))
    return;

  //
  // Generate font display lists.
  //

  m_FontListBase = glGenLists (128);

  Pango::FontDescription font_desc(m_FontString);

  Glib::RefPtr<Pango::Font> font =
    Gdk::GL::Font::use_pango_font(font_desc, 0, 128, m_FontListBase);
  if (!font)
    {
      std::cerr << "*** Can't load font "
                << m_FontString
                << std::endl;
      Gtk::Main::quit();
    }

  Pango::FontMetrics font_metrics = font->get_metrics();

  m_FontHeight = font_metrics.get_ascent() + font_metrics.get_descent();
  m_FontHeight = PANGO_PIXELS(m_FontHeight);

  glClearColor(1.0, 1.0, 1.0, 1.0);
  glClearDepth(1.0);

  glViewport(0, 0, get_width(), get_height());

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0.0, get_width(),
          0.0, get_height(),
          -1.0, 1.0);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glwindow->gl_end();
  // *** OpenGL END ***
}

bool FontGLScene::on_configure_event(GdkEventConfigure* event)
{
  //
  // Get GL::Window.
  //

  Glib::RefPtr<Gdk::GL::Window> glwindow = get_gl_window();

  //
  // GL calls.
  //

  // *** OpenGL BEGIN ***
  if (!glwindow->gl_begin(get_gl_context()))
    return false;

  glViewport(0, 0, get_width(), get_height());

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0.0, get_width(),
          0.0, get_height(),
          -1.0, 1.0);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glwindow->gl_end();
  // *** OpenGL END ***

  return true;
}

bool FontGLScene::on_expose_event(GdkEventExpose* event)
{
  //
  // Get GL::Window.
  //

  Glib::RefPtr<Gdk::GL::Window> glwindow = get_gl_window();

  //
  // GL calls.
  //

  // *** OpenGL BEGIN ***
  if (!glwindow->gl_begin(get_gl_context()))
    return false;

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  //
  // Draw some text.
  //
  glColor3f(0.0, 0.0, 0.0);
  for (int i = 2; i >= -2; --i)
    {
      glRasterPos2f(10.0, 0.5*get_height() + i*m_FontHeight);
      for (int j = ' '; j <= 'Z'; ++j)
        glCallList(m_FontListBase+j);
    }

  //
  // Show font description string.
  //
  glColor3f(1.0, 0.0, 0.0);
  glRasterPos2f(10.0, 10.0);
  glListBase(m_FontListBase);
  glCallLists(m_FontString.length(), GL_UNSIGNED_BYTE, m_FontString.c_str());

  // Swap buffers.
  if (glwindow->is_double_buffered())
    glwindow->swap_buffers();
  else
    glFlush();

  glwindow->gl_end();
  // *** OpenGL END ***

  return true;
}


///////////////////////////////////////////////////////////////////////////////
//
// The application class.
//
///////////////////////////////////////////////////////////////////////////////

class Font : public Gtk::Window
{
public:
  Font();
  virtual ~Font();

protected:
  // signal handlers:
  void on_button_quit_clicked();

protected:
  // member widgets:
  Gtk::VBox m_VBox;
  FontGLScene m_FontGLScene;
  Gtk::Button m_ButtonQuit;

};

Font::Font()
  : m_VBox(false, 0), m_ButtonQuit("Quit")
{
  //
  // Top-level window.
  //

  set_title("Font");

  // Get automatically redrawn if any of their children changed allocation.
  set_reallocate_redraws(true);

  add(m_VBox);

  //
  // Simple OpenGL scene using bitmap font.
  //

  m_FontGLScene.set_size_request(640, 240);

  m_VBox.pack_start(m_FontGLScene);

  //
  // Simple quit button.
  //

  m_ButtonQuit.signal_clicked().connect(
    sigc::mem_fun(*this, &Font::on_button_quit_clicked));

  m_VBox.pack_start(m_ButtonQuit, Gtk::PACK_SHRINK, 0);

  //
  // Show window.
  //

  show_all();
}

Font::~Font()
{}

void Font::on_button_quit_clicked()
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

  Font font;

  kit.run(font);

  return 0;
}
