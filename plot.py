# for testing purposes
# not at all needed for build
import matplotlib.pyplot as plt
import json

with open("out.json") as f:
    dat = json.load(f)['data']

plt.subplot(2, 1, 1, title='in')
plt.scatter(y=dat[0], x=range(len(dat[0])))
plt.plot(
    dat[0],
)

plt.subplot(2, 1, 2, title='out')
plt.scatter(y=dat[1], x=range(len(dat[1])))
plt.plot(
    dat[1],
)
plt.show()
