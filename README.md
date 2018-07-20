# QDSP

## About

QDSP, short for "Quick Dynamic Scatter Plot" (or alternatively, "Quick and Dirty
Scatter Plot") is a lightweight C library for creating dynamic, real-time
scatter plots. On decent hardware, it should be able to render 10^6 points every
frame.

QDSP uses semantic versioning, so any changes made after 1.0.0 will be
backwards-compatible (until I realize that I've made an unforgivable design
decision and end up releasing 2.0.0).

## Motivation

I work in a research group that focuses on computational plasma physics. A few
months back, I was troubleshooting some code, and I needed a way to view phase
plots while debugging (piping the data to gnuplot was too slow) I was learning
OpenGL at the time, so I hacked together some code to monitor my simulation in
real time. This ended up being *really* useful, so I cleaned it up and turned it
into a library.

## Installation

So far, QDSP is only ported to Linux. There's a very good chance that it'll work
on OS X, but I haven't tested it. Windows users *might* be able to get it to
build, but they'll have to mess around with the Makefile to actually install it.

At some point in the future, I'll redo the build system and make it runs on
everything. I'm not really familiar with how Windows manages libraries, so it
will probably be a long time before this happens. If anyone else wants to take a
crack at it, feel free.

QDSP requires the following dependencies, which should be available in most
package repositories:

* [GLFW 3](http://www.glfw.org/docs/latest)
* [SOIL](http://www.lonesock.net/soil.html)
* [ImageMagick](http://www.imagemagick.org/script/index.php)

In addition, [Doxygen](http://www.doxygen.org) is required to generate
documentation for the C API, and [Sphinx](http://www.sphinx-doc.org/en/master/)
is required to generate documentation for the Python API.

To install, just run:

    $ make
    $ sudo make install
    $ sudo ldconfig

After building, this will install the shared library to `/usr/local/lib`, the
header to `/usr/local/include`, and resources to `/usr/local/share/qdsp`. You
may need to add `/usr/local/lib` to your system's ldconfig path. You can modify
the install location by changing the `INSTPREFIX` variable in the Makefile.

The Python bindings can be installed by running `pip install .` in the `python`
directory. You'll need to have the C library installed to actually use them.
I'll get a PyPI package put up at some point.

## Usage

Just put `#include <qdsp.h>` in your code and link with -lqdsp.

## Documentation

API documentation is available [here](https://msmitchell.org/qdsp/).

Look at the file `src/example1.c` for a basic example of how to use QDSP (run
`make example1` if you want to try it out). You can also look at `example2`,
which uses QDSP to render the phase plot of a 1D PIC simulation. A separate
Python example is located in `python/example.py`.

C documentation can be generated by `cd`ing into the `docs` directory and
running `doxygen`. Python documentation can be generated by `cd`ing into
`python/docs` and running `make html`.

## License

QDSP is licensed under the LGPL. This means you can use it for pretty much
anything, but a modified version of the library itself can only be distributed
under a compatible license. For more information, see the `LICENSE` file.

## Author

QDSP was primarily developed by Matt Mitchell. Please direct all questions,
comments, and concerns to <matthew.s.mitchell@colorado.edu>.

## Contributing

Want to help improve QDSP? See `CONTRIBUTING.md` for more information.

## Acknowledgements

Thanks to Matt Miecnikowski for adding the Fortran bindings.
