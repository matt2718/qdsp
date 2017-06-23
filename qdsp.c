#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <omp.h>

#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include "qdsp.h"

// shader sources
static int makeShader(char *buf, int size, GLenum type);
static void resizeCallback(GLFWwindow *window, int width, int height);

QDSPplot qdspInit(const char *title) {
	QDSPplot plot;
	// create context
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	
	// make window, basic config
	plot.window = glfwCreateWindow(800, 600, title, NULL, NULL);
	if (plot.window == NULL) {
		fprintf(stderr, "Couldn't create window\n");
		glfwTerminate();
		plot.success = 0;
		return plot;
	}
	glfwMakeContextCurrent(plot.window);
	glfwSetFramebufferSizeCallback(plot.window, resizeCallback);

	// load extensions via GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		fprintf(stderr, "Couldn't initialize GLAD\n");
		glfwTerminate();
		plot.success = 0;
		return plot;
	}

	// create shaders and link program
#include "shaders.h"
	int vertexShader = makeShader(vertex_glsl, vertex_glsl_len, GL_VERTEX_SHADER);
	int fragmentShader = makeShader(fragment_glsl, fragment_glsl_len, GL_FRAGMENT_SHADER);

	if (vertexShader == 0 || fragmentShader == 0) {
		glfwTerminate();
		plot.success = 0;
		return plot;
	}

	plot.shaderProgram = glCreateProgram();
	glAttachShader(plot.shaderProgram, vertexShader);
	glAttachShader(plot.shaderProgram, fragmentShader);
	glLinkProgram(plot.shaderProgram);

	int success;
	glGetProgramiv(plot.shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		char log[1024];
		glGetProgramInfoLog(plot.shaderProgram, 1024, NULL, log);
		fprintf(stderr, "Error linking program\n");
		fprintf(stderr, "%s\n", log);
		glfwTerminate();
		plot.success = 0;
		return plot;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	// buffer/array setup
	glGenVertexArrays(1, &plot.vertArrayObj);
	glGenBuffers(1, &plot.vertBufferObj);

	glBindVertexArray(plot.vertArrayObj);
	glBindBuffer(GL_ARRAY_BUFFER, plot.vertBufferObj);
	//glBufferData(GL_ARRAY_BUFFER, 3 * numVerts * sizeof(float), vertices, GL_STREAM_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	clock_gettime(CLOCK_MONOTONIC, &plot.lastTime);
	plot.success = 1;
	return plot;
}

int qdspUpdate(QDSPplot *plot, float *vertices, int numVerts) {
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
		glDeleteProgram(plot->shaderProgram);
		glDeleteVertexArrays(1, &plot->vertArrayObj);
		glDeleteBuffers(1, &plot->vertBufferObj);
		return 0;
	} else {
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		
		glBufferData(GL_ARRAY_BUFFER, 3 * numVerts * sizeof(float), vertices, GL_STREAM_DRAW);

		glUseProgram(plot->shaderProgram);
		glBindVertexArray(plot->vertArrayObj);
		glDrawArrays(GL_POINTS, 0, numVerts);

		glfwSwapBuffers(plot->window);
		glfwPollEvents();
		return 1;
	}
}

void resizeCallback(GLFWwindow *window, int width, int height) {
	glViewport(0, 0, width, height);
}

int makeShader(char *buf, int size, GLenum type) {
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
