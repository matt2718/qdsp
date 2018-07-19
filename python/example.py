#!/usr/bin/python3

import numpy as np
from qdsp import QDSPplot

x = np.arange(0, 10, 0.2)
y0 = np.sin(2 * np.pi * x/10)

# create
plot = QDSPplot('QDSP Python Example')

# xmin, xmax, ymin, ymax
plot.setBounds(0,10, -2,2)

# set grid dimensions
# parameters are:
#   location of a single grid line
#   interval between grid lines
#   grid color
# grid lines can be toggled by pressing 'g'
plot.setGridX(0, 2, 0x444444)
plot.setGridY(0, 1, 0x444444)

# yellowish-green points, 2 pixels wide
plot.setPointColor(0x88ff00)
plot.setPointSize(2)

t = 0
cont = 1

while cont:
	y = y0 * np.cos(np.pi * t / 200.0)

	# updateWait waits until a frame has passed and updates the plot
	# (this prevents the program from sending a massive amount of
	# useless draw calls). You can change this behavior by using
	# update or updateIfReady.
	# 
	# The function returns 0 if the user closes the window
	cont = plot.updateWait(x, y)
	
	t += 1
