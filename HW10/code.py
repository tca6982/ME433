from ulab import numpy as np # to get access to ulab numpy functions

t = np.arange(1024)
f = np.sin(t) + np.sin(2*t) + np.sin(3*t)
sp = np.fft.fft(f)

for r, i in zip(sp[0], sp[1]):
    print(r, i)
