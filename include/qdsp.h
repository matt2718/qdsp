#ifndef _QDSP_H
#define _QDSP_H

#include <GLFW/glfw3.h>

typedef struct QDSPplot {
	GLFWwindow *window;

	int paused;
	int overlay;
	int grid;

	int xAutoGrid, yAutoGrid;

	// frame info
	struct timespec lastUpdate;
	double frameInterval;

	// bounds, we could probably use glGetUniform, but storing them is easier
	double xMin, xMax;
	double yMin, yMax;
	
	// opengl stuff:
	int pointsProgram;
	unsigned int pointsVAO;
	unsigned int pointsVBOx;
	unsigned int pointsVBOy;
	unsigned int pointsVBOrgb;

	int gridProgram;
	unsigned int gridVAOx;
	unsigned int gridVBOx;
	unsigned int gridVAOy;
	unsigned int gridVBOy;

	int overlayProgram;
	unsigned int overlayVAO;
	unsigned int overlayVBO;
	unsigned int overlayTexture;

	// needed so we can redraw at will
	int numPoints;
	int numGridX;
	int numGridY;
	
} QDSPplot;

QDSPplot *qdspInit(const char *title);

void qdspDelete(QDSPplot *plot);

int qdspUpdate(QDSPplot *plot, double *x, double *y, int *color, int numPoints);

int qdspUpdateIfReady(QDSPplot *plot, double *x, double *y, int *color, int numPoints);

int qdspUpdateWait(QDSPplot *plot, double *x, double *y, int *color, int numPoints);

void qdspRedraw(QDSPplot *plot);

void qdspSetFramerate(QDSPplot *plot, double framerate);

void qdspSetBounds(QDSPplot *plot, double xMin, double xMax, double yMin, double yMax);

void qdspSetPointColor(QDSPplot *plot, int rgb);

void qdspSetBGColor(QDSPplot *plot, int rgb);

void qdspSetGridX(QDSPplot *plot, double point, double interval, int rgb);

void qdspSetGridY(QDSPplot *plot, double point, double interval, int rgb);

#endif
