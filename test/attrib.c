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

#include <GLES/gl.h>

static SDL_Surface *screen;
static SDL_GLES_Context *context;

static bool fullscreen = false;

static const float vertices[] ={-0.6f, -0.6f, -0.5f, /* Square 1 */
								 0.3f, -0.6f, -0.5f,
						 		-0.6f,  0.8f,  0.1f,
								 0.3f,  0.8f,  0.1f,
								 0.3f,  0.8f,  0.1f, /* Degenerate vertices */
								-0.3f, -0.6f,  0.5f,
								-0.3f, -0.6f,  0.5f, /* Square 2 */
								 0.6f, -0.6f,  0.5f,
						 		-0.3f,  0.8f, -0.1f,
								 0.6f,  0.8f, -0.1f,
								};
static const float colors[] =  { 1.0f,  0.0f,  0.0f, 1.0f,
								 1.0f,  0.0f,  0.0f, 1.0f,
						 		 1.0f,  0.0f,  0.0f, 1.0f,
								 1.0f,  0.0f,  0.0f, 1.0f,
								 0.0f,  0.0f,  0.0f, 1.0f,
								 0.0f,  0.0f,  1.0f, 1.0f,
								 0.0f,  0.0f,  1.0f, 1.0f,
								 0.0f,  0.0f,  1.0f, 1.0f,
						 		 0.0f,  0.0f,  1.0f, 1.0f,
								 0.0f,  0.0f,  1.0f, 1.0f,
								};

static void draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 10);
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
	assert(res == 0);

	draw();
}

int main()
{
	int res, value = 0;
	res = SDL_Init(SDL_INIT_VIDEO);
	assert(res == 0);

	res = SDL_GLES_Init(SDL_GLES_VERSION_1_1);
	assert(res == 0);

	screen = SDL_SetVideoMode(0, 0, 16, SDL_SWSURFACE);
	assert(screen);

	SDL_WM_SetCaption("SDLgles attrib test", "SDLgles v1 test");
	SDL_ShowCursor(SDL_DISABLE);

#define ENABLE_DEPTH_BUFFER 1

#ifdef ENABLE_DEPTH_BUFFER
	/* With a depth buffer, the red square will be partially over the blue sq. */
	res = SDL_GLES_SetAttribute(SDL_GLES_DEPTH_SIZE, 4);
	assert(res == 0);

	res = SDL_GLES_GetAttribute(SDL_GLES_DEPTH_SIZE, &value);
	assert(res == 0);
	assert(value == 4);
	printf("Wanted depth buffer size is %d\n", value);
#else
	/* Without a depth buffer, the blue square will be over the red square. */
	printf("Wanted depth buffer size is %d\n", 0);
#endif

	context = SDL_GLES_CreateContext();
	assert(context);

	res = SDL_GLES_MakeCurrent(context);
	assert(res == 0);

	value = 0;
	res = SDL_GLES_GetAttribute(SDL_GLES_DEPTH_SIZE, &value);
	assert(res == 0);
	printf("Depth buffer size is %d\n", value);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0f, 0.2f, 0.0f, 0.0f);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, vertices);
	glColorPointer(4, GL_FLOAT, 0, colors);

	draw();

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

