import numpy as np
import matplotlib.pyplot as plt

fft = np.loadtxt('data.txt')
freq = np.fft.fftfreq(1024)
plt.plot(freq, fft[:, 0])
plt.plot(freq, fft[:, 1])
plt.savefig('fft.png')
plt.show()
