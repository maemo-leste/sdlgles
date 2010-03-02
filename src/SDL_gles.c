/* This file is part of SDL_gles - SDL addon for OpenGL|ES
 * Copyright (C) 2010 Javier S. Pedro
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA or see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <assert.h>

#include <EGL/egl.h>

#include <SDL.h>
#include <SDL_syswm.h>

#include "SDL_gles.h"

typedef struct SDL_GLES_ContextPriv
{
	SDL_GLES_Context p;

	EGLConfig egl_config;
	EGLContext egl_context;
} SDL_GLES_ContextPriv;

static const char * default_libgl[] = {
	[SDL_GLES_VERSION_1_1] = "/usr/lib/libGLES_CM.so",
	[SDL_GLES_VERSION_2_0] = "/usr/lib/libGLESv2.so"
};

static SDL_GLES_Version gl_version = SDL_GLES_VERSION_NONE;
static void* gl_handle = NULL;
static EGLint egl_major, egl_minor;

static Display *display = NULL;
static EGLDisplay *egl_display = EGL_NO_DISPLAY;
static EGLSurface egl_surface = EGL_NO_SURFACE;
static EGLint attrib_list[] = {
	EGL_BUFFER_SIZE,			0,
	EGL_RED_SIZE,				0,
	EGL_GREEN_SIZE,				0,
	EGL_BLUE_SIZE,				0,
	EGL_ALPHA_SIZE,				0,
	EGL_CONFIG_CAVEAT,			EGL_DONT_CARE,
	EGL_CONFIG_ID,				EGL_DONT_CARE,
	EGL_DEPTH_SIZE,				0,
	EGL_LEVEL,					0,
	EGL_NATIVE_RENDERABLE,		EGL_DONT_CARE,
	EGL_NATIVE_VISUAL_TYPE,		EGL_DONT_CARE,
	EGL_SAMPLE_BUFFERS,			0,
	EGL_SAMPLES,				0,
	EGL_STENCIL_SIZE,			0,
	EGL_SURFACE_TYPE,			EGL_WINDOW_BIT,
	EGL_TRANSPARENT_TYPE,		EGL_NONE,
	EGL_TRANSPARENT_RED_VALUE,	EGL_DONT_CARE,
	EGL_TRANSPARENT_GREEN_VALUE,EGL_DONT_CARE,
	EGL_TRANSPARENT_BLUE_VALUE,	EGL_DONT_CARE,
	EGL_NONE
};
static SDL_GLES_ContextPriv *cur_context = NULL;

static const char * get_error_string(int error) {
	switch (error) {
		case EGL_SUCCESS:
			return "EGL_SUCCESS";
		case EGL_NOT_INITIALIZED:
			return "EGL_NOT_INITIALIZED";
		case EGL_BAD_ACCESS:
			return "EGL_BAD_ACCESS";
		case EGL_BAD_ALLOC:
			return "EGL_BAD_ALLOC";
		case EGL_BAD_ATTRIBUTE:
			return "EGL_BAD_ATTRIBUTE";
		case EGL_BAD_CONFIG:
			return "EGL_BAD_CONFIG";
		case EGL_BAD_CONTEXT:
			return "EGL_BAD_CONTEXT";
		case EGL_BAD_CURRENT_SURFACE:
			return "EGL_BAD_CURRENT_SURFACE";
		case EGL_BAD_DISPLAY:
			return "EGL_BAD_DISPLAY";
		case EGL_BAD_MATCH:
			return "EGL_BAD_MATCH";
		case EGL_BAD_NATIVE_PIXMAP:
			return "EGL_BAD_NATIVE_PIXMAP";
		case EGL_BAD_NATIVE_WINDOW:
			return "EGL_BAD_NATIVE_WINDOW";
		case EGL_BAD_PARAMETER:
			return "EGL_BAD_PARAMETER";
		case EGL_BAD_SURFACE:
			return "EGL_BAD_SURFACE";
		case EGL_CONTEXT_LOST:
			return "EGL_CONTEXT_LOST";
		default:
			return "EGL_UNKNOWN_ERROR";
    }
}

int SDL_GLES_LoadLibrary(const char *path)
{
	if (!path) {
		path = getenv("SDL_VIDEO_GL_DRIVER");
		if (!path) {
			switch (gl_version) {
				case SDL_GLES_VERSION_1_1:
				case SDL_GLES_VERSION_2_0:
					path = default_libgl[gl_version];
				break;
				default:
					SDL_SetError("No GL version specific and SDL_VIDEO_GL_DRIVER set");
					return -1;
			}
		}
	}

	/* Dynamically load the desired GL library */
	gl_handle = dlopen(path, RTLD_LAZY|RTLD_GLOBAL);
	if (!gl_handle) {
		SDL_SetError("Failed to open GL library: %s (%s)", path, dlerror());
		return -2;
	}

	return 0;
}

void* SDL_GLES_GetProcAddress(const char *proc)
{
	if (!gl_handle) return NULL;
	return dlsym(gl_handle, proc);
}

int SDL_GLES_Init(SDL_GLES_Version version)
{
	SDL_SysWMinfo info;
	EGLBoolean res;

	SDL_VERSION(&info.version);
	if (SDL_GetWMInfo(&info) != 1) {
		SDL_SetError("SDL_gles is incompatible with this SDL version");
		return -1;
	}

	display = info.info.x11.gfxdisplay;

	egl_display = eglGetDisplay((EGLNativeDisplayType)display);
	if (egl_display == EGL_NO_DISPLAY) {
		SDL_SetError("EGL found no available displays");
		return -2;
	}

	res = eglInitialize(egl_display, &egl_major, &egl_minor);
	if (!res) {
		SDL_SetError("EGL failed to initialize: %s",
			get_error_string(eglGetError()));
		return -2;
	}

	gl_version = version;
	return 0;
}

void SDL_GLES_Quit()
{
	if (gl_handle) {
		dlclose(gl_handle);
		gl_handle = NULL;
	}
	if (egl_display != EGL_NO_DISPLAY) {
		eglTerminate(egl_display);
		egl_display = EGL_NO_DISPLAY;
	}
}

int SDL_GLES_SetVideoMode()
{
	SDL_SysWMinfo info;

	SDL_VERSION(&info.version);
	if (SDL_GetWMInfo(&info) != 1) {
		SDL_SetError("SDL_gles is incompatible with this SDL version");
		return -1;
	}

	/* Destroy previous surface, if any. */
	if (egl_surface != EGL_NO_SURFACE) {
		eglDestroySurface(egl_display, egl_surface);
		egl_surface = EGL_NO_SURFACE;
	}

	/* No current context? Quietly defer surface creation. */
	if (!cur_context) {
		return 0;
	}

	egl_surface = eglCreateWindowSurface(egl_display, cur_context->egl_config,
		(EGLNativeWindowType)info.info.x11.window, NULL);
	if (egl_surface == EGL_NO_SURFACE) {
		SDL_SetError("EGL failed to create a window surface: %s",
			get_error_string(eglGetError()));
		return -2;
	}

	return 0;
}

SDL_GLES_Context* SDL_GLES_CreateContext(void)
{
	SDL_GLES_ContextPriv *context = malloc(sizeof(SDL_GLES_ContextPriv));
	if (!context) {
		SDL_Error(SDL_ENOMEM);
		return NULL;
	}

	EGLBoolean res;
	EGLConfig configs[1];
	EGLint num_config;

	res = eglChooseConfig(egl_display, attrib_list, configs, 1, &num_config);
	if (!res || num_config < 1) {
		SDL_SetError("EGL failed to find any valid config with required attributes: %s",
			get_error_string(eglGetError()));
		return NULL;
	}

	context->egl_config = configs[0];
	context->egl_context = eglCreateContext(egl_display, configs[0],
		EGL_NO_CONTEXT, NULL);

	return (SDL_GLES_Context*) context;
}

void SDL_GLES_DeleteContext(SDL_GLES_Context* c)
{
	SDL_GLES_ContextPriv *context = (SDL_GLES_ContextPriv*)c;
	if (!context) return;

	if (cur_context == context) {
		/* Deleting the active context */
		SDL_GLES_MakeCurrent(NULL);
	}

	eglDestroyContext(egl_display, context->egl_context);
	free(context);
}

int SDL_GLES_MakeCurrent(SDL_GLES_Context* c)
{
	SDL_GLES_ContextPriv *context = (SDL_GLES_ContextPriv*)c;
	int res;

	if (context) {
		cur_context = context;

		res = SDL_GLES_SetVideoMode();
		if (res != 0) return res; /* Surface (re-)creation failed. */

		res = eglMakeCurrent(egl_display, egl_surface, egl_surface,
			context->egl_context);

		if (!res) {
			SDL_SetError("EGL failed to make current: %s",
				get_error_string(eglGetError()));
			cur_context = NULL;
			return -2;
		}
	} else {
		eglMakeCurrent(egl_display,	EGL_NO_SURFACE, EGL_NO_SURFACE,
			EGL_NO_CONTEXT);
		cur_context = NULL;
		SDL_GLES_SetVideoMode(); /* Will clear current surface. */
	}

	// TODO Update attrib_list

	switch (gl_version) {
		case SDL_GLES_VERSION_1_1:
		case SDL_GLES_VERSION_2_0:
			res = eglBindAPI(EGL_OPENGL_ES_API);
			if (!res) {
				SDL_SetError("EGL failed to bind the required API: %s",
					get_error_string(eglGetError()));
				return -2;
			}
			break;
		default:
			break;
	}

	return 0;
}

void SDL_GLES_SwapBuffers()
{
	eglSwapBuffers(egl_display, egl_surface);
}

