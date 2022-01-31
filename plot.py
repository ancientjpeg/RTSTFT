# for testing purposes
# not at all needed for build
from operator import mod
import matplotlib.pyplot as plt
import json
from math import floor, sin
import math
from numpy import random
from cmath import phase


def math_help(Ma=1024, N=256, F=4, S=2 ** (1 / 12)):
    HopA = N / F
    m = (Ma - N) / HopA + 1
    HopS = HopA * S
    HopSR = round(HopS)
    HopSF = floor(HopS)
    Ms = lambda hop: (m - 1) * hop + N

    real_pos = 0
    int_pos = 0
    hop_pos = 0
    buf_pos = 0
    mod_track = 0
    print(HopSR)
    for i in range(5000):
        bufSize = random.randint(5, 150)
        getSize = bufSize * HopSR / HopA
        getSizeInt = floor(getSize)
        mod_track += getSize % 1
        if mod_track >= 1:
            getSizeInt += 1
            mod_track -= 1
        real_pos += getSize
        int_pos += getSizeInt
        hop_pos = (hop_pos + getSizeInt) % HopSR
        buf_pos += bufSize

    print(
        f"{real_pos=}    {int_pos=}    {mod_track=}    {hop_pos=}    {int_pos%HopSR=}    {buf_pos=}    {buf_pos % HopA=}"
    )


def plot_json():
    with open("out.json") as f:
        dat = json.load(f)["data"]

    print(len(dat[0]) // 2)
    dat[0] = [
        phase(complex(dat[0][i + 1], dat[0][len(dat[0]) - (i + 1)]))
        for i in range(len(dat[0]) // 4)
    ]
    print(len(dat[0]))
    # plt.subplot(2, 1, 1, title="in")
    plt.scatter(y=dat[0], x=range(len(dat[0])))
    plt.plot(
        dat[0],
    )

    # plt.subplot(2, 1, 2, title="out")
    # plt.scatter(y=dat[1], x=range(len(dat[1])))
    # plt.plot(
    #     dat[1],
    # )
    plt.show()


def math_2():
    for i in range(3, 10):
        k = 2 ** i
        z = 3.3
        k_amod = mod(z + math.pi, 2 * math.pi) - math.pi
        ka_mod = mod(z * k + math.pi, 2 * math.pi) - math.pi
        print(f"{k_amod=}, {ka_mod=}")


# math_help()
# math_2()
plot_json()
