#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <omp.h>
#include <unistd.h>

#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <SOIL/SOIL.h>

#include "qdsp.h"

static void closeCallback(GLFWwindow *window);

static void resizeCallback(GLFWwindow *window, int width, int height);

static void keyCallback(GLFWwindow *window, int key, int code, int action, int mods);

static int makeShader(const char *filename, GLenum type);

static int loadTexture(const char *relpath, int *width, int *height);

static void resourcePath(char *fullpath, const char *relpath);

static void charHelper(float *addr, float x0, float y0, int xoff, int yoff, char ch);

QDSPplot *qdspInit(const char *title) {
	QDSPplot *plot = malloc(sizeof(QDSPplot));

	// store base title so we can add status indicators later
	int titleLen = strlen(title);
	plot->title = malloc((titleLen + 1) * sizeof(char));
	memcpy(plot->title, title, titleLen + 1);
	
	// create context
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_CONTEXT_RELEASE_BEHAVIOR, GLFW_RELEASE_BEHAVIOR_NONE);

	// make window, basic config
	plot->window = glfwCreateWindow(800, 600, title, NULL, NULL);

	if (plot->window == NULL) {
		fprintf(stderr, "Couldn't create window\n");
		glfwTerminate();
		free(plot);
		return NULL;
	}
	glfwMakeContextCurrent(plot->window);

	// we need to get the plot in the key handler
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
	int pointsVert = makeShader("shaders/points.vert.glsl", GL_VERTEX_SHADER);
	int pointsFrag = makeShader("shaders/points.frag.glsl", GL_FRAGMENT_SHADER);

	// for grid
	int gridVert = makeShader("shaders/grid.vert.glsl", GL_VERTEX_SHADER);
	int gridFrag = makeShader("shaders/grid.frag.glsl", GL_FRAGMENT_SHADER);

	// for text
	int textVert = makeShader("shaders/text.vert.glsl", GL_VERTEX_SHADER);
	int textFrag = makeShader("shaders/text.frag.glsl", GL_FRAGMENT_SHADER);

	// for overlay
	int overVert = makeShader("shaders/overlay.vert.glsl", GL_VERTEX_SHADER);
	int overFrag = makeShader("shaders/overlay.frag.glsl", GL_FRAGMENT_SHADER);

	// shader creation failed
	if (pointsVert == 0 || pointsFrag == 0 || gridVert == 0 || gridFrag == 0 ||
	    textVert == 0 || textFrag == 0 || overVert == 0 || overFrag == 0) {
		glfwTerminate();
		free(plot);
		return NULL;
	}

	plot->pointsProgram = glCreateProgram();
	glAttachShader(plot->pointsProgram, pointsVert);
	glAttachShader(plot->pointsProgram, pointsFrag);
	glLinkProgram(plot->pointsProgram);

	plot->textProgram = glCreateProgram();
	glAttachShader(plot->textProgram, textVert);
	glAttachShader(plot->textProgram, textFrag);
	glLinkProgram(plot->textProgram);

	plot->gridProgram = glCreateProgram();
	glAttachShader(plot->gridProgram, gridVert);
	glAttachShader(plot->gridProgram, gridFrag);
	glLinkProgram(plot->gridProgram);

	plot->overlayProgram = glCreateProgram();
	glAttachShader(plot->overlayProgram, overVert);
	glAttachShader(plot->overlayProgram, overFrag);
	glLinkProgram(plot->overlayProgram);

	int pointSuccess, gridSuccess, textSuccess, overSuccess;
	glGetProgramiv(plot->pointsProgram, GL_LINK_STATUS, &pointSuccess);
	glGetProgramiv(plot->gridProgram, GL_LINK_STATUS, &gridSuccess);
	glGetProgramiv(plot->textProgram, GL_LINK_STATUS, &textSuccess);
	glGetProgramiv(plot->overlayProgram, GL_LINK_STATUS, &overSuccess);
	if (!pointSuccess || !gridSuccess || !textSuccess || !overSuccess) {
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
	glDeleteShader(gridVert);
	glDeleteShader(gridFrag);
	glDeleteShader(textVert);
	glDeleteShader(textFrag);
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

	// buffer setup for x grid
	glGenVertexArrays(1, &plot->gridVAOx);
	glGenBuffers(1, &plot->gridVBOx);

	glBindVertexArray(plot->gridVAOx);

	glBindBuffer(GL_ARRAY_BUFFER, plot->gridVBOx);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), NULL);
	glEnableVertexAttribArray(0);

	// buffer setup for y grid
	glGenVertexArrays(1, &plot->gridVAOy);
	glGenBuffers(1, &plot->gridVBOy);

	glBindVertexArray(plot->gridVAOy);

	glBindBuffer(GL_ARRAY_BUFFER, plot->gridVBOy);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), NULL);
	glEnableVertexAttribArray(0);

	// buffer setup for x grid labels
	glGenVertexArrays(1, &plot->textVAOx);
	glGenBuffers(1, &plot->textVBOx);

	glBindVertexArray(plot->textVAOx);
	glBindBuffer(GL_ARRAY_BUFFER, plot->textVBOx);

	// xy start and offset
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
	                      (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// texture coord
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
	                      (void*)(4 * sizeof(float)));
	glEnableVertexAttribArray(2);
	
	// buffer setup for y grid labels
	glGenVertexArrays(1, &plot->textVAOy);
	glGenBuffers(1, &plot->textVBOy);

	glBindVertexArray(plot->textVAOy);
	glBindBuffer(GL_ARRAY_BUFFER, plot->textVBOy);

	// xy start and offset
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
	                      (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// texture coord
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
	                      (void*)(4 * sizeof(float)));
	glEnableVertexAttribArray(2);

	int imgWidth, imgHeight;
	// texture for drawing digits
	glGenTextures(1, &plot->numTexture);
	glBindTexture(GL_TEXTURE_2D, plot->numTexture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	loadTexture("images/digits.png", &imgWidth, &imgHeight);	

	glUseProgram(plot->textProgram);
	glUniform2f(glGetUniformLocation(plot->textProgram, "charDims"),
	            imgWidth/15, imgHeight);
	
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

	loadTexture("images/helpmessage.png", &imgWidth, &imgHeight);
	
	// dimensions
	glUseProgram(plot->overlayProgram);
	glUniform2f(glGetUniformLocation(plot->overlayProgram, "imgDims"),
	            imgWidth, imgHeight);

	// transparency
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 

	glEnable(GL_PROGRAM_POINT_SIZE);

	// default to 60 fps
	qdspSetFramerate(plot, 60);
	
	// default bounds
	qdspSetBounds(plot, -1.0f, 1.0f, -1.0f, 1.0f);

	// default: single points, yellow, black background
	qdspSetConnected(plot, 0);
	qdspSetPointColor(plot, 0xffff33);
	qdspSetBGColor(plot, 0x000000);

	// opaque points, 1 pixel wide
	qdspSetPointAlpha(plot, 1.0);
	qdspSetPointSize(plot, 1);
	
	plot->paused = 0;
	plot->frozen = 0;
	plot->overlay = 0;
	plot->grid = 0;

	plot->xAutoGrid = 1;
	plot->yAutoGrid = 1;

	resizeCallback(plot->window, 800, 600);
	glfwSwapInterval(0);
	// framerate stuff
	clock_gettime(CLOCK_MONOTONIC, &plot->lastUpdate);

	return plot;
}

void qdspDelete(QDSPplot *plot) {
	glfwTerminate();
	free(plot->title);
	free(plot);
}

int qdspUpdate(QDSPplot *plot, double *x, double *y, int *color, int numPoints) {
	glfwMakeContextCurrent(plot->window);
	// we just got updated
	clock_gettime(CLOCK_MONOTONIC, &plot->lastUpdate);

	while (plot->paused) {
		glfwWaitEvents();
	}
		
	// someone closed the window
	if (glfwWindowShouldClose(plot->window)) {
		glfwDestroyWindow(plot->window);
		return 0;
	}

	// frozen: don't update data
	if (plot->frozen) {
		glfwPollEvents();
		return 2;
	}
	
	// copy all our vertex stuff
	glUseProgram(plot->pointsProgram);

	glBindBuffer(GL_ARRAY_BUFFER, plot->pointsVBOx);
	glBufferData(GL_ARRAY_BUFFER, numPoints * sizeof(double), x, GL_STREAM_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, plot->pointsVBOy);
	glBufferData(GL_ARRAY_BUFFER, numPoints * sizeof(double), y, GL_STREAM_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, plot->pointsVBOrgb);
	glBufferData(GL_ARRAY_BUFFER, numPoints * sizeof(int), color, GL_STREAM_DRAW);

	// should we use the default color?
	glUniform1i(glGetUniformLocation(plot->pointsProgram, "useCustom"), color != NULL);

	plot->numPoints = numPoints;
	
	// drawing:
	qdspRedraw(plot);
	
	glfwPollEvents();

	return 1;
}

int qdspUpdateIfReady(QDSPplot *plot, double *x, double *y, int *color, int numPoints) {
	// get ms since last full update
	struct timespec lastTime = plot->lastUpdate;
	struct timespec newTime;
	clock_gettime(CLOCK_MONOTONIC, &newTime);
	double msDiff = ((double)newTime.tv_sec*1.0e3 + newTime.tv_nsec*1.0e-6) - 
		((double)lastTime.tv_sec*1.0e3 + lastTime.tv_nsec*1.0e-6);
	
	if (msDiff >= plot->frameInterval)
		return qdspUpdate(plot, x, y, color, numPoints);
	else
		return 2;
}

int qdspUpdateWait(QDSPplot *plot, double *x, double *y, int *color, int numPoints) {
	// get ms since last full update
	struct timespec lastTime = plot->lastUpdate;
	struct timespec newTime;
	clock_gettime(CLOCK_MONOTONIC, &newTime);
	double msDiff = ((double)newTime.tv_sec*1.0e3 + newTime.tv_nsec*1.0e-6) -
		((double)lastTime.tv_sec*1.0e3 + lastTime.tv_nsec*1.0e-6);

	if (msDiff < plot->frameInterval)
		usleep((plot->frameInterval - msDiff) * 1000);

	return qdspUpdate(plot, x, y, color, numPoints);
}

void qdspRedraw(QDSPplot *plot) {
	glfwMakeContextCurrent(plot->window);
	
	glClear(GL_COLOR_BUFFER_BIT);

	// grid
	if (plot->grid) {
		glUseProgram(plot->gridProgram);

		glUniform1i(glGetUniformLocation(plot->gridProgram, "useY"), 0);
		glBindVertexArray(plot->gridVAOx);
		glDrawArrays(GL_LINES, 0, 4 * plot->numGridX);

		glUniform1i(glGetUniformLocation(plot->gridProgram, "useY"), 1);
		glBindVertexArray(plot->gridVAOy);
		glDrawArrays(GL_LINES, 0, 4 * plot->numGridY);
	}
	
	// points
	glUseProgram(plot->pointsProgram);
	glBindVertexArray(plot->pointsVAO);
	glDrawArrays(plot->connected ? GL_LINE_STRIP : GL_POINTS,
	             0, plot->numPoints);

	// labels
	if (plot->grid) {
		glUseProgram(plot->textProgram);
		glBindTexture(GL_TEXTURE_2D, plot->numTexture);
			
		glBindVertexArray(plot->textVAOx);
		glDrawArrays(GL_TRIANGLES, 0, 6 * 10 * plot->numGridX);

		glBindVertexArray(plot->textVAOy);
		glDrawArrays(GL_TRIANGLES, 0, 6 * 10 * plot->numGridY);
	}
	
	// help overlay
	if (plot->overlay) {
		glUseProgram(plot->overlayProgram);
		glBindVertexArray(plot->overlayVAO);
		glBindTexture(GL_TEXTURE_2D, plot->overlayTexture);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
	
	glfwSwapBuffers(plot->window);
}

void qdspSetFramerate(QDSPplot *plot, double framerate) {
	if (framerate <= 0)
		plot->frameInterval = 0;
	else
		// frameInterval is specified in ms
		plot->frameInterval = 1000.0 / framerate;
}

void qdspSetBounds(QDSPplot *plot, double xMin, double xMax, double yMin, double yMax) {
	glfwMakeContextCurrent(plot->window);
	
	glUseProgram(plot->pointsProgram);
	glUniform1f(glGetUniformLocation(plot->pointsProgram, "xMin"), xMin);
	glUniform1f(glGetUniformLocation(plot->pointsProgram, "xMax"), xMax);
	glUniform1f(glGetUniformLocation(plot->pointsProgram, "yMin"), yMin);
	glUniform1f(glGetUniformLocation(plot->pointsProgram, "yMax"), yMax);
	plot->xMin = xMin;
	plot->xMax = xMax;
	plot->yMin = yMin;
	plot->yMax = yMax;

	// if no grid has been set, we need to make sure it doesn't look like crap
	if (plot->xAutoGrid)
		qdspSetGridX(plot, xMin, (xMax - xMin) / 4, 0x000000);

	if (plot->yAutoGrid)
		qdspSetGridY(plot, yMin, (yMax - yMin) / 4, 0x000000);
}

void qdspSetConnected(QDSPplot *plot, int connected) {
	plot->connected = connected;
}

void qdspSetPointSize(QDSPplot *plot, int pixels) {
	glfwMakeContextCurrent(plot->window);
	
	glUseProgram(plot->pointsProgram);
	glUniform1i(glGetUniformLocation(plot->pointsProgram, "pointSize"), pixels);
}

void qdspSetPointAlpha(QDSPplot *plot, double alpha) {
	glfwMakeContextCurrent(plot->window);
	
	glUseProgram(plot->pointsProgram);
	glUniform1f(glGetUniformLocation(plot->pointsProgram, "alpha"), alpha);
}

void qdspSetPointColor(QDSPplot *plot, int rgb) {
	glfwMakeContextCurrent(plot->window);
	
	glUseProgram(plot->pointsProgram);
	glUniform1i(glGetUniformLocation(plot->pointsProgram, "defaultColor"), rgb);
}

void qdspSetBGColor(QDSPplot *plot, int rgb) {
	glfwMakeContextCurrent(plot->window);

	glClearColor((0xff & rgb >> 16) / 255.0,
	             (0xff & rgb >> 8) / 255.0,
	             (0xff & rgb) / 255.0,
	             1.0f);
}

void qdspSetGridX(QDSPplot *plot, double point, double interval, int rgb) {
	glfwMakeContextCurrent(plot->window);
	
	if (interval <= 0) return;

	plot->xAutoGrid = 0;
	
	int iMin = (int)ceil((plot->xMin - point) / interval);
	int iMax = (int)floor((plot->xMax - point) / interval);
	int numLines = (iMax - iMin + 1);
	plot->numGridX = numLines;
	
	float *coords = malloc(4 * numLines * sizeof(float));

	// 10 chars/label, 36 floats/char
	float *labels = malloc(10 * 36 * numLines * sizeof(float));

	for (int i = 0; i < numLines; i++) {
		double x = point + (iMin + i) * interval;
		double xNorm = 2 * (x - plot->xMin) / (plot->xMax - plot->xMin) - 1;
		coords[4*i + 0] = xNorm;
		coords[4*i + 1] = -1;
		coords[4*i + 2] = xNorm;
		coords[4*i + 3] = 1;

		char str[11];
		snprintf(str, 11, "% .3e", x);
		
		int off = (i == 0); // to prevent overlap, offset bottom left by 1
		for (int j = 0; j < 10; j++)
			// compute position and texture data for each character
			charHelper(&labels[36*(10*i + j)], xNorm, -1, off + j, 0, str[j]);
	}

	glUseProgram(plot->gridProgram);
	
	// pass x,y
	glBindVertexArray(plot->gridVAOx);
	glBindBuffer(GL_ARRAY_BUFFER, plot->gridVBOx);
	glBufferData(GL_ARRAY_BUFFER, 4 * numLines * sizeof(float), coords, GL_STATIC_DRAW);

	// pass label data
	glBindVertexArray(plot->textVAOx);
	glBindBuffer(GL_ARRAY_BUFFER, plot->textVBOx);
	glBufferData(GL_ARRAY_BUFFER, 10 * 36 * numLines * sizeof(float), labels, GL_STATIC_DRAW);
	
	// pass rgba
	glUniform4f(glGetUniformLocation(plot->gridProgram, "xColor"),
	            (0xff & rgb >> 16) / 255.0,
	            (0xff & rgb >> 8) / 255.0,
	            (0xff & rgb) / 255.0,
	            1.0f);
	
	free(coords);
	free(labels);
}

void qdspSetGridY(QDSPplot *plot, double point, double interval, int rgb) {
	glfwMakeContextCurrent(plot->window);
	
	if (interval <= 0) return;

	plot->yAutoGrid = 0;

	int iMin = (int)ceil((plot->yMin - point) / interval);
	int iMax = (int)floor((plot->yMax - point) / interval);
	int numLines = (iMax - iMin + 1);
	plot->numGridY = numLines;
	
	float *coords = malloc(4 * numLines * sizeof(float));
	// 10 chars/label, 36 floats/char
	float *labels = malloc(10 * 36 * numLines * sizeof(float));
	
	for (int i = 0; i < numLines; i++) {
		double y = point + (iMin + i) * interval;
		double yNorm = 2 * (y - plot->yMin) / (plot->yMax - plot->yMin) - 1;
		coords[4*i + 0] = -1;
		coords[4*i + 1] = yNorm;
		coords[4*i + 2] = 1;
		coords[4*i + 3] = yNorm;

		char str[11];
		snprintf(str, 11, "% .3e", y);
		
		int off = (i == 0); // to prevent overlap, offset bottom left by 1
		for (int j = 0; j < 10; j++)
			// compute position and texture data for each character
			charHelper(&labels[36*(10*i + j)], -1, yNorm, j, off, str[j]);
	}

	glUseProgram(plot->gridProgram);
	
	// pass x,y
	glBindVertexArray(plot->gridVAOy);
	glBindBuffer(GL_ARRAY_BUFFER, plot->gridVBOy);
	glBufferData(GL_ARRAY_BUFFER, 4 * numLines * sizeof(float), coords, GL_STATIC_DRAW);

	// pass label data
	glBindVertexArray(plot->textVAOy);
	glBindBuffer(GL_ARRAY_BUFFER, plot->textVBOy);
	glBufferData(GL_ARRAY_BUFFER, 10 * 36 * numLines * sizeof(float), labels, GL_STATIC_DRAW);
	
	// pass rgba
	glUniform4f(glGetUniformLocation(plot->gridProgram, "yColor"),
	            (0xff & rgb >> 16) / 255.0,
	            (0xff & rgb >> 8) / 255.0,
	            (0xff & rgb) / 255.0,
	            1.0f);	
	
	free(coords);
	free(labels);
}

// each character in the grid labels requires 2 triangles with spatial and texture
// coords. this function computes those given a character and its position in the
// label
static void charHelper(float *addr, float x0, float y0, int xoff, int yoff, char ch) {
	// get location of ch in image, which contains "0123456789.+-e "
	int charIdx;
	if ('0' <= ch && ch <= '9') charIdx = ch - '0';
	else if (ch == '.') charIdx = 10;
	else if (ch == '+') charIdx = 11;
	else if (ch == '-') charIdx = 12;
	else if (ch == 'e' || ch == 'E') charIdx = 13;
	else charIdx = 14; // default to ' '

	// coordinates for vertices of each triangle
	//          tri1       tri2
	int dx[] = {0, 1, 0,   1, 0, 1};
	int dy[] = {0, 0, 1,   0, 1, 1};

	for (int i = 0; i < 6; i++) {
		// xbase, ybase (same for all)
		addr[6*i + 0] = x0;
		addr[6*i + 1] = y0;
		// xoff, yoff
		addr[6*i + 2] = xoff + dx[i];
		addr[6*i + 3] = yoff + dy[i];
		// tex coords
		addr[6*i + 4] = (charIdx + dx[i]) / 15.0;
		addr[6*i + 5] = 1 - dy[i];
	}
}

static void closeCallback(GLFWwindow *window) {
	QDSPplot *plot = glfwGetWindowUserPointer(window);
	plot->paused = 0;
}

static void resizeCallback(GLFWwindow *window, int width, int height) {
	QDSPplot *plot = glfwGetWindowUserPointer(window);
	glfwMakeContextCurrent(window);

	glUseProgram(plot->overlayProgram);
	glUniform2f(glGetUniformLocation(plot->overlayProgram, "pixDims"),
	            width, height);

	glUseProgram(plot->textProgram);
	glUniform2f(glGetUniformLocation(plot->textProgram, "pixDims"),
	            width, height);

	glViewport(0, 0, width, height);

	if (plot->paused)
		qdspRedraw(plot);
}

static void updateTitle(QDSPplot *plot) {
	// when frozen or paused, we need to toggle [f] and [p] in the titlebar
	int titleLen = strlen(plot->title);
	char *curTitle = malloc((titleLen + 10) * sizeof(char));
	memcpy(curTitle, plot->title, titleLen + 1);

	if (plot->frozen)
		strcat(curTitle, " [f]");
	if (plot->paused)
		strcat(curTitle, " [p]");

	glfwSetWindowTitle(plot->window, curTitle);
	
	free(curTitle);
}

static void keyCallback(GLFWwindow *window, int key, int code, int action, int mods) {
	QDSPplot *plot = glfwGetWindowUserPointer(window);
	glfwMakeContextCurrent(window);
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
		updateTitle(plot);
	}

	// f - freeze
	if (key == GLFW_KEY_F && action == GLFW_PRESS) {
		plot->frozen = !plot->frozen;
		updateTitle(plot);
	}
	
	// h - display help
	if (key == GLFW_KEY_H && action == GLFW_PRESS) {
		plot->overlay = !plot->overlay;

		// redraw so the user can toggle the overlay while paused
		qdspRedraw(plot);
	}

	// g - toggle grid
	if (key == GLFW_KEY_G && action == GLFW_PRESS) {
		plot->grid = !plot->grid;
		qdspRedraw(plot);
	}
}

static int makeShader(const char *filename, GLenum type) {
	// will fail with a crazy-long filename, but users can't call this anyway
	char fullpath[256];
	resourcePath(fullpath, filename);
	FILE *file = fopen(fullpath, "r");
	
	// try current directory if it failed
	if (file == NULL) {
		file = fopen(filename, "r");
	}
	
	if (file == NULL) {
		fprintf(stderr, "Could not find file: %s\n", fullpath);
		fprintf(stderr, "Could not find file: ./%s\n", filename);
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

// load image with SOIL and pass to currently bound texture
static int loadTexture(const char *relpath, int *width, int *height) {
	// width or height could be null, so it's safer to use our own
	int imgWidth, imgHeight;
	
	char fullpath[256];
	resourcePath(fullpath, relpath);

	unsigned char *imgData = SOIL_load_image(fullpath, &imgWidth, &imgHeight,
	                                         NULL, SOIL_LOAD_RGBA);
	if (imgData == NULL)
		// loading failed, try current dir
		imgData = SOIL_load_image(relpath, &imgWidth, &imgHeight,
		                          NULL, SOIL_LOAD_RGBA);
		
	if (imgData == NULL) { // file is nowhere
		fprintf(stderr, "Could not find file: %s\n", fullpath);
		fprintf(stderr, "Could not find file: %s\n", relpath);
		return 0;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imgWidth, imgHeight, 0, GL_RGBA,
	             GL_UNSIGNED_BYTE, imgData);
	
	SOIL_free_image_data(imgData);

	if (width != NULL) *width = imgWidth;
	if (height != NULL) *height = imgHeight;

	return 1;
}

static void resourcePath(char *fullpath, const char *relpath) {
#ifdef QDSP_RESOURCE_DIR
	strcpy(fullpath, QDSP_RESOURCE_DIR);
#else
	// default when nothing specified
	strcpy(fullpath, "/usr/local/share/qdsp/");
#endif

	strcat(fullpath, relpath);
}
