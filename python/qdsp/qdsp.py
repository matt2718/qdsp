import numpy as np
from ctypes import *

try:
	lib = cdll.LoadLibrary('libqdsp.so')
except OSError:
	print('ERROR: could not load libqdsp.so')

class QDSPplot:
	ptr = None
	def __init__(self, title):
		# QDSPplot *qdspInit(const char *title);
		lib.qdspInit.restype = c_void_p
		self.ptr = lib.qdspInit(title.encode('utf-8'))

	def delete(self):
		# void qdspDelete(QDSPplot *plot);
		lib.qdspDelete(self.ptr)

	def redraw(self):
		# void qdspRedraw(QDSPplot *plot);
		lib.qdspRedraw(self.ptr)
		
	def setBGColor(self, rgb):
		# void qdspSetBGColor(QDSPplot *plot, int rgb);
		lib.qdspSetBGColor(self.ptr, rgb)
		
	def setBounds(self, xmin, xmax, ymin, ymax):
		# void qdspSetBounds(QDSPplot *plot, double xMin, double xMax,
		#                    double yMin, double yMax);
		lib.qdspSetBounds(self.ptr, c_double(xmin), c_double(xmax),
		                  c_double(ymin), c_double(ymax))

	def setConnected(self, connected):
		# void qdspSetConnected(QDSPplot *plot, int connected);
		lib.qdspSetConnected(self.ptr, connected)
		
	def setGridX(self, point, interval, rgb):
		# void qdspSetGridX(QDSPplot *plot, double point, double interval,
		#                   int rgb);
		lib.qdspSetGridX(self.ptr, c_double(point), c_double(interval), rgb)
		
	def setGridY(self, point, interval, rgb):
		# void qdspSetGridY(QDSPplot *plot, double point, double interval,
		#                   int rgb);
		lib.qdspSetGridY(self.ptr, c_double(point), c_double(interval), rgb)
		
	def setFramerate(self, framerate):
		# void qdspSetFramerate(QDSPplot *plot, double framerate);
		lib.qdspRedraw(self.ptr, c_double(framerate))

	def setPointAlpha(self, alpha):
		# void qdspSetPointAlpha(QDSPplot *plot, double alpha);
		lib.qdspSetPointAlpha(self.ptr, c_double(alpha))
		
	def setPointColor(self, rgb):
		# void qdspSetPointColor(QDSPplot *plot, int rgb);
		lib.qdspSetPointColor(self.ptr, rgb)
		
	def setPointSize(self, pixels):
		# void qdspSetPointSize(QDSPplot *plot, int pixels);
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
		# int qdspUpdate(QDSPplot *plot, double *x, double *y, int *color,
		#                int numPoints);
		xptr, yptr, cptr, size = self.__xyc2ptr(xvals, yvals, colors)
		return lib.qdspUpdate(self.ptr, xptr, yptr, cptr, size)
	
	def updateIfReady(self, xvals, yvals, colors=None):
		# int qdspUpdateIfReady(QDSPplot *plot, double *x, double *y,
		#                       int *color, int numPoints);
		xptr, yptr, cptr, size = self.__xyc2ptr(xvals, yvals, colors)
		return lib.qdspUpdateIfReady(self.ptr, xptr, yptr, cptr, size)
	
	def updateWait(self, xvals, yvals, colors=None):
		# int qdspUpdateWait(QDSPplot *plot, double *x, double *y,
		#                    int *color, int numPoints);
		xptr, yptr, cptr, size = self.__xyc2ptr(xvals, yvals, colors)
		return lib.qdspUpdateWait(self.ptr, xptr, yptr, cptr, size)
