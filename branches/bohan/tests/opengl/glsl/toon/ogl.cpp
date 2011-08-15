/*
	Simple Demo for GLSL 2.0
	www.lighthouse3d.com
*/

#define GL_GLEXT_PROTOTYPES
//#include <GL/glew.h>
#include <GL/glut.h>

#include <sstream>
#include <fstream>
#include <iostream>
#include <cstdlib> // for std::exit

void change_size(int w, int h) {
	// Prevent a divide by zero, when window is too short
	// (you cant make a window of zero width).
	if(h == 0) h = 1;

	// Reset the coordinate system before modifying
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	// Set the viewport to be the entire window
	glViewport(0, 0, w, h);

	// Set the correct perspective.
	gluPerspective(45, float(w) / h, 1, 1000);
	glMatrixMode(GL_MODELVIEW);
}


void render_scene() {
	float static light_pos[] = {1, 0.5, 1, 0};

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();
	gluLookAt(
		0.0, 0.0, 5.0, 
		0.0, 0.0, -1.0,
		0.0f, 1.0f, 0.0f
	);

	glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
	glutSolidTeapot(1);

	glutSwapBuffers();
}

void process_normal_keys(unsigned char key, int x, int y) {
	if(key == 27) std::exit(0);
}

void compile_shader(GLuint shader) {
	glCompileShader(shader);
	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if(status == GL_FALSE) {
		GLint len;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
		GLchar * s = new GLchar[len];
		try {
			glGetShaderInfoLog(shader, len, &len, s);
			std::cerr << "error compiling shader: " << shader << ":\n" << s << '\n' << std::flush;
		} catch(...) {
			delete[] s;
			throw;
		}
		delete[] s;
	}
}

GLuint compile_shader(GLenum type, std::string const & file_name) {
	std::string s;
	{ std::ostringstream ss;
		{ std::ifstream fs;
			fs.exceptions(std::ifstream::eofbit | std::ifstream::failbit | std::ifstream::badbit);
			fs.open(file_name.c_str(), std::ifstream::binary);
			ss << fs.rdbuf();
		}
		s = ss.str();
	}
	char const * c = s.c_str();
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &c, 0);
	compile_shader(shader);
	return shader;
}

void link_program(GLuint program) {
	glLinkProgram(program);
	GLint status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if(status == GL_FALSE) {
		GLint len;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
		GLchar * s = new GLchar[len];
		try {
			glGetProgramInfoLog(program, len, &len, s);
			std::cerr << "error linking program: " << program << ":\n" << s << '\n' << std::flush;
		} catch(...) {
			delete[] s;
			throw;
		}
		delete[] s;
	}
}

GLuint set_program() {
	GLuint p = glCreateProgram();
	glAttachShader(p, compile_shader(GL_VERTEX_SHADER, "toon.vert"));
	glAttachShader(p, compile_shader(GL_FRAGMENT_SHADER, "toon.frag"));
	glAttachShader(p, compile_shader(GL_FRAGMENT_SHADER, "toon2.frag"));
	link_program(p);
	glUseProgram(p);
	return p;
}

int main(int argc, char ** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(320, 320);
	glutCreateWindow("MM 2004-05");

	glutDisplayFunc(render_scene);
	//glutIdleFunc(render_scene);
	glutReshapeFunc(change_size);
	glutKeyboardFunc(process_normal_keys);

	glEnable(GL_DEPTH_TEST);
	glClearColor(1.0, 1.0, 1.0, 1.0);
	//glEnable(GL_CULL_FACE);

	#if 0
	glewInit();
	if(glewIsSupported("GL_VERSION_2_0"))
		std::cout << "OpenGL 2.0 supported\n";
	else {
		std::cerr << "OpenGL 2.0 not supported\n";
		std::exit(1);
	}
	#endif
	GLuint p = set_program();

	glutMainLoop();

	glDeleteProgram(p);

	return 0;
}

