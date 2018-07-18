#!/usr/bin/python3

import numpy as np
from qdsp import QDSPplot

x = np.arange(0, 10, 0.1)
y0 = np.sin(2 * np.pi * x/10)

plot = QDSPplot('Title')
plot.setBounds(0,10, -2,2)

t = 0
cont = 1

while cont:
	y = y0 * np.cos(np.pi * t / 200.0)
	cont = plot.updateWait(x, y)
	t += 1
