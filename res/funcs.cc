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
#include "funcs.hh"
#include <boost/algorithm/string/replace.hpp>

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

    const string fsrc_ss_relu =
        "func ssrelu (intXXX in0, intXXX in1, intXXX r) {         "
        "    intXXX sum = in0 + in1;                              "
        "    intXXX ret = 0;                                      "
        "    if (sum > 0) { ret = sum - r; }                      "
        "    else { ret = 0 - r; }                                "
        "    return ret; }                                        ";

    const string fsrc_ss_relugrad =
        "func ss_relugrad (intXXX in0, intXXX in1, intXXX r) {    "
        "     intXXX sum = in0 + in1;                             "
        "     intXXX ret = 0;                                     "
        "     if (sum > 0) { ret = sum - r; }                       "
        "     else { ret = 0 - r; }                               "
        "     return ret; }                                       ";


    string find_n_replace(string s, string pattern, string subst)
    {
        boost::replace_all(s, pattern, subst);
        return s;
    }

    int FuncRegistry::registar(string name, int nparam, const string fsrc) {
        if (m_func_map.find(name) != m_func_map.end()) {
            return -G_EEXIST;
        }

        m_func_map.emplace(name, fsrc);
        m_nparam_map.emplace(name, nparam);

        return 0;
    }

    bool FuncRegistry::has_func(string name) {
        return m_func_map.find(name) != m_func_map.end();
    }

    int FuncRegistry::load(string name, u32 bitsize, string& fdst) {
        if (m_func_map.find(name) == m_func_map.end()) {
            return -G_ENOENT;
        }

        fdst = m_func_map.find(name)->second;
        fdst = find_n_replace(fdst, "XXX", to_string(bitsize));
        return 0;
    }

    int FuncRegistry::load_data(string varname, string valstr, string& output)
    {
        output = output + " #definput " + varname + " " + valstr;
        return 0;
    }


    REGISTER_FUNC(billionaire, 2, fsrc_billionaire);
    REGISTER_FUNC(add, 2, fsrc_add);
    REGISTER_FUNC(sub, 2, fsrc_sub);
    REGISTER_FUNC(relu, 1, fsrc_relu);
    REGISTER_FUNC(ss_relu, 3, fsrc_ss_relu);
    REGISTER_FUNC(ss_relugrad, 3, fsrc_ss_relugrad);

}  // gashres
