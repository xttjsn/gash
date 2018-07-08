//
//  func.hpp
//  gash
//
//  Created by Xiaoting Tang on 6/17/18.
//  Copyright © 2018 Xiaoting Tang. All rights reserved.
//

#ifndef func_hpp
#define func_hpp

#include <stdio.h>
#include <iostream>
#include <stdexcept>
#include <map>
#include <string>
#include "util.hpp"
using std::map;
using std::string;
using std::to_string;

#define FUNC_BILLIONAIRE       0x1
#define FUNC_ADD               0x2
#define FUNC_SUB               0x3
#define FUNC_DIV               0x4
#define FUNC_MUL               0x5
#define FUNC_RELU              0x6
#define FUNC_SS_DIV            0x7
#define FUNC_SS_RELU           0x8

typedef map<string, string> NameStringMap;
typedef map<string, int> NameNumMap;

class FuncRegistry {
    
    NameStringMap m_func_map;
    NameNumMap    m_nparam_map;
    
    FuncRegistry() {}
    public:
    
    static FuncRegistry& instance() {
        static FuncRegistry ins;
        return ins;
    }
    
    int registar(string name, int nparam, const string fsrc) {
        if (m_func_map.find(name) != m_func_map.end()) {
            abort();
        }
        m_func_map.emplace(name, fsrc);
        m_nparam_map.emplace(name, nparam);
        return 0;
    }

    bool has_func(string name) {
        return m_func_map.find(name) != m_func_map.end();
    }
    
    int load(string name, int bitsize, string& fdst) {
        if (m_func_map.find(name) == m_func_map.end()) {
            
            // Special handle exp circuit
            if (name == "ss_exp") {
                char buf[128];
                string cmd = "python ss_exp_gen.py " + to_string(bitsize);
                FILE* pipe = popen(cmd.c_str(), "r");
                if (!pipe) throw std::runtime_error("popen() failed!");
                try {
                    while (!feof(pipe)) {
                        if (fgets(buf, 128, pipe) != NULL)
                            fdst += buf;
                    }
                } catch (...) {
                    pclose(pipe);
                    throw;
                }
                pclose(pipe);
            }
            abort();
        }
        
        fdst = m_func_map.find(name)->second;
        fdst = find_n_replace(fdst, "XXX", to_string(bitsize));
        return 0;
    }
    
    int load_data(string varname, string valstr, string& output) {
        output = output + " #definput " + varname + " " + valstr;
        return 0;
    }
    
    FuncRegistry(FuncRegistry const&)     = delete;
    void operator=(FuncRegistry const&)   = delete;
};

class FuncEntry {
    public:
    FuncEntry(string name, int nparam, string fsrc) {
        FuncRegistry::instance().registar(name, nparam, fsrc);
    }
};

#define REGISTER_FUNC(fname, nparam, fsrc)                    \
    FuncEntry func##fname(#fname, nparam, fsrc)

#define LOAD_CIRCUIT(fname, bitsize, circ)                    \
    FuncRegistry::instance().load(fname, bitsize, circ)

#define LOAD_DATA(varname, val, output)                         \
    FuncRegistry::instance().load_data(varname, val, output)





#endif /* func_hpp */
