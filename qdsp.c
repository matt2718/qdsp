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

	// buffer setup
	glGenVertexArrays(1, &plot->vertArrayObj);
	glGenBuffers(1, &plot->vertBufferObjX);
	glGenBuffers(1, &plot->vertBufferObjY);
	glGenBuffers(1, &plot->vertBufferObjCol);

	glBindVertexArray(plot->vertArrayObj);

	glBindBuffer(GL_ARRAY_BUFFER, plot->vertBufferObjX);
	glVertexAttribPointer(0, 1, GL_DOUBLE, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, plot->vertBufferObjY);
	glVertexAttribPointer(1, 1, GL_DOUBLE, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, plot->vertBufferObjCol);
	glVertexAttribIPointer(2, 1, GL_INT, 0, NULL);
	glEnableVertexAttribArray(2);

	// default bounds
	qdspSetBounds(plot, -1.0f, 1.0f, -1.0f, 1.0f);

	// default colors: yellow points, black background
	qdspSetPointColor(plot, 0xffff33);
	qdspSetBGColor(plot, 0x000000);

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

void qdspSetPointColor(QDSPplot *plot, int rgb) {
	glUseProgram(plot->shaderProgram);
	glUniform1i(glGetUniformLocation(plot->shaderProgram, "defaultColor"), rgb);
}

void qdspSetBGColor(QDSPplot *plot, int rgb) {
	glClearColor((0xff & rgb >> 16) / 255.0,
	             (0xff & rgb >> 8) / 255.0,
	             (0xff & rgb) / 255.0,
	             1.0f);
}

int qdspUpdate(QDSPplot *plot, double *x, double *y, int *color, int numVerts) {
	// get ms since last full update (not last call)
	struct timespec lastTime = plot->lastTime;
	struct timespec newTime;
	clock_gettime(CLOCK_MONOTONIC, &newTime);
	double timeDiff = ((double)newTime.tv_sec*1.0e3 + newTime.tv_nsec*1.0e-6) - 
		((double)lastTime.tv_sec*1.0e3 + lastTime.tv_nsec*1.0e-6);

	// this whole function is a waste of time if no frame update is needed
	if (timeDiff >= 16)
		plot->lastTime = newTime;
	else
		return 2;

	// someone closed the window
	if (glfwWindowShouldClose(plot->window)) {
		glfwTerminate();
		return 0;
	}

	// copy all our vertex stuff
	glUseProgram(plot->shaderProgram);

	glBindBuffer(GL_ARRAY_BUFFER, plot->vertBufferObjX);
	glBufferData(GL_ARRAY_BUFFER, numVerts * sizeof(double), x, GL_STREAM_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, plot->vertBufferObjY);
	glBufferData(GL_ARRAY_BUFFER, numVerts * sizeof(double), y, GL_STREAM_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, plot->vertBufferObjCol);
	glBufferData(GL_ARRAY_BUFFER, numVerts * sizeof(int), color, GL_STREAM_DRAW);


	// should we use the default color?
	glUniform1i(glGetUniformLocation(plot->shaderProgram, "useCustom"), color != NULL);

	// drawing
	glClear(GL_COLOR_BUFFER_BIT);
	glBindVertexArray(plot->vertArrayObj);
	glDrawArrays(GL_POINTS, 0, numVerts);

	glfwSwapBuffers(plot->window);
	glfwPollEvents();

	return 1;
}

void qdspDelete(QDSPplot *plot) {
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
