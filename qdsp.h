#ifndef _QDSP_H
#define _QDSP_H

#include <GLFW/glfw3.h>

typedef struct QDSPplot {
	int success;
	GLFWwindow *window;
	int shaderProgram;
	unsigned int vertArrayObj;
	unsigned int vertBufferObj;
	struct timespec lastTime;
} QDSPplot;

QDSPplot qdspInit(const char *title);
int qdspUpdate(QDSPplot *plot, float *vertices, int numVerts);

#endif
