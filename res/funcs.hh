/*
 * funcs.hh -- Store pre-defined functions
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

/**
 * The registry contains all the pre-defined functions
 *
 */
    typedef map<string, string> NameStringMap;
    typedef map<string, u32> NameNumMap;

    class FuncRegistry {

        NameStringMap m_func_map;
        NameNumMap    m_nparam_map;

        FuncRegistry() {}
    public:


        static FuncRegistry& instance() {
            static FuncRegistry ins;
            return ins;
        }

        /**
         * Register the function.
         *
         * @param name
         * @param nparam
         * @param fsrc
         *
         * @return
         */
        int registar(string name, int nparam, const string fsrc);

        /**
         * Return true if the function is in the registry.
         *
         * @param name
         *
         * @return
         */
        bool has_func(string name);

        /**
         * Load a function and instantiate the bit size.
         *
         * @param name
         * @param bitsize
         * @param fdst
         *
         * @return
         */
        int load(string name, u32 bitsize, string& fdst);

        /**
         * Generate input directives
         *
         * @param bitsize
         * @param varname
         * @param valstr
         * @param output
         *
         * @return
         */
        int load_data(string varname, string valstr, string& output);

        FuncRegistry(FuncRegistry const&)     = delete;
        void operator=(FuncRegistry const&)   = delete;
    };

    class FuncEntry {
    public:
        FuncEntry(string name, u32 nparam, string fsrc) {
            FuncRegistry::instance().registar(name, nparam, fsrc);
        }
    };

#define REGISTER_FUNC(fname, nparam, fsrc)                    \
    FuncEntry func##fname(#fname, nparam, fsrc)

#define LOAD_CIRCUIT(fname, bitsize, circ)                    \
    gashres::FuncRegistry::instance().load(fname, bitsize, circ)

#define LOAD_DATA(varname, val, output)                         \
    gashres::FuncRegistry::instance().load_data(varname, val, output)
}  // gashres
