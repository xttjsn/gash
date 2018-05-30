# GArbled circuit SyntHesizer (GASH)

## Functionality

GASH is designed for compiling circuit description files (written in `gcdf`
language, which has somewhat similar syntax with C++) to binary circuits (in
`circ` format, which has similar syntax with the [And Inverter
Graph](http://fmv.jku.at/aiger/format-20070427.pdf)-AIG circuit syntax).

Then, GASH garbles the circuit described by the `circ` file (if the machine is
garbler), connects to peer, and perform Yao's protocol.

## Example - Millionaire problem

Alice holds her secret value `a` and Bob holds the secret value `b`, they want
to determine whether `a > b` without telling each other her/his secret value.
They decide to use GASH to solve the problem.

Suppose `a=1000`, Alice writes the following circuit description file
`alice.gcdf`:

```
func millionaire(int32 a, int32 b) {
    int32 ret = 0;
    if (a > b) { ret = 1; }
    return ret;
}
#definput a 1000
#defrole GARBLER
#defport 47665
```

Suppose `b=500` and Alice has ip address `178.56.1.9`, Bob writes the following
circuit description file `bob.gcdf`:

```
func millionaire(int32 a, int32 b) {
    int32 ret = 0;
    if (a > b) { ret = 1; }
    return ret;
}

#definput b 500
#defrole EVALUATOR
#defport 47665
#defip 178.56.1.9
```

Alice opens a terminal and does
```
gashlang -i alice.gcdf -o alice.circ -d alice.data
```

Bob opens a terminal and does
```
gashlang -i bob.gcdf -o bob.circ -d bob.data
```

<!-- ## More examples -->

<!-- We've constructed several example files -->

<!-- ## GCDF Syntax -->

## Install

GASH is tested on Fedora 28.

GASH requires *g++* (v8), *flex*, *bison*, *gmp*, *openssl*, *boost*
(program_options only), and [*gtest*](https://github.com/google/googletest) . Please
install those packages and their headers before trying to compile.

For Fedora users, you may call
```
./install_dependency.sh
```
to install those dependencies.

After all dependencies are installed, run
```
make
```
at the root folder of gash to build all the libraries and tests.

It also requires CPUs that support SSE4 (since we will be using the flag
`-msse4`). Most Intel and AMD CPUs support it (click
[here](https://en.wikipedia.org/wiki/SSE4#Supporting_CPUs) for an extended
reference)

## Test

To run tests, go to `gash/test`, in there are several folders which contain
different type of unit tests. Most users would like to go to the `exec` folder
and run the test executables for different tests. For example, if you want to
test the billionaire problem, go to `gash/test/exec` and run
```
./test_exec_billionaire
```

The result would look like the following
```
[==========] Running 1 test from 1 test case.
[----------] Global test environment set-up.
[----------] 1 test from EXECTest
[ RUN      ] EXECTest.Billionaire64
R:9e6da844e4c1a1fc 8d53a648ee96aa95

Connect() failed, try again later: Connection refused
Connect() failed, try again later: Connection refused
Connect() failed, try again later: Connection refused
connected
Receiving using 8 bytes
Receving using 64 selection bits
Sending 64 labels
Verifying 1oo2 OT
Verifying 1oo2 OT
OT Verification successful
OT Verification successful
7.856	4180	2101
------ OT sender ------
Send amount: 4181
Recv amount: 2101
8.032	2109	4198
------ OT receiver ------
Send amount: 2109
Recv amount: 4198
707:1
Evaluator's sockets closed!
707:1
Garbler's sockets closed!
[       OK ] EXECTest.Billionaire64 (182 ms)
[----------] 1 test from EXECTest (182 ms total)

[----------] Global test environment tear-down
[==========] 1 test from 1 test case ran. (182 ms total)
[  PASSED  ] 1 test.
[       OK ] EXECTest.Billionaire64 (182 ms)
[----------] 1 test from EXECTest (182 ms total)

[----------] Global test environment tear-down
[==========] 1 test from 1 test case ran. (182 ms total)
[  PASSED  ] 1 test.
```

Notice that there are two lines that say
```
707: 1
707: 1
```

The first line is the output of the evaluator, meaning that the value of wire
707 is 1. The second line is the output of the garbler. That is why the two
lines are the same (since the output is always the same).
