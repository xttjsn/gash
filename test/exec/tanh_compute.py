from math import *
import random

def compute_ts():
    n1 = -0.2716
    n2 = -0.0848
    c1 = 1
    c2 = 0.42654
    d1 = 0.016
    d2 = 0.4519

    x = 3.1415926
    scale = 2**30
    rand_scale = 2**40
    ringsize = 2**60

    bigX = round(x * scale)
    x1   = random.randint(1, rand_scale)
    x2   = bigX - x1

    r    = random.randint(1, rand_scale)

    t11 = n1 * x1**2 / scale + c1 * x1 + 0 * (d1 * scale)
    t12 = n1 * x2**2 / scale + c1 * x2 + 1 * (d1 * scale)

    t21 = n2 * x1**2 / scale + c2 * x1 + 0 * (d2 * scale)
    t22 = n2 * x2**2 / scale + c2 * x2 + 1 * (d2 * scale)

    t31 = -n1 * x1**2 / scale + c1 * x1 - 0 * (d1 * scale)
    t32 = -n1 * x2**2 / scale + c1 * x2 - 1 * (d1 * scale)

    t41 = -n2 * x1**2 / scale + c2 * x1 - 0 * (d2 * scale)
    t42 = -n2 * x2**2 / scale + c2 * x2 - 1 * (d2 * scale)


    bigX1  = x1
    bigX2  = x2 + ringsize if x2 < 0 else x2

    bigT11 = round(t11) + ringsize if t11 < 0 else round(t11)
    bigT12 = round(t12) + ringsize if t12 < 0 else round(t12)

    bigT21 = round(t21) + ringsize if t21 < 0 else round(t21)
    bigT22 = round(t22) + ringsize if t22 < 0 else round(t22)

    bigT31 = round(t31) + ringsize if t31 < 0 else round(t31)
    bigT32 = round(t32) + ringsize if t32 < 0 else round(t32)

    bigT41 = round(t41) + ringsize if t41 < 0 else round(t41)
    bigT42 = round(t42) + ringsize if t42 < 0 else round(t42)

    print("x1: {}".format(int( bigX1 )))
    print("x2: {}".format(int( bigX2 )))

    print("t11: {}".format(int( bigT11 )))
    print("t12: {}".format(int( bigT12 )))

    print("t21: {}".format(int( bigT21 )))
    print("t22: {}".format(int( bigT22 )))

    print("t31: {}".format(int( bigT31 )))
    print("t32: {}".format(int( bigT32 )))

    print("t41: {}".format(int( bigT41 )))
    print("t42: {}".format(int( bigT42 )))

    print("r:  {}".format(int(r)))

if __name__ == '__main__':
    compute_ts()
