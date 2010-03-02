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

static float box_step = 0.1f;

static const float w = 0.28f, h = 0.4f;
static float x = -1.0f, y = 0.0f;
static float box_v[4*3];

static Uint32 tick(Uint32 interval, void* param)
{
	SDL_Event e;
	e.type = SDL_VIDEOEXPOSE;

	x += box_step;
	if (x >= 1.0f || x <= -1.0f) {
		box_step *= -1.0f;
	}

	const float x1 = x - w/2, y1 = y - h/2;
	const float x2 = x + w/2, y2 = y + h/2;
	const float z = 0.5f;
	box_v[0] = x1; box_v[1] = y1;  box_v[2] = z;
	box_v[3] = x2; box_v[4] = y1;  box_v[5] = z;
	box_v[6] = x1; box_v[7] = y2;  box_v[8] = z;
	box_v[9] = x2; box_v[10] = y2; box_v[11] = z;

	SDL_PushEvent(&e);

	return interval;
}

int main()
{
	int res;
	res = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
	assert(res == 0);

	res = SDL_GLES_Init(SDL_GLES_VERSION_1_1);
	assert(res == 0);

	screen = SDL_SetVideoMode(0, 0, 16, SDL_SWSURFACE);
	assert(screen);

	SDL_TimerID timer = SDL_AddTimer(10, tick, NULL);
	assert(timer != NULL);

	context = SDL_GLES_CreateContext();
	assert(context);

	SDL_GLES_MakeCurrent(context);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glClearColor(0.0f, 0.0f, 1.0f, 0.0f);
	glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, box_v);

	SDL_Event event;
	while (SDL_WaitEvent(&event)) {
		switch (event.type) {
			case SDL_QUIT:
				goto quit;
			case SDL_VIDEOEXPOSE:
				glClear(GL_COLOR_BUFFER_BIT);
				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
				SDL_GLES_SwapBuffers();
				break;
		}
	}

quit:
	SDL_GLES_DeleteContext(context);

	SDL_GLES_Quit();
	SDL_Quit();

	return 0;
}
