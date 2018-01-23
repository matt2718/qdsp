#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "qdsp.h"

// system dimensiosn
const double XMAX = 16.0;
const double YMAX = 16.0;

// particle number
const int PART_NUM = 100000;

void init(double *x, double *y, int *color);

int main(int argc, char **argv) {
	// allocate memory
	double *xcoord = malloc(PART_NUM * sizeof(double));
	double *ycoord = malloc(PART_NUM * sizeof(double));

	int *color = malloc(PART_NUM * sizeof(double));
	
	// initialize x, y, and color
	for (int n = 0; n < PART_NUM; n++) {
		xcoord[n] = XMAX * n / PART_NUM;
		ycoord[n] = YMAX * rand() / (double)RAND_MAX;
		int red  = 255 * ycoord[n] / YMAX;
		int blue = 255 * (1 - ycoord[n] / YMAX);
		color[n] = (red << 16) + blue;

		ycoord[n] -= YMAX / 2;
		ycoord[n] *= 0.5 + 0.1 * sin(4 * M_PI * xcoord[n] / XMAX);
	}

	////////////////////////////////////////////////////////////
	// This is the first section relevant to QDSP

	// create phase plot with given title
	QDSPplot *plot = qdspInit("QDSP Example");

	// The framerate is capped in order to prevent superfluous updates before
	// the monitor is refreshed. The default cap is 60 FPS.
	qdspSetFramerate(plot, 120);
	
	// set x and y bounds. parameters are xmin, xmax, ymin, ymax
	qdspSetBounds(plot, 0, XMAX, -YMAX/2, YMAX/2);

	// set grid dimensions
	// parameters are:
	//   point where 1 grid line is
	//   interval between grid lines
	//   grid color
	// grid lines can be toggled by pressing 'g'
	qdspSetGridX(plot, 0, 2, 0x000000);
	qdspSetGridY(plot, 0, 2, 0x000000);
	
	// default point color and background color. pretty self-explanatory
	// the first one won't be used if we specify a color array when updating
	qdspSetPointColor(plot, 0x000000);
	qdspSetBGColor(plot, 0xffffff);

	// see below for update and cleanup code
	////////////////////////////////////////////////////////////

	struct timespec startTime;
	struct timespec endTime;
	clock_gettime(CLOCK_MONOTONIC, &startTime);
	
	int open = 1;
	int frames = 0;
	for (int i = 0; open ; i++) {
		// update particle positions
		for (int n = 0; n < PART_NUM; n++) {
			xcoord[n] = fmod(xcoord[n] + 0.01, XMAX);
			//ycoord[n] += 0.001 * sin(2 * M_PI * xcoord[n] / XMAX);
		}

		////////////////////////////////////////////////////////////
		// This function copies coordinates to the GPU and updates the plot
		//
		// parameters 2 and 3: arrays of x and y coords, both of type double
		//
		// parameter 4: array of point colors (as RGB ints)
		//     leave this NULL to use the default color
		//
		// parameter 5: number of points to plot
		//
		// The function returns 0 if the window has closed. If we update
		// before a frame has passed, it returns 2 without doing anything.
		// You can block until the next frame by calling qdspUpdateWait or
		// override the framerate cap by directly calling qdspUpdate
		open = qdspUpdateIfReady(plot, xcoord, ycoord, color, PART_NUM);
		////////////////////////////////////////////////////////////

		if (open == 1) frames++;
	}

	// calculate framerate
	clock_gettime(CLOCK_MONOTONIC, &endTime);
	double msDiff = ((double)endTime.tv_sec*1.0e3 + endTime.tv_nsec*1.0e-6) - 
		((double)startTime.tv_sec*1.0e3 + startTime.tv_nsec*1.0e-6);
	printf("%f F/s\n", 1000 * frames / msDiff);
	
	// cleanup
	free(xcoord);
	free(ycoord);

	////////////////////////////////////////////////////////////
	// Frees plot resources. Self-explanatory.
	qdspDelete(plot);

	return 0;
}
