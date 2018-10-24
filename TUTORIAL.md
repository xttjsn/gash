##Tutorial for gash

This tutorial shows how to write simple arithmetic operations in `gash-lang`, generate the circuit,
and execute it under SMC framework.

Note: this tutorial only works for Fedora 27/28.

### Clone the repository (Recursively)
Run `git clone --recursive git@github.com:xttjsn/gash.git`

### Install dependencies
Run `./install_dependencies.sh`

### Build the project
Run `make` (This could take a while)
If the first build fails, please run `make clean` and run `make` again. The second build usually
works.

### Install the library
Run `sudo make install`

### Run the tests
Run `cd test/exec`, and then `./test_exec_<test_name>`

### Run the test for `tanh`
Run `cd test/exec`, and then `./test_exec_tanh`, the result should be something like

```
[==========] Running 1 test from 1 test case.
[----------] Global test environment set-up.
[----------] 1 test from EXECTest
[ RUN       ] EXECTest.Tanh
R:a5360eaabdcf300e 35f29c720a7308d7

connected
Receiving using 52 bytes
Receving using 414 selection bits
Sending 414 labels
Verifying 1oo2 OT
Verifying 1oo2 OT
OT Verification successful
OT Verification successful
6.71926589      8245
------ OT sender ------
Send amount: 26589
Recv amount: 8245
6.821824526597
------ OT receiver ------
Send amount: 8245
Recv amount: 26     598
11830:0
11831:1
11832:0
11833:1
11834:0
11835:1
11836:1
11837:0
11838:0
11839:1
11840:0
11841:0
11842:0
11843:0
11844:1
11845:1
11846:1
11847:0
11848:1
11849:0
11850:0
11851:0
11852:1
11853:0
11854:0
11855:1
11856:1
11857:0
11858:1
11859:0
11860:1
11861:1
11862:1
11863:1
11864:0
11865:0
11866:0
11867:1
11868:1
11869:0
11870:0
11871:0
11872:0
11873:0
11874:0
11875:0
11876:0
11877:0
11878:0
11879:0
11880:0
11881:0
11882:0
11883:0
11884:0
11885:0
11886:0
11887:0
11888:0
11889:0
11890:0
11891:0
11892:0
11893:0
11830:0
11831:1
11832:0
11833:1
11834:0
11835:1
Evaluator's sockets closed!
11836:1
11837:0
11838:0
11839:1
11840:0
11841:0
11842:0
11843:0
11844:1
11845:1
11846:1
11847:0
11848:1
11849:0
11850:0
11851:0
11852:1
11853:0
11854:0
11855:1
11856:1
11857:0
11858:1
11859:0
11860:1
11861:1
11862:1
11863:1
11864:0
11865:0
11866:0
11867:1
11868:1
11869:0
11870:0
11871:0
11872:0
11873:0
11874:0
11875:0
11876:0
11877:0
11878:0
11879:0
11880:0
11881:0
11882:0
11883:0
11884:0
11885:0
11886:0
11887:0
11888:0
11889:0
11890:0
11891:0
11892:0
11893:0
Garbler's sockets closed!
-------------------Garbler report---------------
Parsing, Build circuit, Read input, Garble circuit, Init connection, Send encrypted garbled truth tables, Send peer labels, Send self labels, Send output map, Receive output, Report output, 
38090336, 7876870, 408861, 4966254, 2035773, 179886277, 151427067, 8293342, 366906, 2725448, 400904, Accumulated send amount:204464
Accumulated recv amount:516
[       OK  ] EXECTest.Tanh (404 ms)
[----------] 1 test from EXECTest (404 ms total)

[----------] Global test environment tear-down
[==========] 1 test from 1 test case ran. (404 ms total)
[  PASSED   ] 1 test.
```

The list of values have the following meaning:
`wire:val`, where `wire` is the number of the wire and `val` is the bit value of that wire.

Then it shows report for both the garbler and the evaluator. The numbers are the time spent on the specific task (Copy it into a csv editor and you will see it better).
The unit of the time is 10^(-9)s, or ns.

