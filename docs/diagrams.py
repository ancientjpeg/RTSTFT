import numpy as np
from matplotlib import pyplot as plt

sine = np.linspace(0, np.pi * 16, 256)
sine = np.sin(sine)
window = np.hanning(len(sine)) * sine

plt.figure(figsize=(12, 3))
plt.subplot(121)
plt.axis("off")
plt.title("Before Hanning Window")
plt.plot(sine, "black")
plt.subplot(122)
plt.axis("off")
plt.title("After Hanning Window")
plt.plot(window, "black")
plt.savefig("window.png", transparent=True)

sineX = np.linspace(0, 2, 512)
sineY = np.sin(sineX * np.pi * 2 + np.pi)
fig = plt.figure()
plt.plot(sineX, sineY, "black")
plt.tick_params("both", which="both", left=False)
plt.xticks([0, 1, 2])
plt.yticks([-1, 0, 1])
print(type(fig.axes[0]))
ax = fig.axes[0]
ax.set_xticklabels(["0", "π", "2π"])
plt.savefig("phi_omega.png", transparent=True)
