#ifndef _QDSP_H
#define _QDSP_H

#include <GLFW/glfw3.h>

typedef struct QDSPplot {
	GLFWwindow *window;

	int paused;
	int overlay;

	// frame info
	struct timespec lastUpdate;
	double frameInterval;

	// opengl stuff:
	int pointsProgram;
	unsigned int pointsVAO;
	unsigned int pointsVBOx;
	unsigned int pointsVBOy;
	unsigned int pointsVBOrgb;

	int overlayProgram;
	unsigned int overlayVAO;
	unsigned int overlayVBO;
	unsigned int overlayTexture;

} QDSPplot;

QDSPplot *qdspInit(const char *title);

void qdspDelete(QDSPplot *plot);

int qdspUpdate(QDSPplot *plot, double *x, double *y, int *color, int numVerts);

int qdspUpdateIfReady(QDSPplot *plot, double *x, double *y, int *color, int numVerts);

int qdspUpdateWait(QDSPplot *plot, double *x, double *y, int *color, int numVerts);

void qdspSetFramerate(QDSPplot *plot, double framerate);

void qdspSetBounds(QDSPplot *plot, double xMin, double xMax, double yMin, double yMax);

void qdspSetPointColor(QDSPplot *plot, int rgb);

void qdspSetBGColor(QDSPplot *plot, int rgb);

#endif
