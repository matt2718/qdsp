CC=gcc
CFLAGS=-std=gnu99 -fPIC
LDFLAGS=-shared
LDLIBS=-lglfw
EXAMPLE_CFLAGS=-std=gnu99 -fopenmp

SOURCES=qdsp.c glad/glad.c
SHADERS=vertex.glsl fragment.glsl overlay-vertex.glsl overlay-fragment.glsl
OBJECTS=$(SOURCES:.c=.o)

INSTPREFIX=/usr/local

.PHONY: all
all: libqdsp.so

.PHONY: debug
debug: libqdsp.so example
debug: CFLAGS += -g -O0
debug: EXAMPLE_CFLAGS += -g -O0

.PHONY: clean
clean:
	rm -f libqdsp.so $(OBJECTS)
	rm -f example

.PHONY: install
install: libqdsp.so qdsp.h
	mkdir -p $(INSTPREFIX)/share/qdsp
	cp libqdsp.so $(INSTPREFIX)/lib
	cp qdsp.h $(INSTPREFIX)/include
	cp $(SHADERS) $(INSTPREFIX)/share/qdsp
	@echo "Installed successfully. You may need to run ldconfig."

.PHONY: uninstall
uninstall:
	rm -rf $(INSTPREFIX)/share/qdsp
	rm -f $(INSTPREFIX)/lib/libqdsp.so
	rm -f $(INSTPREFIX)/include/qdsp.h

# actual rules and dependencies here:

libqdsp.so: $(OBJECTS)
	$(CC) -o libqdsp.so $(CFLAGS) $(LDFLAGS) $(OBJECTS) $(LDLIBS)

.c.o:
	$(CC) -o $@ -c $(CFLAGS) $<

example: example.c libqdsp.so
	$(CC) -o example $(EXAMPLE_CFLAGS) example.c libqdsp.so -lm -lfftw3 -Lqdsp -Wl,-R.

qdsp.o: qdsp.h glad/glad.h

glad/glad.o: glad/glad.h glad/KHR/khrplatform.h
