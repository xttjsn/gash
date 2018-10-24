# GArbled circuit SyntHesizer (GASH)

## Functionality

GASH is designed for compiling circuit description files (written in `gcdf`
language, which has somewhat similar syntax with C++) to binary circuits (in
`circ` format, which has similar syntax with the [And Inverter
Graph](http://fmv.jku.at/aiger/format-20070427.pdf)-AIG circuit syntax).

Then, GASH garbles the circuit described by the `circ` file (if the machine is
garbler), connects to peer, and perform Yao's protocol.

###Write code in `gash-lang`

`gash-lang` is a simple language with syntax similar to C. It consists of two parts, a main
function, and a few directives.

```
func FuncName(Type name, ...) {

}
#Directives
...
```

What gash will do is first compile the function into a boolean circuit, and use the directives as
hints to execute the circuits in Yao's GC framework. The directives include
- `#definput Symbol Value`. Sets the value of `Symbol` to `Value`. `Value` will be truncated if the
symbol size is less than that of `Value`.
- `#defip Ip`. Sets the *peer*'s ip address. The peer of the garbler is the evaluator, and vice
versa. It is usually better for the evaluator to connect to the garbler.
- `#defrole Role`. Sets the role of the current program, the value could be `GARBLER` or `EVALUATOR`.
- `#defport Port`. Sets the peer's port used by the GC framework.
- `#defotport Port`. Sets the peer's port used by Oblivious Transfer.

####Data Type
Gash currently has 1 generic data types `intX`, where `1<=X<=64`. All integers are signed, thus the
largest number we can represent is `1<<63`.

###Control Flow
Gash has a rather restricted control flow than C and other high-level programming language. This
is because Gash is not turing complete. Every program/circuit that gash generates finishes in a
finite number of steps. Here are the two restrictions of gash compared to C.

- No `while` and `for` loop.
- No `else if`.

However, there are (trivial) ways to achieve `for` loop with finite steps and `else if` in certain
conditions. If you want to write  
```
    for (int i = 0; i < 10; i++) {
        <doSomething i>
    }
```
where `<doSomething i>` is the code that you want to executes in the loop, parameterized by `i`. You
can write it in `gash-lang` as such.
```
        <doSomething 0>
        <doSomething 1>
        ...
        <doSomething 9>
        <doSomething 10>
```

If you want to write
```
    if (<condition 0>) {
        <doSomething 0>    
    } else if (<condition 1>) {
        <doSomething 1>
    }
    ...
    else if (<condition n-1>) {
        <doSomething n-1>
    } else  {
        <doSomething n>
    }
```

You should nest the `else if`s by writing it as follows.
```
    if (<condition 0>) {
        <doSomething 0>
    } else {
        if (<condition 1>) {
            <doSomething 1>
        } else {
            ...
                ...
                    ...
                        if (<condition n-1>) {
                            <doSomething n-1>
                        } else {
                            <doSomething n>
                        }
                    ...
                ...
            ...    
        }
    }
```

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
