import argparse
import string
import numpy as np
def generate_exp_gash_code(nbit):
    start = -20
    end   = 20
    ntaus = 100
    nscale = 20
    taus = np.linspace(start, end, ntaus)

    ks = np.zeros(ntaus - 1)
    for i in range(ntaus - 1):
        ks[i] = (np.exp(taus[i+1]) - np.exp(taus[i])) / (taus[i+1] - taus[i])

    bs = np.zeros(ntaus - 1)
    for i in range(ntaus - 1):
        bs[i] = np.exp(taus[i+1]) - ks[i] * taus[i+1]

    def scale(d):
        return np.floor(d * (1 << nscale))

    def gen_next_level_code(i):
        s = "\n" + "    " * i
        if i < ntaus - 2:
            s += "    if (sum > %d && sum < %d) { ret = prod%d; }\n" % (scale(taus[i]), scale(taus[i+1]), i+1)
            s += "    " * i
            s += "    else { %s " % (gen_next_level_code(i+1))
            s += "    " * (i+1) +  "}\n"
        else:
            s += "    if (sum > %d ) { ret = prod%d}\n"  % (scale(taus[i]), i+1)
        return s

    code  = "func ss_exp(intXXX x0, intXXX x1, intXXX r, \n"
    for i in range(ntaus - 1):
        if i != ntaus - 2:
            code += "intXXX prod%d, " % i
        else:
            code += "intXXX prod%d" % i
        if i % 10 == 0 and i != 0:
            code += "\n"
    code += ") {\n"
    code += "    intXXX ret = 0;                           \n"
    code += "    intXXX sum = x0 + x1;                     \n"
    code += "    if (sum < %d) { ret = prod%d }    \n    else { %s    } " % (scale(taus[0]), 0, gen_next_level_code(1))
    code += "\n    ret = ret - r;                          \n"
    code += "    return ret; \n}"

    code = string.replace(code, "XXX", str(nbit))
    return code

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('bitsize', metavar='N', type=int)
    args = parser.parse_args()
    print generate_exp_gash_code(args.bitsize)
