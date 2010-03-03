/* gles1 - a simple SDL_gles OpenGL|ES 1.1 sample
 *
 * This file is in the public domain, furnished "as is", without technical
 * support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#include <SDL.h>
#include <SDL_gles.h>

#include <GLES2/gl2.h>

static SDL_Surface *screen;
static SDL_GLES_Context *context;

static GLuint program;
static bool fullscreen = false;

static GLuint compile_shader(GLenum type, const char *src)
{
	GLuint shader = glCreateShader(type);
	GLint compiled;

	assert(shader != 0),
	glShaderSource(shader, 1, &src, NULL);
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	assert(compiled);

	return shader;
}

static void init()
{
	const char vertex_shader_src[] =  
		"attribute vec4 pos;\n"
		"void main()\n"
		"{\n"
		"	gl_Position = pos;\n"
		"}\n";

	const char fragment_shader_src[] =  
		"precision mediump float;\n"
		"void main()\n"
		"{\n"
		"	gl_FragColor = vec4 ( 1.0, 0.0, 0.0, 1.0 );\n"
		"}\n";

	GLuint vertex_shader = compile_shader(GL_VERTEX_SHADER, vertex_shader_src);
	GLuint fragment_shader = compile_shader(GL_FRAGMENT_SHADER, fragment_shader_src);

	program = glCreateProgram();
	assert(program);

	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);

	glBindAttribLocation(program, 0, "pos");

	GLint linked;
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &linked);
	assert(linked);

	glClearColor(0.0f, 0.0f, 0.2f, 0.0f);
}

static void render()
{
	GLfloat tri_v[] = 	{0.0f,  0.5f, 0.0f,
						-0.5f, -0.5f, 0.0f,
						 0.5f, -0.5f, 0.0f};

	glViewport(0, 0, screen->w, screen->h);
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(program);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, tri_v);
	glEnableVertexAttribArray(0);

	glDrawArrays(GL_TRIANGLES, 0, 3);

	SDL_GLES_SwapBuffers();
}

static void toggle_fullscreen()
{
	int res;

	fullscreen = !fullscreen;

	screen = SDL_SetVideoMode(0, 0, 16, SDL_SWSURFACE |
		(fullscreen ? SDL_FULLSCREEN : 0));
	assert(screen);

	res = SDL_GLES_SetVideoMode();
	if (res != 0) puts(SDL_GetError());
	assert(res == 0);

	render();
}

int main()
{
	int res;
	res = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
	assert(res == 0);

	res = SDL_GLES_Init(SDL_GLES_VERSION_2_0);
	assert(res == 0);

	screen = SDL_SetVideoMode(0, 0, 16, SDL_SWSURFACE);
	assert(screen);

	SDL_WM_SetCaption("SDLgles v2 test", "SDLgles v2 test");
	SDL_ShowCursor(SDL_DISABLE);

	context = SDL_GLES_CreateContext();
	assert(context);

	res = SDL_GLES_MakeCurrent(context);
	assert(res == 0);

	init();
	render();

	SDL_Event event;
	while (SDL_WaitEvent(&event)) {
		switch (event.type) {
			case SDL_QUIT:
				goto quit;
			case SDL_MOUSEBUTTONDOWN:
				toggle_fullscreen();
				break;
		}
	}

quit:
	SDL_GLES_DeleteContext(context);

	SDL_GLES_Quit();
	SDL_Quit();

	return 0;
}
