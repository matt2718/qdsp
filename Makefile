CC=gcc
CFLAGS=-std=gnu99 -fPIC -I./include
LDFLAGS=-shared
LDLIBS=-lGL -lglfw -lSOIL
EXAMPLE_CFLAGS=-std=gnu99 -fopenmp -I./include

SOURCES=qdsp.c glad.c
SHADERS=points.vert.glsl points.frag.glsl overlay.vert.glsl overlay.frag.glsl
OBJECTS=$(SOURCES:.c=.o)

INSTPREFIX=/usr/local

VPATH = src:include:shaders:resources

.PHONY: all
all: libqdsp.so helpmessage.png

.PHONY: debug
debug: libqdsp.so example
debug: CFLAGS += -g -O0
debug: EXAMPLE_CFLAGS += -g -O0

.PHONY: clean
clean:
	rm -f helpmessage.png
	rm -f libqdsp.so $(OBJECTS)
	rm -f example

.PHONY: install
install: libqdsp.so qdsp.h $(SHADERS) helpmessage.png
	mkdir -p $(INSTPREFIX)/share/qdsp/resources
	cp libqdsp.so $(INSTPREFIX)/lib
	cp include/qdsp.h $(INSTPREFIX)/include
	cp -r shaders $(INSTPREFIX)/share/qdsp
	cp helpmessage.png $(INSTPREFIX)/share/qdsp/resources
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
	$(CC) -o example $(EXAMPLE_CFLAGS) $< libqdsp.so -lm -lfftw3 -Lqdsp -Wl,-R.

helpmessage.png: helpmessage
	convert -size 400x400 xc:black -font "Ubuntu-Mono" -pointsize 16 \
	-fill white -annotate +15+15 "$$(cat resources/helpmessage)" helpmessage.png

qdsp.o: qdsp.h glad/glad.h

glad.o: glad/glad.h KHR/khrplatform.h
