CFLAGS:=-g -O0 -Wall

TEST_LDLIBS:=$(shell sdl-config --libs) -lSDL_gles -lEGL
TEST_CFLAGS:=$(shell sdl-config --cflags)

TEST_1_LDLIBS:=-lGLES_CM
TEST_2_LDLIBS:=-lGLESv2

TESTS_1:=gles1 attrib
TESTS_2:=gles2
TESTS:=$(TESTS_1) $(TESTS_2)

all: $(TESTS)

$(TESTS_1): %: %.o
	$(CC) $(LDFLAGS) $(TEST_LDFLAGS) $(LDLIBS) $(TEST_1_LDLIBS) $(TEST_LDLIBS) -o $@ $^
	
$(TESTS_2): %: %.o
	$(CC) $(LDFLAGS) $(TEST_LDFLAGS) $(LDLIBS) $(TEST_2_LDLIBS) $(TEST_LDLIBS) -o $@ $^
	
%.o: %.c
	$(CC) $(CFLAGS) $(TEST_CFLAGS) -c -o $@ $^
	
clean:
	rm -f *.o $(TESTS)

