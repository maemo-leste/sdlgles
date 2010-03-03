 /* Created by exoticorn ( http://talk.maemo.org/showthread.php?t=37356 )
  * edited and commented by André Bergner [endboss]
  * edited by Javier for SDL_gles
  *
  * librariries needed:   libgles2-dev, libsdl-gles1.2-dev, libsdl1.2-dev
  *
  * compile with:   g++ -O2 -o simplegl `sdl-config --cflags --libs` -lSDL_gles -lGLESv2  simplegl.cc
  */

#include  <iostream>
using namespace std;

#include <SDL.h>
#include <SDL_gles.h>

#include  <cmath>
#include  <sys/time.h>

#include  <GLES2/gl2.h>

static const char vertex_src[] = "                                        \
    attribute vec4        position;       \
    varying mediump vec2  pos;            \
    uniform vec4          offset;         \
                                          \
    void main()                           \
    {                                     \
       gl_Position = position + offset;   \
       pos = position.xy;                 \
    }                                     \
 ";


static const char fragment_src[] =
	"                                                      \
    varying mediump vec2    pos;                        \
    uniform mediump float   phase;                      \
                                                        \
    void  main()                                        \
    {                                                   \
       gl_FragColor  =  vec4( 1., 0.9, 0.7, 1.0 ) *     \
         cos( 30.*sqrt(pos.x*pos.x + 1.5*pos.y*pos.y)   \
              + atan(pos.y,pos.x) - phase );            \
    }                                                   \
 ";

static void print_shader_info_log(GLuint shader	// handle to the shader
	)
{
	GLint length, success;

	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
	if (length) {
		char *buffer = new char[length];
		glGetShaderInfoLog(shader, length, NULL, buffer);
		cout << "shader info: " << buffer << flush;
		delete[]buffer;
	}

	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (success != GL_TRUE) {
		cerr << "failed to build shader" << endl;
		exit(1);
	}
}

static GLuint load_shader(const char *shader_source, GLenum type)
{
	GLuint shader = glCreateShader(type);

	glShaderSource(shader, 1, &shader_source, NULL);
	glCompileShader(shader);

	print_shader_info_log(shader);

	return shader;
}

static SDL_Surface *screen;
static SDL_GLES_Context *context;

GLfloat norm_x = 0.0, norm_y = 0.0,
	offset_x = 0.0, offset_y = 0.0, p1_pos_x = 0.0, p1_pos_y = 0.0;

GLint phase_loc, offset_loc, position_loc;

bool update_pos = false;

const float vertexArray[] = {
	0.0, 0.5, 0.0,
	-0.5, 0.0, 0.0,
	0.0, -0.5, 0.0,
	0.5, 0.0, 0.0,
	0.0, 0.5, 0.0
};


void render()
{
	static float phase = 0;

	//// draw
	glClear(GL_COLOR_BUFFER_BIT);

	glUniform1f(phase_loc, phase);	// write the value of phase to the shaders phase
	phase = fmodf(phase + 0.5f, 2.f * 3.141f);	// and update the local variable

	if (update_pos) {			// if the position of the texture has changed due to user action
		GLfloat old_offset_x = offset_x;
		GLfloat old_offset_y = offset_y;

		offset_x = norm_x - p1_pos_x;
		offset_y = norm_y - p1_pos_y;

		p1_pos_x = norm_x;
		p1_pos_y = norm_y;

		offset_x += old_offset_x;
		offset_y += old_offset_y;

		update_pos = false;
	}

	glUniform4f(offset_loc, offset_x, offset_y, 0.0, 0.0);

	glVertexAttribPointer(position_loc, 3, GL_FLOAT, false, 0,
						  vertexArray);
	glEnableVertexAttribArray(position_loc);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 5);

	SDL_GLES_SwapBuffers();
}


int main()
{
	SDL_Init(SDL_INIT_VIDEO);
	SDL_GLES_Init(SDL_GLES_VERSION_2_0);
	screen = SDL_SetVideoMode(0, 0, 16, SDL_SWSURFACE | SDL_FULLSCREEN);
	SDL_WM_SetCaption("SimpleGL", "SimpleGL");
	SDL_ShowCursor(SDL_DISABLE);
	context = SDL_GLES_CreateContext();
	SDL_GLES_MakeCurrent(context);

	glClearColor(0.08, 0.06, 0.07, 1.0);	// background color

	GLuint vertexShader = load_shader(vertex_src, GL_VERTEX_SHADER);	// load vertex shader
	GLuint fragmentShader = load_shader(fragment_src, GL_FRAGMENT_SHADER);	// load fragment shader

	GLuint shaderProgram = glCreateProgram();	// create program object
	glAttachShader(shaderProgram, vertexShader);	// and attach both...
	glAttachShader(shaderProgram, fragmentShader);	// ... shaders to it

	glLinkProgram(shaderProgram);	// link the program
	glUseProgram(shaderProgram);	// and select it for usage

	//// now get the locations (kind of handle) of the shaders variables
	position_loc = glGetAttribLocation(shaderProgram, "position");
	phase_loc = glGetUniformLocation(shaderProgram, "phase");
	offset_loc = glGetUniformLocation(shaderProgram, "offset");
	if (position_loc < 0 || phase_loc < 0 || offset_loc < 0) {
		cerr << "Unable to get uniform location" << endl;
		return 1;
	}

	const float window_width = screen->w, window_height = screen->h;

	//// this is needed for time measuring  -->  frames per second
	struct timezone tz;
	timeval t1, t2;
	gettimeofday(&t1, &tz);
	int num_frames = 0;

	bool quit = false;
	while (!quit) {				// the main loop
		SDL_Event e;
		GLfloat window_y, window_x;
		while (SDL_PollEvent(&e)) {	// check for events
			switch(e.type) {
			case SDL_MOUSEMOTION: // if mouse has moved
				window_y = (window_height - e.motion.y) - window_height / 2.0;
				norm_y = window_y / (window_height / 2.0);
				window_x = e.motion.x - window_width / 2.0;
				norm_x = window_x / (window_width / 2.0);
				update_pos = true;
				break;
			case SDL_KEYDOWN:
			case SDL_QUIT:
				quit = true;
				break;
			}
		}

		render();				// now we finally put something on the screen

		if (++num_frames % 100 == 0) { // update framerate counter
			gettimeofday(&t2, &tz);
			float dt =
				t2.tv_sec - t1.tv_sec + (t2.tv_usec - t1.tv_usec) * 1e-6;
			cout << "fps: " << num_frames / dt << endl;
			num_frames = 0;
			t1 = t2;
		}
	}

	////  cleaning up...
	SDL_GLES_Quit();
	SDL_Quit();

	return 0;
}

