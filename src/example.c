#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <fftw3.h>
#include <omp.h>

#include "qdsp.h"

const double XMAX = 16.0; // system length
const int NGRID = 256; // grid size
double DX;

// particle number and properties
const int PART_NUM = 100000;
const double PART_MASS = 1.0;
const double PART_CHARGE = -0.01;
const double EPS_0 = 1.0;

// time info
const double DT = 0.0005;
const double NMAX = 1000;

// plans and buffers for fft
fftw_plan rhoFFT;
fftw_plan eIFFT;
double *rhoxBuf, *exBuf;
fftw_complex *rhokBuf, *ekBuf;

void init(double *x, double *v, int *color);

void deposit(double *x, double *rho);
void poisson(double *rho, double *e);
void xPush(double *x, double *v);
void vHalfPush(double *x, double *v, double *e, int forward);

int main(int argc, char **argv) {
	DX = XMAX / NGRID;
	// allocate memory
	double *x = malloc(PART_NUM * sizeof(double));
	double *v = malloc(PART_NUM * sizeof(double));
	int *color = malloc(PART_NUM * sizeof(int));

	double *rho = malloc(NGRID * sizeof(double));
	double *eField = malloc(NGRID * sizeof(double));

	// transform buffers
	rhoxBuf = fftw_malloc(NGRID * sizeof(double));
	exBuf = fftw_malloc(NGRID * sizeof(double));
	rhokBuf = fftw_malloc(NGRID * sizeof(fftw_complex));
	ekBuf = fftw_malloc(NGRID * sizeof(fftw_complex));
	// plan transforms
	rhoFFT = fftw_plan_dft_r2c_1d(NGRID, rhoxBuf, rhokBuf, FFTW_MEASURE);
	eIFFT =  fftw_plan_dft_c2r_1d(NGRID, ekBuf, exBuf, FFTW_MEASURE);

	// initialize particles
	init(x, v, color);

	////////////////////////////////////////////////////////////
	// This is the first section relevant to QDSP

	// create phase plot with given title
	QDSPplot *plot = qdspInit("PIC phase plot");

	// set x and y bounds. parameters are xmin, xmax, ymin, ymax
	qdspSetBounds(plot, 0, XMAX, -30, 30);

	// set grid dimensions
	// parameters are:
	//   point where 1 grid line is
	//   interval between grid lines
	//   grid color
	// grid lines can be toggled by pressing 'g'
	qdspSetGridX(plot, 0, 4, 0x000000);
	qdspSetGridY(plot, 0, 5, 0x000000);
	
	// default point color and background color. pretty self-explanatory
	// the first one won't be used if we specify a color array when updating
	qdspSetPointColor(plot, 0x000000);
	qdspSetBGColor(plot, 0xffffff);

	// see below for update and cleanup code
	////////////////////////////////////////////////////////////

	deposit(x, rho);
	poisson(rho, eField);
	vHalfPush(x, v, eField, 0); // push backwards

	int open = 1;
	for (int i = 0; open ; i++) {
		deposit(x, rho);
		poisson(rho, eField);

		vHalfPush(x, v, eField, 1);

		////////////////////////////////////////////////////////////
		// this function copies coords to the GPU and updates the plot
		//
		// parameters 2 and 3: arrays of x and y coords, both of type double
		//
		// parameter 4: array of point colors (as RGB ints),
		//     leave this NULL to use the default color
		//
		// parameter 5: specifies the number of points to plot
		//
		// the function returns zero iff the window has closed
		// it returns 2 without doing anything if we update before a frame has passed
		
		open = qdspUpdateIfReady(plot, x, v, color, PART_NUM);

		////////////////////////////////////////////////////////////
		
		vHalfPush(x, v, eField, 1);

		xPush(x, v);
	}

	// cleanup
	free(x);
	free(v);
	free(color);
	free(rho);
	free(eField);

	fftw_free(rhoxBuf);
	fftw_free(rhokBuf);
	fftw_free(exBuf);
	fftw_free(ekBuf);

	fftw_destroy_plan(rhoFFT);
	fftw_destroy_plan(eIFFT);

	////////////////////////////////////////////////////////////
	// frees plot resources. self-explanatory.
	qdspDelete(plot);

	return 0;
}

void init(double *x, double *v, int *color) {
	double stdev = sqrt(50000 / (5.1e5));
	for (int i = 0; i < PART_NUM; i++) {
		x[i] = i * XMAX / PART_NUM;

		if (i % 2) {
			v[i] = 8.0;
			color[i] = 0xff0000;
		} else {
			v[i] = -8.0;
			color[i] = 0x0000ff;
		}

		double r1 = (rand() + 1) / ((double)RAND_MAX + 1); // log(0) breaks stuff
		double r2 = (rand() + 1) / ((double)RAND_MAX + 1);
		v[i] += stdev * sqrt(-2 * log(r1)) * cos(2 * M_PI * r2);
	}
}

void deposit(double *x, double *rho) {
	for (int j = 0; j < NGRID; j++)
		// neutralizing bg
		rho[j] = -PART_NUM * PART_CHARGE / XMAX;

#pragma omp parallel
	{
		double *myRho = calloc(NGRID, sizeof(double));

#pragma omp for
		for (int i = 0; i < PART_NUM; i++) {
			int j = x[i] / DX;
			double xg = j * DX;
			myRho[j] += PART_CHARGE * (xg + DX - x[i]) / (DX * DX);
			myRho[(j+1) % NGRID] += PART_CHARGE * (x[i] - xg) / (DX * DX);
		}
				
#pragma omp critical
		{
			for (int j = 0; j < NGRID; j++)
				rho[j] += myRho[j];
		}
		free(myRho);
	}
}

// determines E from rho
void poisson(double *rho, double *e) {
	memcpy(rhoxBuf, rho, NGRID * sizeof(double));

	fftw_execute(rhoFFT);
	for (int j = 0; j < NGRID; j++) {
		double k = 2 * M_PI * j / XMAX;
		ekBuf[j][0] =  rhokBuf[j][1] / (k * EPS_0);
		ekBuf[j][1] = -rhokBuf[j][0] / (k * EPS_0);
	}
	ekBuf[0][0] = 0;
	ekBuf[0][1] = 0;

	fftw_execute(eIFFT);

	memcpy(e, exBuf, NGRID * sizeof(double));
}

void xPush(double *x, double *v) {
#pragma omp parallel for
	for (int i = 0; i < PART_NUM; i++) {
		x[i] += DT * v[i];

		// periodicity
		// (not strictly correct, but if a particle is moving several grid
		// lengths in 1 timestep, something has gone horribly wrong)
		if (x[i] < 0) x[i] += XMAX;
		if (x[i] >= XMAX) x[i] -= XMAX;
	}
}

// pushes particles, electric field calculated via linear interpoation
void vHalfPush(double *x, double *v, double *e, int forward) {
#pragma omp parallel for
	for (int i = 0; i < PART_NUM; i++) {
		// calculate index and placement between grid points
		int jint = (int)(x[i] * NGRID / XMAX);
		double jfrac = (x[i] * NGRID / XMAX) - jint;

		// interpolate e(x_i)
		double e1 = e[jint];
		double e2 = e[(jint + 1) % NGRID];
		double ePart = e1 + (e2 - e1) * jfrac;

		// push
		if (forward)
			v[i] += DT/2 * (PART_CHARGE / PART_MASS) * ePart;
		else
			v[i] -= DT/2 * (PART_CHARGE / PART_MASS) * ePart;
	}
}
