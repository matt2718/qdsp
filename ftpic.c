#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <fftw3.h>

const double XMAX = 16.0; // system length
const int NGRID = 256; // grid size

// particle number and properties
const int PART_NUM = 1000;
const double PART_MASS = 1.0;
const double PART_CHARGE = -0.01;
const double EPS_0 = 1.0;

// time info
const double DT = 0.01;
const double NMAX = 1000;

double shape(int x);
void init(double *x, double *v);

void deposit(double *x, fftw_complex *rhok, fftw_complex *sk);
void poisson(fftw_complex *rhok, fftw_complex *ek);
void eField(fftw_complex *ek, fftw_complex *sk, fftw_plan eIFFT);
void push(double *ex, double *x, double *v);

void updatePlot(FILE *pipe, double *x, double *v);

int main(int argc, char **argv) {
	// allocate memory
	double *x = malloc(PART_NUM * sizeof(double));
	double *v = malloc(PART_NUM * sizeof(double));

	fftw_complex *rhok = fftw_malloc(NGRID * sizeof(fftw_complex));
	fftw_complex *ek = fftw_malloc(NGRID * sizeof(fftw_complex));
	double *ex = fftw_malloc(NGRID * sizeof(double));

	double *sx = fftw_malloc(NGRID * sizeof(double));
	fftw_complex *sk = fftw_malloc(NGRID * sizeof(fftw_complex));

	// plan transforms
	fftw_plan sx2sk = fftw_plan_dft_r2c_1d(NGRID, sx, sk, FFTW_ESTIMATE);
	fftw_plan ek2ex = fftw_plan_dft_c2r_1d(NGRID, ek, ex, FFTW_MEASURE);

	// determine s(k)
	for (int j = 0; j < NGRID; j++) {
		double x = j * XMAX / NGRID;
		sx[j] += shape(x) + shape(XMAX - x);
	}
	fftw_execute(sx2sk);

	// initialize particles
	init(x, v);

	// set up plotting
	FILE *plot = popen("gnuplot -persistent", "w");
	fprintf(plot, "set style fill transparent solid 0.35 noborder\n");
	fprintf(plot, "set style circle radius 0.02\n");

	updatePlot(plot, x, v);

	int in = getchar();
	while (in != EOF) {
		for (int i = 0; i < 20; i++) {
			deposit(x, rhok, sk);
			poisson(rhok, ek);
			eField(ek, sk, ek2ex);
			push(ex, x, v);
		}
		updatePlot(plot, x, v);
		in = getchar();
	}



	// cleanup
	free(x);
	free(v);

	fftw_free(rhok);
	fftw_free(ek);
	fftw_free(ex);

	fftw_free(sx);
	fftw_free(sk);

	fftw_destroy_plan(sx2sk);
	fftw_destroy_plan(ek2ex);

	pclose(plot);

	return 0;
}

// particle shape function, centered at 0, gaussian in this case
double shape(int x) {
	const double sigma = 0.5;
	return exp(-x*x / (2 * sigma * sigma)) / sqrt(2 * M_PI * sigma * sigma);
}

void init(double *x, double *v) {
	srand(251); // consistent seed for random numbers
	for (int i = 0; i < PART_NUM; i++) {
		x[i] = XMAX * (rand() / ((double)RAND_MAX + 1));
		v[i] = (rand() % 2) ? 1.0 : -1.0;
		v[i] += 0.2 * (rand() / ((double)RAND_MAX + 1)) - 0.1;
	}
}

// determines rho(k) from list of particle positions
void deposit(double *x, fftw_complex *rhok, fftw_complex *sk) {
	for (int j = 0; j < NGRID; j++) {
		double k = (2 * M_PI / XMAX) * j;
		double real = 0;
		double imag = 0;
		for (int i = 0; i < PART_NUM; i++) {
			real += PART_CHARGE * cos(k * x[i]);
			imag -= PART_CHARGE * sin(k * x[i]);
		}
		rhok[j][0] = real * sk[j][0] - imag * sk[j][1];
		rhok[j][1] = real * sk[j][1] + imag * sk[j][0];
	}

	// background
	rhok[0][0] = 0;
}

// determines E(k) from rho(k)
void poisson(fftw_complex *rhok, fftw_complex *ek) {
	for (int j = 0; j < NGRID; j++) {
		double k = (2 * M_PI / XMAX) * j;
		ek[j][0] =  EPS_0/k * rhok[j][1];
		ek[j][1] = -EPS_0/k * rhok[j][0];
	}
}

// determines adjusted E(x) from convolution with s(k)
// modifies E(k), so don't record that after calling this
void eField(fftw_complex *ek, fftw_complex *sk, fftw_plan eIFFT) {
	for (int j = 0; j < NGRID; j++) {
		double real = ek[j][0];
		double imag = ek[j][1];
		ek[j][0] = real * sk[j][0] - imag * sk[j][1];
		ek[j][1] = real * sk[j][1] + imag * sk[j][0];
	}
	fftw_execute(eIFFT);
}

// pushes particles, electric field calculated via linear interpoation
// TODO: leapfrog integration
void push(double *ex, double *x, double *v) {
	for (int i = 0; i < PART_NUM; i++) {
		x[i] = fmod(x[i], XMAX);
		if (x[i] < 0) x[i] += XMAX;

		// calculate index and placement between grid points
		int jint = (int)(x[i] * NGRID / XMAX);
		double jfrac = (x[i] * NGRID / XMAX) - jint;

		// interpolate e(x_i)
		double e1 = ex[jint];
		double e2 = ex[(jint + 1) % NGRID];
		double ePart = e1 + (e2 - e1) * jfrac;

		// push
		v[i] += DT * (PART_CHARGE / PART_MASS) * ePart;
		x[i] += DT * v[i];

		x[i] = fmod(x[i], XMAX);
		if (x[i] < 0) x[i] += XMAX;
	}
}

// clears phase plot and outputs new data
void updatePlot(FILE *pipe, double *x, double *v) {
	fprintf(pipe, "clear\n");
	fprintf(pipe, "set yrange [-2:2]\n");
	fprintf(pipe, "plot '-' with circles lc rgb 'blue' title ''\n");
	for (int i = 0; i < PART_NUM; i++)
		fprintf(pipe, "%lf %lf\n", x[i], v[i]);
	fprintf(pipe, "e\n");
	fflush(pipe);
}
