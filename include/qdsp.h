/**
 * @file qdsp.h
 * @brief Functions for QDSP
 * @author Matt Mitchell
 *
 * This file contains all public function prototypes for QDSP.
 */

#ifndef _QDSP_H
#define _QDSP_H

#include <GLFW/glfw3.h>

typedef struct QDSPplot {
	GLFWwindow *window;

	char *title;
	
	int paused;
	int frozen;
	int overlay;
	int grid;

	int xAutoGrid, yAutoGrid;

	// frame info
	struct timespec lastUpdate;
	double frameInterval;

	// bounds, we could probably use glGetUniform, but storing them is easier
	double xMin, xMax;
	double yMin, yMax;

	int connected;
	
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

	int textProgram;
	unsigned int textVAOx;
	unsigned int textVBOx;
	unsigned int textVAOy;
	unsigned int textVBOy;
	unsigned int numTexture;

	int overlayProgram;
	unsigned int overlayVAO;
	unsigned int overlayVBO;
	unsigned int overlayTexture;

	// needed so we can redraw at will
	int numPoints;
	int numGridX;
	int numGridY;

} QDSPplot;


/** Destroys a plot.
 *
 * The plot object is freed and all resources are deleted.
 *
 * @param plot The plot to destroy.
 *
 * @see @ref qdspInit
 */
void qdspDelete(QDSPplot *plot);

/** Creates a new plot.
 *
 * A new plot is created in a window with the given title.
 *
 * In order to see a list of plot hotkeys, press 'h' while the plot is running.
 *
 * @param title The window title.
 *
 * @return A pointer to the plot handle, or NULL if a plot could not be created.
 *
 * @see @ref qdspDelete
 */
QDSPplot *qdspInit(const char *title);

/** Redraws a plot.
 *
 * The given plot is redrawn immediately, ignoring any specified framerate.
 *
 * @param plot The plot to redraw.
 *
 * @see @ref qdspUpdate
 * @see @ref qdspUpdateIfReady
 * @see @ref qdspUpdateWait
 */
void qdspRedraw(QDSPplot *plot);

/** Sets the background color
 *
 * This function sets the background color of the plot area.
 *
 * @param plot The plot to act on.
 * @param rgb The background color, as an integer. Bits 23-16 specify the red
 *   component, bits 15-8 specify the green component, and bits 7-0 specify the
 *   blue component. All components are interpreted as 8-bit unsigned integers.
 *
 * @see @ref qdspSetPointAlpha
 * @see @ref qdspSetPointColor
 * @see @ref qdspSetPointSize
 */
void qdspSetBGColor(QDSPplot *plot, int rgb);

/** Sets the x and y bounds of a plot
 *
 * This function sets the bounds of the plot window. The default bounds are
 * (-1,-1) to (1,1).
 *
 * @param plot The plot to act on.
 * @param xMin The x coordinate of the plot's left boundary.
 * @param xMax The x coordinate of the plot's right boundary.
 * @param yMin The y coordinate of the plot's lower boundary.
 * @param yMax The y coordinate of the plot's upper boundary.
 */
void qdspSetBounds(QDSPplot *plot, double xMin, double xMax, double yMin, double yMax);

/** Specifies whether to connect the plot points
 * 
 * This function tells QDSP whether the points in the specified plot should be
 * disconnected (to draw a scatter plot) or connected (to draw a line plot).
 * Points will always be connected in the order they appear in in the input
 * array.
 *
 * @param plot The plot to act on.
 * @param connected Zero if the points should be disconnected, nonzero if they
 *   should be connected.
 */
void qdspSetConnected(QDSPplot *plot, int connected);

/** Sets the locations of x gridlines
 *
 * This function determines the spacing of the x gridlines. Gridlines will be
 * spaced the given interval apart, with one gridline placed exactly at the
 * given point.
 * 
 * @param plot The plot to act on.
 * @param point A specific x-coordinate that a gridline should be placed at. All
 *   gridlines will be placed an integer multiple of the specified interval from
 *   this position.
 * @param interval The distance between gridlines.
 * @param rgb The point color. See @ref qdspSetBGColor for a description of the
 *   color format.
 *
 * @see @ref qdspSetGridY
 */
void qdspSetGridX(QDSPplot *plot, double point, double interval, int rgb);

/** Sets the locations of y gridlines
 *
 * This function determines the spacing of the y gridlines. Gridlines will be
 * spaced the given interval apart, with one gridline placed exactly at the
 * given point.
 * 
 * @param plot The plot to act on.
 * @param point A specific y-coordinate that a gridline should be placed at. All
 *   gridlines will be placed an integer multiple of the specified interval from
 *   this position.
 * @param interval The distance between gridlines.
 * @param rgb The point color. See @ref qdspSetBGColor for a description of the
 *   color format.
 *
 * @see @ref qdspSetGridX
 */
void qdspSetGridY(QDSPplot *plot, double point, double interval, int rgb);

/** Caps the update framerate of a plot
 *
 * This function sets a framerate for updating the specified plot, which will be
 * used by @ref qdspUpdateIfReady and @ref qdspUpdateWait. If the specified
 * framerate is less than or equal to 0, the framerate will be uncapped and the
 * aforementioned functions will behave like @ref qdspUpdate when called.
 *
 * The default framerate is 60 FPS.
 *
 * @param plot The plot to act on.
 * @param framerate The framerate, in frames per second.
 *
 * @see @ref qdspUpdateIfReady
 * @see @ref qdspUpdateWait
 */
void qdspSetFramerate(QDSPplot *plot, double framerate);

/** Sets the point transparency
 * 
 * This function sets the transparency of the plotted points.
 *
 * @param plot The plot to act on.
 * @param alpha The alpha value to use for the points. This must be between 0
 *   (completely transparent) and 1 (completely opaque), inclusive.
 *
 * @see @ref qdspSetBGColor
 * @see @ref qdspSetPointColor
 * @see @ref qdspSetPointSize
 */
void qdspSetPointAlpha(QDSPplot *plot, double alpha);

/** Sets the default point color
 * 
 * This function sets the point color to use when no color array is specified
 * during an update.
 *
 * @param plot The plot to act on.
 * @param rgb The point color. See @ref qdspSetBGColor for a description of the
 *   color format.
 *
 * @see @ref qdspSetBGColor
 * @see @ref qdspSetPointAlpha
 * @see @ref qdspSetPointSize
 */
void qdspSetPointColor(QDSPplot *plot, int rgb);

/** Sets the size of the plot points
 * 
 * This function sets the size of the points drawn by QDSP. Points will appear
 * as squares of the specified width. The width parameter defaults to 1 pixel,
 * and will be ignored in connected mode.
 *
 * @param plot The plot to act on.
 * @param pixels The point width, in pixels.
 *
 * @see @ref qdspSetBGColor
 * @see @ref qdspSetPointAlpha
 * @see @ref qdspSetPointColor
 */
void qdspSetPointSize(QDSPplot *plot, int pixels);

/** Updates a plot immediately.
 *
 * The plot is updated with the new vertex data and immediately redrawn,
 * ignoring any specified framerate. If color is NULL, all points will be the
 * default color.
 *
 * @ref qdspUpdateIfReady should be preferred in many cases, as repeated calls
 * to qdspUpdate per frame will result in a lot of useless overhead.
 *
 * @param plot The plot to update.
 * @param x An array containing the x coordinates.
 * @param y An array containing the y coordinates.
 * @param color An array containing the point colors, or NULL. See
 *   @ref qdspSetBGColor for a description of the color format.
 * @param numPoints The number of points to render.
 *
 * @return 1 if the plot was updated successfully, 0 otherwise.
 *
 * @see @ref qdspUpdateIfReady
 * @see @ref qdspUpdateWait
 * @see @ref qdspRedraw
 */
int qdspUpdate(QDSPplot *plot, double *x, double *y, int *color, int numPoints);

/** Updates a plot if enough time has passed since the last update.
 *
 * The plot is updated with the new vertex data and redrawn if at least
 * 1.0/framerate seconds have passed since the last call to @ref qdspUpdate,
 * @ref qdspUpdateIfReady, @ref qdspUpdateWait, or @ref qdspRedraw in which
 * the specified plot was redrawn. If color is NULL, all points will be the
 * default color.
 *
 * This function should be used over @ref qdspUpdate in many cases, as it
 * eliminates the useless overhead of copying vertex data to the GPU before the
 * monitor can be refreshed.
 *
 * @param plot The plot to update.
 * @param x An array containing the x coordinates.
 * @param y An array containing the y coordinates.
 * @param color An array containing the point colors, or NULL. See
 *   @ref qdspSetBGColor for a description of the color format.
 * @param numPoints The number of points to render.
 *
 * @return 1 if the plot was updated successfully, 2 if plot was not ready for
 * an update, 0 otherwise.
 *
 * @see @ref qdspUpdate
 * @see @ref qdspUpdateWait
 * @see @ref qdspRedraw
 * @see @ref qdspSetFramerate
 */
int qdspUpdateIfReady(QDSPplot *plot, double *x, double *y, int *color, int numPoints);

/** Updates a plot after waiting for a new frame
 *
 * This function waits until at least 1.0/framerate seconds have passed since
 * the last call to @ref qdspUpdate, @ref qdspUpdateIfReady,
 * @ref qdspUpdateWait, or @ref qdspRedraw in which the specified plot was
 * redrawn. It then updates the plot with the vertex data and redraws it. If
 * color is NULL, all points will be the default color.
 *
 * This function is primarily useful when you wish to limit your code to a
 * specific real-time update interval.
 *
 * @param plot The plot to update.
 * @param x An array containing the x coordinates.
 * @param y An array containing the y coordinates.
 * @param color An array containing the point colors, or NULL. See
 *   @ref qdspSetBGColor for a description of the color format.
 * @param numPoints The number of points to render.
 *
 * @return 1 if the plot was updated successfully, 0 otherwise.
 *
 * @see @ref qdspUpdate
 * @see @ref qdspUpdateIfReady
 * @see @ref qdspRedraw
 * @see @ref qdspSetFramerate
 */
int qdspUpdateWait(QDSPplot *plot, double *x, double *y, int *color, int numPoints);

#endif
