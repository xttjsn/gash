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
