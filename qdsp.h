#ifndef _QDSP_H
#define _QDSP_H

#include <GLFW/glfw3.h>

typedef struct QDSPplot {
	GLFWwindow *window;

	int shaderProgram;
	unsigned int vertArrayObj;
	unsigned int vertBufferObj;

	double xMin, xMax;
	double yMin, yMax;

	float redFG, greenFG, blueFG;
	float redBG, greenBG, blueBG;

	struct timespec lastTime;

} QDSPplot;

typedef enum QDSPtype {
	QDSP_INT,
	QDSP_FLOAT,
	QDSP_DOUBLE
} QDSPtype; 

QDSPplot *qdspInit(const char *title);

void qdspSetBounds(QDSPplot *plot, double xMin, double xMax, double yMin, double yMax);

void qdspSetPointColor(QDSPplot *plot, float red, float green, float blue);

void qdspSetBGColor(QDSPplot *plot, float red, float green, float blue);

int qdspUpdate(QDSPplot *plot, void *x, void *y, int numVerts, QDSPtype type);

void qdspDelete(QDSPplot *plot);

#endif
