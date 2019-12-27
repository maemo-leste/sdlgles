PREFIX:=$(shell sdl-config --prefix)
CFLAGS:=-g -O2 -Wall -Wextra

CC:=gcc
LIBTOOL=libtool

RELEASE:=1.2
VERSION:=0:1:0

SDL_GLES_TARGET:=libSDL_gles.la

SDL_GLES_LDLIBS:=$(shell sdl-config --libs) -lEGL
SDL_GLES_CFLAGS:=$(shell sdl-config --cflags)
SDL_GLES_LDFLAGS:=-release $(RELEASE) -version-info $(VERSION) -rpath $(PREFIX)/lib

all: $(SDL_GLES_TARGET)

$(SDL_GLES_TARGET): SDL_gles.lo
	$(LIBTOOL) --mode=link $(CC) $(LDFLAGS) $(SDL_GLES_LDFLAGS) $(LDLIBS) $(SDL_GLES_LDLIBS) -o $@ $^
	
SDL_gles.lo: SDL_gles.c
	$(LIBTOOL) --mode=compile $(CC) $(CFLAGS) $(SDL_GLES_CFLAGS) -c $^
	
clean:
	$(LIBTOOL) --mode=clean rm -f *.o *.lo $(SDL_GLES_TARGET)
	
install: $(SDL_GLES_TARGET)
	install -d $(DESTDIR)$(PREFIX)/include/SDL $(DESTDIR)$(PREFIX)/lib
	install SDL_gles.h $(DESTDIR)$(PREFIX)/include/SDL/
	$(LIBTOOL) --mode=install install -c $(SDL_GLES_TARGET) $(DESTDIR)$(PREFIX)/lib/
ifeq ($(DESTDIR),)
	$(LIBTOOL) --mode=finish $(PREFIX)/lib
endif

uninstall:
	-$(LIBTOOL) --mode=uninstall rm $(DESTDIR)$(PREFIX)/lib/$(SDL_GLES_TARGET)
	-rm $(DESTDIR)$(PREFIX)/include/SDL/SDL_gles.h

