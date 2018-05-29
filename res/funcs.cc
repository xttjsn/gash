/*
 * funcs.cc -- Stores commonly used functions
 *
 * Author: Xiaoting Tang <tang_xiaoting@brown.edu>
 * Copyright: Xiaoting Tang (2018)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "../include/common.hh"

namespace gashres {

    const string fsrc_billionaire =
        "func billionaire (intXXX a, intXXX b) {       "
        "    int1 ret = 0;                             "
        "    if (a > b) { ret = 1; }                   "
        "    return ret; }                             ";

    const string fsrc_add =
        "func add (intXXX a, intXXX b) {               "
        "    intXXX ret = a + b;                       "
        "    return ret; }                             ";

    const string fsrc_sub =
        "func sub (intXXX a, intXXX b) {               "
        "    intXXX ret = a - b;                       "
        "    return ret; }                             ";


    // TODO: add multiplication and division

    const string fsrc_relu =
        "func relu (intXXX x) {                        "
        "    intXXX ret = 0;                           "
        "    if (x > 0) { ret = x; }                   "
        "    return ret; }                             ";


}  // gashres
