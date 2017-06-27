#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <omp.h>

#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include "qdsp.h"

static int makeShader(char *buf, int size, GLenum type);

static void resizeCallback(GLFWwindow *window, int width, int height);

static void xy2vert(QDSPplot *plot, void *x, void *y, float *vertices,
                    int numVerts, QDSPtype type);

QDSPplot *qdspInit(const char *title) {
	QDSPplot *plot = malloc(sizeof(QDSPplot));

	// create context
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	
	// make window, basic config
	plot->window = glfwCreateWindow(800, 600, title, NULL, NULL);
	if (plot->window == NULL) {
		fprintf(stderr, "Couldn't create window\n");
		glfwTerminate();
		free(plot);
		return NULL;
	}
	glfwMakeContextCurrent(plot->window);
	glfwSetFramebufferSizeCallback(plot->window, resizeCallback);

	// load extensions via GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		fprintf(stderr, "Couldn't initialize GLAD\n");
		glfwTerminate();
		free(plot);
		return NULL;
	}

	// create shaders and link program
#include "shaders.h"
	int vertexShader = makeShader(vertex_glsl, vertex_glsl_len, GL_VERTEX_SHADER);
	int fragmentShader = makeShader(fragment_glsl, fragment_glsl_len, GL_FRAGMENT_SHADER);

	if (vertexShader == 0 || fragmentShader == 0) {
		glfwTerminate();
		free(plot);
		return NULL;
	}

	plot->shaderProgram = glCreateProgram();
	glAttachShader(plot->shaderProgram, vertexShader);
	glAttachShader(plot->shaderProgram, fragmentShader);
	glLinkProgram(plot->shaderProgram);

	int success;
	glGetProgramiv(plot->shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		char log[1024];
		glGetProgramInfoLog(plot->shaderProgram, 1024, NULL, log);
		fprintf(stderr, "Error linking program\n");
		fprintf(stderr, "%s\n", log);
		glfwTerminate();
		free(plot);
		return NULL;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	// buffer/array setup
	glGenVertexArrays(1, &plot->vertArrayObj);
	glGenBuffers(1, &plot->vertBufferObjX);
	glGenBuffers(1, &plot->vertBufferObjY);

	glBindVertexArray(plot->vertArrayObj);

	glBindBuffer(GL_ARRAY_BUFFER, plot->vertBufferObjX);
	glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, plot->vertBufferObjY);
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(1);

	// default bounds
	qdspSetBounds(plot, -1.0f, 1.0f, -1.0f, 1.0f);

	// default colors: yellow points, black background
	qdspSetPointColor(plot, 1.0f, 1.0f, 0.2f);
	qdspSetBGColor(plot, 0.0f, 0.0f, 0.0f);

	// framerate stuff
	clock_gettime(CLOCK_MONOTONIC, &plot->lastTime);

	return plot;
}

void qdspSetBounds(QDSPplot *plot, double xMin, double xMax, double yMin, double yMax) {
	glUseProgram(plot->shaderProgram);
	glUniform1f(glGetUniformLocation(plot->shaderProgram, "xMin"), xMin);
	glUniform1f(glGetUniformLocation(plot->shaderProgram, "xMax"), xMax);
	glUniform1f(glGetUniformLocation(plot->shaderProgram, "yMin"), yMin);
	glUniform1f(glGetUniformLocation(plot->shaderProgram, "yMax"), yMax);
}

void qdspSetPointColor(QDSPplot *plot, float red, float green, float blue) {
	glUseProgram(plot->shaderProgram);
	glUniform4f(glGetUniformLocation(plot->shaderProgram, "pointColor"),
	            red, green, blue, 1.0f);
}

void qdspSetBGColor(QDSPplot *plot, float red, float green, float blue) {
	plot->redBG = red;
	plot->greenBG = green;
	plot->blueBG = blue;	
}

int qdspUpdate(QDSPplot *plot, void *x, void *y, int numVerts, QDSPtype type) {
	struct timespec lastTime = plot->lastTime;
	struct timespec newTime;
	clock_gettime(CLOCK_MONOTONIC, &newTime);
	double timeDiff = ((double)newTime.tv_sec*1.0e3 + newTime.tv_nsec*1.0e-6) - 
		((double)lastTime.tv_sec*1.0e3 + lastTime.tv_nsec*1.0e-6);

	if (timeDiff >= 16)
		plot->lastTime = newTime;
	else
		return 2;

	if (glfwWindowShouldClose(plot->window)) {
		glfwTerminate();
		return 0;
	} else {
		float *vertices = malloc(3 * numVerts * sizeof(float));
		xy2vert(plot, x, y, vertices, numVerts, type);

		glBindBuffer(GL_ARRAY_BUFFER, plot->vertBufferObjX);
		glBufferData(GL_ARRAY_BUFFER, numVerts * sizeof(float), vertices, GL_STREAM_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, plot->vertBufferObjY);
		glBufferData(GL_ARRAY_BUFFER, numVerts * sizeof(float), vertices + numVerts, GL_STREAM_DRAW);

		free(vertices);

		glClearColor(plot->redBG, plot->greenBG, plot->blueBG, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(plot->shaderProgram);
		glBindVertexArray(plot->vertArrayObj);
		glDrawArrays(GL_POINTS, 0, numVerts);

		glfwSwapBuffers(plot->window);
		glfwPollEvents();

		return 1;
	}
}

void qdspDelete(QDSPplot *plot) {
		glDeleteProgram(plot->shaderProgram);
		glDeleteVertexArrays(1, &plot->vertArrayObj);
		glDeleteBuffers(1, &plot->vertBufferObjX);
		glDeleteBuffers(1, &plot->vertBufferObjY);
		free(plot);
}

static void resizeCallback(GLFWwindow *window, int width, int height) {
	glViewport(0, 0, width, height);
}

static int makeShader(char *buf, int size, GLenum type) {
	int shader = glCreateShader(type);
	glShaderSource(shader, 1, (const GLchar**)&buf, &size);
	glCompileShader(shader);
	
	// error checking
	int success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		char log[1024];
		glGetShaderInfoLog(shader, 1024, NULL, log);
		if (type == GL_VERTEX_SHADER)
			fprintf(stderr, "Error compiling vertex shader\n");
		else if (type == GL_FRAGMENT_SHADER)
			fprintf(stderr, "Error compiling fragment shader\n");
		else
			fprintf(stderr, "Error compiling shader\n");

		fprintf(stderr, "%s\n", log);
		return 0;
	}

	return shader;
}

static void xy2vert(QDSPplot *plot, void *x, void *y, float *vertices,
                    int numVerts, QDSPtype type) {	
#pragma omp parallel for
	for (int i = 0; i < numVerts; i++) {
		double xval, yval;
		switch (type) {
		case QDSP_INT:
			xval = ((int*)x)[i];
			yval = ((int*)y)[i];
			break;
		case QDSP_FLOAT:
			xval = ((float*)x)[i];
			yval = ((float*)y)[i];
			break;
		case QDSP_DOUBLE:
		default:
			xval = ((double*)x)[i];
			yval = ((double*)y)[i];
			break;
		}
			
		vertices[i] = (float)xval;
		vertices[numVerts + i] = (float)yval;
		vertices[2*numVerts + i] = 0.0f;
	}
}
