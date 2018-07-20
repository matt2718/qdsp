"""
.. module:: qdsp

"""

import numpy as np
from ctypes import *

try:
	lib = cdll.LoadLibrary('libqdsp.so')
except OSError:
	print('ERROR: could not load libqdsp.so')

class QDSPplot:
	"""This class represents a plot in QDSP, acting as a wrapper for the
	underlying QDSPplot C struct.

	In order to see a list of plot hotkeys, press 'h' while the plot is running.

	"""
	
	def __init__(self, title):
		"""
		
		:param title: The window title.

		"""
		lib.qdspInit.restype = c_void_p
		self.ptr = lib.qdspInit(title.encode('utf-8'))

	def delete(self):
		"""Destroys a plot.
		
		The plot object is freed and all resources are deleted.

		"""
		lib.qdspDelete(self.ptr)

	def redraw(self):
		"""Redraws a plot.
		
		The given plot is redrawn immediately, ignoring any specified
		framerate.

		"""
		lib.qdspRedraw(self.ptr)
		
	def setBGColor(self, rgb):
		"""Sets the background color
		
		This function sets the background color of the plot area.
		
		:param rgb: The background color, as an integer corresponding to a
		            hexadecimal RGB triplet.
		"""
		lib.qdspSetBGColor(self.ptr, rgb)
		
	def setBounds(self, xmin, xmax, ymin, ymax):
		"""Sets the x and y bounds of a plot
		
		This function sets the bounds of the plot window. The default bounds
		are (-1,-1) to (1,1).
		
		:param xMin: The x coordinate of the plot's left boundary.
		:param xMax: The x coordinate of the plot's right boundary.
		:param yMin: The y coordinate of the plot's lower boundary.
		:param yMax: The y coordinate of the plot's upper boundary.

		"""
		lib.qdspSetBounds(self.ptr, c_double(xmin), c_double(xmax),
		                  c_double(ymin), c_double(ymax))

	def setConnected(self, connected):
		"""Specifies whether to connect the plot points
		
		This function tells QDSP whether the points in the specified plot
		should be disconnected (to draw a scatter plot) or connected (to
		draw a line plot). Points will always be connected in the order they
		appear in in the input array.
		
		:param connected: Zero if the points should be disconnected, nonzero
		                  if they should be connected.

		"""
		lib.qdspSetConnected(self.ptr, connected)
		
	def setGridX(self, point, interval, rgb):
		"""Sets the locations of x gridlines
		
		This function determines the spacing of the x gridlines. Gridlines
		will be spaced the given interval apart, with one gridline placed
		exactly at the given point.
		
		:param point: A specific x-coordinate at which a gridline should be
		              placed. All gridlines will be placed an integer
		              multiple of the specified interval from this position.
		:param interval: The distance between gridlines.
		:param rgb: The background color, as an integer corresponding to a
		            hexadecimal RGB triplet.

		"""
		
		lib.qdspSetGridX(self.ptr, c_double(point), c_double(interval), rgb)
		
	def setGridY(self, point, interval, rgb):
		"""Sets the locations of y gridlines
		
		This function determines the spacing of the y gridlines. Gridlines
		will be spaced the given interval apart, with one gridline placed
		exactly at the given point.
		
		:param point: A specific y-coordinate at which a gridline should be
		              placed. All gridlines will be placed an integer
		              multiple of the specified interval from this position.
		:param interval: The distance between gridlines.
		:param rgb: The background color, as an integer corresponding to a
		            hexadecimal RGB triplet.

		"""
		lib.qdspSetGridY(self.ptr, c_double(point), c_double(interval), rgb)
		
	def setFramerate(self, framerate):
		"""Caps the update framerate of a plot
		
		This function sets a framerate for updating the specified plot,
		which will be used by @ref updateIfReady and @ref updateWait. If the
		specified framerate is less than or equal to 0, the framerate will
		be uncapped and the aforementioned functions will behave like @ref
		update when called. The default framerate is 60 FPS.
		
		:param framerate: The framerate, in frames per second.

		"""
		lib.qdspRedraw(self.ptr, c_double(framerate))

	def setPointAlpha(self, alpha):
		"""Sets the point transparency
		
		This function sets the transparency of the plotted points. The
		default alpha value is 1 (opaque).
		
		:param alpha: The alpha value to use for the points. This must be
		              between 0 (completely transparent) and 1 (completely
		              opaque), inclusive.

		"""
		lib.qdspSetPointAlpha(self.ptr, c_double(alpha))
		
	def setPointColor(self, rgb):
		"""Sets the default point color
		
		This function sets the point color to use when no color array is
		specified during an update.
		
		:param rgb: The background color, as an integer corresponding to a
		            hexadecimal RGB triplet.

		"""

		lib.qdspSetPointColor(self.ptr, rgb)
		
	def setPointSize(self, pixels):
		"""Sets the size of the plot points
		
		This function sets the size of the points drawn by QDSP. Points will
		appear as squares of the specified width. The width parameter
		defaults to 1 pixel, and will be ignored in connected mode.
		
		:param pixels: The point width, in pixels.

		"""
		lib.qdspSetPointSize(self.ptr, pixels)

	# helper function for update calls
	def __xyc2ptr(self, xvals, yvals, colors):
		# None will not be passed as NULL due to numpy
		if xvals is None or yvals is None:
			return -1
		
		size = min(len(xvals), len(yvals))
		xptr = np.asarray(xvals).ctypes.data_as(POINTER(c_double))
		yptr = np.asarray(yvals).ctypes.data_as(POINTER(c_double))
		if colors:
			cptr = np.asarray(colors).ctypes.data_as(POINTER(c_int))
			# if colors is non-NULL, it can't be too small
			size = min(size, len(colors))
		else:
			cptr = None
		
		return [xptr, yptr, cptr, size]
	
	def update(self, xvals, yvals, colors=None):
		"""Updates a plot immediately.
		
		The plot is updated with the new vertex data and immediately
		redrawn, ignoring any specified framerate. If color is NULL, all
		points will be the default color.
		
		@ref updateIfReady should be preferred in many cases, as repeated calls
		to update every frame will result in a lot of useless overhead.
		
		:param xvals: An array containing the x coordinates.
		:param yvals: An array containing the y coordinates.
		:param colors: An array containing the point colors, represented as
		               integers.

		:returns: 1 if the plot was updated successfully, 0 otherwise.

		"""
		xptr, yptr, cptr, size = self.__xyc2ptr(xvals, yvals, colors)
		return lib.qdspUpdate(self.ptr, xptr, yptr, cptr, size)
	
	def updateIfReady(self, xvals, yvals, colors=None):
		"""Updates a plot if enough time has passed since the last update.
		
		The plot is updated with the new vertex data and redrawn if at least
		1.0/framerate seconds have passed since the last call to @ref
		update, @ref updateIfReady, @ref updateWait, or @ref redraw in which
		the specified plot was redrawn. If color is NULL, all points will be
		the default color.
		
		This function should be used over @ref update in many cases, as it
		eliminates the useless overhead of copying vertex data to the GPU
		before the monitor can be refreshed.
		
		:param x: An array containing the x coordinates.
		:param y: An array containing the y coordinates.
		:param colors: An array containing the point colors, represented as
		               integers.
		
		:returns: 1 if the plot was updated successfully, 2 if plot was not ready for
		          an update, 0 otherwise.

		"""
		xptr, yptr, cptr, size = self.__xyc2ptr(xvals, yvals, colors)
		return lib.qdspUpdateIfReady(self.ptr, xptr, yptr, cptr, size)
	
	def updateWait(self, xvals, yvals, colors=None):
		"""Updates a plot after waiting for a new frame
		
		This function waits until at least 1.0/framerate seconds have passed
		since the last call to @ref update, @ref updateIfReady, @ref
		updateWait, or @ref redraw in which the specified plot was redrawn.
		It then updates the plot with the vertex data and redraws it. If
		color is NULL, all points will be the default color.
		
		This function is primarily useful when you wish to limit your code
		to a specific real-time update interval.
		
		:param xvals: An array containing the x coordinates.
		:param yvals: An array containing the y coordinates.
		:param colors: An array containing the point colors, or NULL. See
		:param colors: An array containing the point colors, represented as
		               integers.

		:returns: 1 if the plot was updated successfully, 0 otherwise.

		"""
		xptr, yptr, cptr, size = self.__xyc2ptr(xvals, yvals, colors)
		return lib.qdspUpdateWait(self.ptr, xptr, yptr, cptr, size)
