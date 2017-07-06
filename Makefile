CC=gcc
CFLAGS=-std=gnu99 -fPIC -I./include -D QDSP_RESOURCE_DIR=\"$(INSTPREFIX)/share/qdsp/\"
LDFLAGS=-shared
LDLIBS=-lGL -lglfw -lSOIL
EXAMPLE_CFLAGS=-std=gnu99 -fopenmp -I./include

SOURCES=qdsp.c glad.c
SHADERS=points.vert.glsl points.frag.glsl overlay.vert.glsl overlay.frag.glsl
IMAGES=images/helpmessage.png images/numbers.png

OBJECTS=$(SOURCES:.c=.o)

INSTPREFIX=/usr/local

VPATH = src:include:shaders:resources

.PHONY: all
all: libqdsp.so $(IMAGES)

.PHONY: debug
debug: libqdsp.so example
debug: CFLAGS += -g -O0
debug: EXAMPLE_CFLAGS += -g -O0

.PHONY: clean
clean:
	rm -rf images
	rm -f libqdsp.so $(OBJECTS)
	rm -f example

.PHONY: install
install: all qdsp.h $(SHADERS)
	mkdir -p $(INSTPREFIX)/share/qdsp
	cp libqdsp.so $(INSTPREFIX)/lib
	cp include/qdsp.h $(INSTPREFIX)/include
	cp -r shaders images $(INSTPREFIX)/share/qdsp
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

example: example.c all
	$(CC) -o example $(EXAMPLE_CFLAGS) $< libqdsp.so -lm -lfftw3 -Lqdsp -Wl,-R.

images/helpmessage.png: helpmessage
	mkdir -p images
	convert -size 400x400 xc:black -font "Ubuntu-Mono" -pointsize 16 \
	  -fill white -annotate +15+15 "$$(cat resources/helpmessage)" \
	  images/helpmessage.png

images/numbers.png:
	convert -size 312x48 xc:white -font "Ubuntu-Mono" -pointsize 48 \
	  -fill black -annotate +0+40 "0123456789.-e" images/numbers.png

qdsp.o: qdsp.h glad/glad.h

glad.o: glad/glad.h KHR/khrplatform.h
