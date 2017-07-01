CC=gcc
CFLAGS=-std=gnu99 -fPIC
LDFLAGS=-shared
LDLIBS=-lglfw
EXAMPLE_CFLAGS=-std=gnu99 -fopenmp

SOURCES=qdsp.c glad/glad.c
SHADERS=vertex.glsl fragment.glsl
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
	rm -f libqdsp.so $(OBJECTS) shaders.h

.PHONY: install
install: libqdsp.so qdsp.h
	mkdir -p $(INSTPREFIX)/share/qdsp/
	cp libqdsp.so $(INSTPREFIX)/lib/
	cp qdsp.h $(INSTPREFIX)/include/
	cp fragment.glsl vertex.glsl $(INSTPREFIX)/share/qdsp/
	@echo "Installed successfully. You may need to run ldconfig."

libqdsp.so: $(OBJECTS)
	$(CC) -o libqdsp.so $(CFLAGS) $(LDFLAGS) $(OBJECTS) $(LDLIBS)

.c.o:
	$(CC) -o $@ -c $(CFLAGS) $<

qdsp.o: qdsp.h glad/glad.h

glad/glad.o: glad/glad.h glad/KHR/khrplatform.h

example: example.c libqdsp.so
	$(CC) -o example $(EXAMPLE_CFLAGS) example.c libqdsp.so -lm -lfftw3
