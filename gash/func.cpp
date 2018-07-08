//
//  func.cpp
//  gash
//
//  Created by Xiaoting Tang on 6/21/18.
//  Copyright © 2018 Xiaoting Tang. All rights reserved.
//

#include "func.hpp"

const string fsrc_billionaire =
"func billionaire (intXXX a, intXXX b) {       \n"
"    int1 ret = 0;                             \n"
"    if (a > b) { ret = 1; }                   \n"
"    return ret; }                             \n"
"#definput a input_a                           \n"
"#definput b input_b                           \n";

const string fsrc_add =
"func add (intXXX a, intXXX b) {               \n"
"    intXXX ret = a + b;                       \n"
"    return ret; }                             \n"
"#definput a input_a                           \n"
"#definput b input_b                           \n";

const string fsrc_sub =
"func sub (intXXX a, intXXX b) {               \n"
"    intXXX ret = a - b;                       \n"
"    return ret; }                             \n"
"#definput a input_a                           \n"
"#definput b input_b                           \n";

const string fsrc_mul =
"func mul (intXXX a, intXXX b) {               \n"
"    intXXX ret = a * b;                       \n"
"    return ret; }                             \n"
"#definput a input_a                           \n"
"#definput b input_b                           \n";

const string fsrc_div =
"func div (intXXX a, intXXX b) {               \n"
"   intXXX ret = a / b;                        \n"
"   return ret; }                              \n"
"#definput a input_a                           \n"
"#definput b input_b                           \n";

const string fsrc_relu =
"func relu (intXXX x) {                        \n"
"    intXXX ret = 0;                           \n"
"    if (x > 0) { ret = x; }                   \n"
"    return ret; }                             \n"
"#definput x input_x                           \n";

const string fsrc_ss_relu =
"func ssrelu (intXXX in0, intXXX in1, intXXX r) {         \n"
"    intXXX sum = in0 + in1;                              \n"
"    intXXX ret = 0;                                      \n"
"    if (sum > 0) { ret = sum - r; }                      \n"
"    else { ret = 0 - r; }                                \n"
"    return ret; }                                        \n"
"#definput in0 input_in0                                  \n"
"#definput in1 input_in1                                  \n";

const string fsrc_ss_relugrad =
"func ss_relugrad (intXXX in0, intXXX in1, intXXX r) {    \n"
"     intXXX sum = in0 + in1;                             \n"
"     intXXX ret = 0;                                     \n"
"     if (sum > 0) { ret = sum - r; }                     \n"
"     else { ret = 0 - r; }                               \n"
"     return ret; }                                       \n"
"#definput in0 input_in0                                  \n"
"#definput in1 input_in1                                  \n";

const string fsrc_ss_la =
"func ss_la(intXXX a0, intXXX b0, intXXX a1, intXXX b1) { \n"
"    intXXX a = a0 + a1;                                  \n"
"    intXXX b = b0 + b1;                                  \n"
"    int1   ret = 0;                                      \n"
"    if (a > b) { ret = 1; }                              \n"
"    return ret;                                          \n"
"#definput a0 input_a0                                    \n"
"#definput b0 input_b0                                    \n"
"#definput a1 input_a1                                    \n"
"#definput b1 input_b1                                    \n";

const string fsrc_ss_div =
"func ss_div(intXXX a0, intXXX b0, intXXX a1, intXX b1, intXXX r) { \n"
"    intXXX a = a0 + a1;                                            \n"
"    intXXX b = b0 + b1;                                            \n"
"    int div = a / b;                                               \n"
"    ret = div - r;                                                 \n"
"    return ret;                                                    \n"
"#definput a0 input_a0                                              \n"
"#definput b0 input_b0                                              \n"
"#definput a1 input_a1                                              \n"
"#definput b1 input_b1                                              \n"
"#definput r input_r                                                \n";

REGISTER_FUNC(billionaire, 2, fsrc_billionaire);
REGISTER_FUNC(add, 2, fsrc_add);
REGISTER_FUNC(sub, 2, fsrc_sub);
REGISTER_FUNC(div, 2, fsrc_div);
REGISTER_FUNC(mul, 2, fsrc_mul);
REGISTER_FUNC(relu, 1, fsrc_relu);
REGISTER_FUNC(ss_relu, 3, fsrc_ss_relu);
REGISTER_FUNC(ss_relugrad, 3, fsrc_ss_relugrad);
