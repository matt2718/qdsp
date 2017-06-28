# QDSP

## About

QDSP, short for "Quick Dynamic Scatter Plot" (or alternatively, "Quick and Dirty Scatter Plot") is a lightweight C library for creating dynamic, real-time scatter plots. It was originally created to render phase plots for particle-in-cell plasma simulations.

QDSP is still in early development, and should not be considered stable under any circumstances. The API could change without warning, at least until version 1.0.0 is released. I'm using semantic versioning, so any changes made after 1.0.0 will be backwards-compatible (unless I realize that I've made an unforgiveable design decision and end up releasing a 2.0.0).

## Installation

So far, QDSP is only ported to Linux. There's a good chance that it will work on Mac, but I haven't tested it. At some point in the near future, I'll redo the build system in CMake and make sure it runs on everything.

QDSP's only dependency is GLFW 3.

To install, just run:

    $ make
    $ sudo make install
	$ sudo ldconfig

After building, this will install the shared library to `/usr/local/lib` and the header to `/usr/local/include`. You may need to add `/usr/local/lib` to your system's ldconfig path.

## Usage

As of now, I have not written any documentation. This will change very soon. Check the file `example.c` for a basic example of a PIC simulation with QDSP (run `make example` if you want to try it out).

Remember to `#include <qdsp.h>`
