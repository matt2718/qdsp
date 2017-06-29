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
debug: libqdsp.so
debug: CFLAGS += -g -O0
debug: EXAMPLE_CFLAGS += -g -O0

.PHONY: clean
clean:
	rm -f libqdsp.so $(OBJECTS) shaders.h

.PHONY: install
install: libqdsp.so qdsp.h
	cp libqdsp.so $(INSTPREFIX)/lib/
	cp qdsp.h $(INSTPREFIX)/include/
	@echo "Installed successfully. You may need to run ldconfig."

libqdsp.so: $(OBJECTS)
	$(CC) -o libqdsp.so $(CFLAGS) $(LDFLAGS) $(OBJECTS) $(LDLIBS)

.c.o:
	$(CC) -o $@ -c $(CFLAGS) $<

shaders.h: $(SHADERS)
	xxd -i vertex.glsl > shaders.h
	xxd -i fragment.glsl >> shaders.h

qdsp.o: qdsp.h glad/glad.h shaders.h
glad/glad.o: glad/glad.h glad/KHR/khrplatform.h


example: example.c
	$(CC) -o example $(EXAMPLE_CFLAGS) example.c -lm -lfftw3 -lqdsp
