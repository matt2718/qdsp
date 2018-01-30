#!/usr/bin/python3

import ctypes

try:
	lib = ctypes.cdll.LoadLibrary('libqdsp.so')
except OSError:
	print('ERROR: could not load libqdsp.so')
