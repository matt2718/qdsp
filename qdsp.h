#ifndef _QDSP_H
#define _QDSP_H

#include <GLFW/glfw3.h>

typedef struct QDSPplot {
	GLFWwindow *window;

	int shaderProgram;
	unsigned int vertArrayObj;
	unsigned int vertBufferObjX;
	unsigned int vertBufferObjY;

	double xMin, xMax;
	double yMin, yMax;

	struct timespec lastTime;

} QDSPplot;

QDSPplot *qdspInit(const char *title);

void qdspSetBounds(QDSPplot *plot, double xMin, double xMax, double yMin, double yMax);

void qdspSetPointColor(QDSPplot *plot, int rgb);

void qdspSetBGColor(QDSPplot *plot, int rgb);

int qdspUpdate(QDSPplot *plot, double *x, double *y, int numVerts);

void qdspDelete(QDSPplot *plot);

#endif
