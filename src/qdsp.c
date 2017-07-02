#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <omp.h>

#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <SOIL/SOIL.h>

#include "qdsp.h"

static int makeShader(const char *filename, GLenum type);

static void closeCallback(GLFWwindow *window);

static void resizeCallback(GLFWwindow *window, int width, int height);

static void keyCallback(GLFWwindow *window, int key, int code, int action, int mods);

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

	// we need to get the plot in the key hander
	glfwSetWindowUserPointer(plot->window, plot);

	glfwSetWindowCloseCallback(plot->window, closeCallback);
	glfwSetFramebufferSizeCallback(plot->window, resizeCallback);
	glfwSetKeyCallback(plot->window, keyCallback);
	
	// load extensions via GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		fprintf(stderr, "Couldn't initialize GLAD\n");
		glfwTerminate();
		free(plot);
		return NULL;
	}

	// create shaders and link program

	// for points
	int pointsVert = makeShader("points.vert.glsl", GL_VERTEX_SHADER);
	int pointsFrag = makeShader("points.frag.glsl", GL_FRAGMENT_SHADER);

	// for overlay
	int overVert = makeShader("overlay.vert.glsl", GL_VERTEX_SHADER);
	int overFrag = makeShader("overlay.frag.glsl", GL_FRAGMENT_SHADER);
	
	if (pointsVert == 0 || pointsFrag == 0 || overVert == 0 || overFrag == 0) {
		glfwTerminate();
		free(plot);
		return NULL;
	}

	plot->pointsProgram = glCreateProgram();
	glAttachShader(plot->pointsProgram, pointsVert);
	glAttachShader(plot->pointsProgram, pointsFrag);
	glLinkProgram(plot->pointsProgram);

	plot->overlayProgram = glCreateProgram();
	glAttachShader(plot->overlayProgram, overVert);
	glAttachShader(plot->overlayProgram, overFrag);
	glLinkProgram(plot->overlayProgram);

	int pointSuccess, overSuccess;
	glGetProgramiv(plot->pointsProgram, GL_LINK_STATUS, &pointSuccess);
	glGetProgramiv(plot->overlayProgram, GL_LINK_STATUS, &overSuccess);
	if (!pointSuccess || !overSuccess) {
		char log[1024];
		glGetProgramInfoLog(plot->pointsProgram, 1024, NULL, log);
		fprintf(stderr, "Error linking program\n");
		fprintf(stderr, "%s\n", log);
		glfwTerminate();
		free(plot);
		return NULL;
	}

	glDeleteShader(pointsVert);
	glDeleteShader(pointsFrag);
	glDeleteShader(overVert);
	glDeleteShader(overFrag);

	// buffer setup for points
	glGenVertexArrays(1, &plot->pointsVAO);
	glGenBuffers(1, &plot->pointsVBOx);
	glGenBuffers(1, &plot->pointsVBOy);
	glGenBuffers(1, &plot->pointsVBOrgb);

	glBindVertexArray(plot->pointsVAO);

	glBindBuffer(GL_ARRAY_BUFFER, plot->pointsVBOx);
	glVertexAttribPointer(0, 1, GL_DOUBLE, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, plot->pointsVBOy);
	glVertexAttribPointer(1, 1, GL_DOUBLE, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, plot->pointsVBOrgb);
	glVertexAttribIPointer(2, 1, GL_INT, 0, NULL);
	glEnableVertexAttribArray(2);

	// buffer setup for overlay
	glGenVertexArrays(1, &plot->overlayVAO);
	glGenBuffers(1, &plot->overlayVBO);
	
	glBindVertexArray(plot->overlayVAO);

	glBindBuffer(GL_ARRAY_BUFFER, plot->overlayVBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
	glEnableVertexAttribArray(0);

	// coords for overlay
	float overVertices[] = {
		// lower left triangle
		0.0f, 0.0f, 1.0f,
		0.0f, 1.0f, 1.0f,
		1.0f, 0.0f, 1.0f,
		// upper right triangle
		0.0f, 1.0f, 1.0f,
		1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 1.0f
	};
	glBufferData(GL_ARRAY_BUFFER, sizeof(overVertices), overVertices, GL_STATIC_DRAW);

	// overlay texture
	glGenTextures(1, &plot->overlayTexture);
	glBindTexture(GL_TEXTURE_2D, plot->overlayTexture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// load image with SOIL
	int imgWidth, imgHeight;
	unsigned char *imgData =
		SOIL_load_image("/usr/local/share/qdsp/helpmessage.png",
		                &imgWidth, &imgHeight, NULL, SOIL_LOAD_RGB);

	if (imgData == NULL)
		// loading failed, try current dir
		imgData = SOIL_load_image("helpmessage.png", &imgWidth, &imgHeight,
		                          NULL, SOIL_LOAD_RGB);
		
	if (imgData == NULL) { // file is nowhere
		fprintf(stderr, "Error loading image file helpmessage.png\n");
		glfwTerminate();
		free(plot);
		return NULL;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imgWidth, imgHeight, 0, GL_RGB,
	             GL_UNSIGNED_BYTE, imgData);
	
	SOIL_free_image_data(imgData);

	// dimensions
	glUseProgram(plot->overlayProgram);
	glUniform2f(glGetUniformLocation(plot->overlayProgram, "imgDims"),
	            imgWidth, imgHeight);
	resizeCallback(plot->window, 800, 600);
	
	// transparency
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 

	// default to 60 fps
	qdspSetFramerate(plot, 60);
	
	// default bounds
	qdspSetBounds(plot, -1.0f, 1.0f, -1.0f, 1.0f);

	// default colors: yellow points, black background
	qdspSetPointColor(plot, 0xffff33);
	qdspSetBGColor(plot, 0x000000);

	plot->paused = 0;
	plot->overlay = 0;
	
	// framerate stuff
	clock_gettime(CLOCK_MONOTONIC, &plot->lastUpdate);

	return plot;
}

void qdspDelete(QDSPplot *plot) {
	free(plot);
}

int qdspUpdate(QDSPplot *plot, double *x, double *y, int *color, int numVerts) {
	// we just got updated
	clock_gettime(CLOCK_MONOTONIC, &plot->lastUpdate);

	while (plot->paused) {
		glfwWaitEvents();
	}
		
	// someone closed the window
	if (glfwWindowShouldClose(plot->window)) {
		glfwTerminate();
		return 0;
	}
	
	// copy all our vertex stuff
	glUseProgram(plot->pointsProgram);

	glBindBuffer(GL_ARRAY_BUFFER, plot->pointsVBOx);
	glBufferData(GL_ARRAY_BUFFER, numVerts * sizeof(double), x, GL_STREAM_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, plot->pointsVBOy);
	glBufferData(GL_ARRAY_BUFFER, numVerts * sizeof(double), y, GL_STREAM_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, plot->pointsVBOrgb);
	glBufferData(GL_ARRAY_BUFFER, numVerts * sizeof(int), color, GL_STREAM_DRAW);

	// should we use the default color?
	glUniform1i(glGetUniformLocation(plot->pointsProgram, "useCustom"), color != NULL);

	plot->lastNumVerts = numVerts;
	
	// drawing
	glClear(GL_COLOR_BUFFER_BIT);
	
	glBindVertexArray(plot->pointsVAO);
	glDrawArrays(GL_POINTS, 0, numVerts);

	if (plot->overlay) {
		glUseProgram(plot->overlayProgram);
		glBindVertexArray(plot->overlayVAO);
		glBindTexture(GL_TEXTURE_2D, plot->overlayTexture);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
	
	glfwSwapBuffers(plot->window);
	glfwPollEvents();

	return 1;
}

int qdspUpdateIfReady(QDSPplot *plot, double *x, double *y, int *color, int numVerts) {
	// get ms since last full update
	struct timespec lastTime = plot->lastUpdate;
	struct timespec newTime;
	clock_gettime(CLOCK_MONOTONIC, &newTime);
	double msDiff = ((double)newTime.tv_sec*1.0e3 + newTime.tv_nsec*1.0e-6) - 
		((double)lastTime.tv_sec*1.0e3 + lastTime.tv_nsec*1.0e-6);
	
	if (msDiff >= plot->frameInterval)
		return qdspUpdate(plot, x, y, color, numVerts);
	else
		return 2;
}

int qdspUpdateWait(QDSPplot *plot, double *x, double *y, int *color, int numVerts) {
	// get ms since last full update
	struct timespec lastTime = plot->lastUpdate;
	struct timespec newTime;
	clock_gettime(CLOCK_MONOTONIC, &newTime);
	double msDiff = ((double)newTime.tv_sec*1.0e3 + newTime.tv_nsec*1.0e-6) -
		((double)lastTime.tv_sec*1.0e3 + lastTime.tv_nsec*1.0e-6);

	if (msDiff < plot->frameInterval)
		glfwWaitEventsTimeout((plot->frameInterval - msDiff) / 1000);

	return qdspUpdate(plot, x, y, color, numVerts);
}

void qdspSetFramerate(QDSPplot *plot, double framerate) {
	if (framerate <= 0)
		plot->frameInterval = 0;
	else
		plot->frameInterval = 1000.0 / framerate;
}

void qdspSetBounds(QDSPplot *plot, double xMin, double xMax, double yMin, double yMax) {
	glUseProgram(plot->pointsProgram);
	glUniform1f(glGetUniformLocation(plot->pointsProgram, "xMin"), xMin);
	glUniform1f(glGetUniformLocation(plot->pointsProgram, "xMax"), xMax);
	glUniform1f(glGetUniformLocation(plot->pointsProgram, "yMin"), yMin);
	glUniform1f(glGetUniformLocation(plot->pointsProgram, "yMax"), yMax);
}

void qdspSetPointColor(QDSPplot *plot, int rgb) {
	glUseProgram(plot->pointsProgram);
	glUniform1i(glGetUniformLocation(plot->pointsProgram, "defaultColor"), rgb);
}

void qdspSetBGColor(QDSPplot *plot, int rgb) {
	glClearColor((0xff & rgb >> 16) / 255.0,
	             (0xff & rgb >> 8) / 255.0,
	             (0xff & rgb) / 255.0,
	             1.0f);
}

static void closeCallback(GLFWwindow *window) {
	QDSPplot *plot = glfwGetWindowUserPointer(window);
	plot->paused = 0;
}

static void resizeCallback(GLFWwindow *window, int width, int height) {
	QDSPplot *plot = glfwGetWindowUserPointer(window);
	glUseProgram(plot->overlayProgram);
	glUniform2f(glGetUniformLocation(plot->overlayProgram, "pixDims"),
	            width, height);
	
	glViewport(0, 0, width, height);
}

static void keyCallback(GLFWwindow *window, int key, int code, int action, int mods) {
	QDSPplot *plot = glfwGetWindowUserPointer(window);
	
	// ESC - close
	// q - close
	if ((key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q)
	    && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, 1);
		plot->paused = 0; // can't run cleanup code while paused
	}

	// p - pause
	if (key == GLFW_KEY_P && action == GLFW_PRESS) {
		plot->paused = !plot->paused;
	}

	// h - display help
	if (key == GLFW_KEY_H && action == GLFW_PRESS) {
		plot->overlay = !plot->overlay;

		// if we don't draw here, the user can't toggle the overlay while paused
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(plot->pointsProgram);
		glBindVertexArray(plot->pointsVAO);
		glDrawArrays(GL_POINTS, 0, plot->lastNumVerts);

		if (plot->overlay) {
			glUseProgram(plot->overlayProgram);
			glBindVertexArray(plot->overlayVAO);
			glBindTexture(GL_TEXTURE_2D, plot->overlayTexture);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
	
		glfwSwapBuffers(plot->window);
	}
}

static int makeShader(const char *filename, GLenum type) {
	// will fail with a crazy-long filename, but users can't call this anyway
	char fullpath[256] = "/usr/local/share/qdsp/shaders/";
	char localpath[256] = "./shaders/";
	strcat(fullpath, filename);
	FILE *file = fopen(fullpath, "r");

	// try current directory if it failed
	if (file == NULL) {
		strcat(localpath, filename);
		file = fopen(localpath, "r");
	}
	
	if (file == NULL) {
		fprintf(stderr, "Could not find file: %s\n", fullpath);
		fprintf(stderr, "Could not find file: ./%s\n", localpath);
		return 0;
	}

	// allocate memory
	int size;
	char *buf;
	fseek(file, 0L, SEEK_END);
	size = ftell(file); // file length
	rewind(file);
	buf = malloc(size * sizeof(char));

	fread(buf, 1, size, file);

	fclose(file);

	int shader = glCreateShader(type);
	glShaderSource(shader, 1, (const GLchar**)&buf, &size);
	glCompileShader(shader);
	
	// error checking
	int success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		char log[1024];
		glGetShaderInfoLog(shader, 1024, NULL, log);
		fprintf(stderr, "Error compiling shader from file: %s\n", filename);
		fprintf(stderr, "%s\n", log);
		free(buf);
		return 0;
	}

	free(buf);
	return shader;
}
