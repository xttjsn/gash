/*
 * circuit.cc -- Circuit implementation
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

#include "circuit.hh"
#include "util.hh"

namespace gashgc {

    /**
   * Circuit
   *
   */

    Gate* Circuit::get_gate(u32 i)
    {
        if (m_gate_map.find(i) == m_gate_map.end()) {
            return NULL;
        }
        return m_gate_map[i];
    }

    WireInstance* Circuit::get_wireins(u32 i)
    {
        if (m_wi_map.find(i) == m_wi_map.end()) {
            return NULL;
        }
        return m_wi_map[i];
    }

    int Circuit::add_wireins(WireInstance* w)
    {
        if (m_wi_map.find(w->get_id()) != m_wi_map.end()) {
            return -G_EEXIST;
        }
        m_wi_map.emplace(w->get_id(), w);
        return 0;
    }

    int Circuit::add_gate(Gate* g)
    {
        if (has_gate(g->get_id())) {
            return -G_EEXIST;
        }
        m_gate_map.emplace(g->get_id(), g);
        return 0;
    }

    bool Circuit::has_gate(u32 id)
    {
        return m_gate_map.find(id) != m_gate_map.end();
    }

    int Circuit::create_gate(u32 outid, int func, u32 in0id, u32 in1id, bool in0inv, bool in1inv)
    {

        if (has_gate(outid)) {
            WARNING("Gate is already in the circuit");
            return -G_EEXIST;
        }

        // WI* out = new WI(outid, false);
        WI* out = get_wireins(outid);
        WI* in0 = get_wireins(in0id);
        WI* in1 = get_wireins(in1id);

        if (out == NULL) {
            out = new WI(outid, false);
            REQUIRE_GOOD_STATUS(add_wireins(out));
        }

        if (!in0) {
            WARNING("Wire instance in0 does not exist. ID:" << in0id);
            return -G_ENOENT;
        }
        if (!in1) {
            WARNING("Wire instance in1 does not exist. ID:" << in1id);
            return -G_ENOENT;
        }

        if (in0->get_inv() != in0inv) {
            in0->set_inv(in0inv);
        }

        if (in1->get_inv() != in1inv) {
            in1->set_inv(in1inv);
        }

        Gate* g = new Gate(func, in0, in1, out);
        REQUIRE_GOOD_STATUS(add_gate(g));

        return 0;
    }

    int build_circuit(string circ_file_path, Circuit& circ)
    {

        ifstream file(circ_file_path);

        if (!file.is_open()) {
            FATAL("Unable to open circuit file " << circ_file_path);
        }

        string line;
        int func;
        u32 linum;
        u32 numVar = 0, numIn = 0, numOut = 0, numAND = 0,
            numOR = 0, numXOR = 0, numDFF = 0;
        u32 idx;
        u32 id;
        u32 in0id, in1id, outid;
        u32 in0idx, in1idx, outidx;
        int val;
        vector<string> items;
        vector<string> subitems;
        map<u32, bool> id_inv_map;
        const char* delim = " ";
        const char* subdelim = ":";
        WI* w;
        Gate* g;

        while (getline(file, line)) {

            linum++;
            items.clear();
            subitems.clear();
            split(line, delim, items);

            if (items.size() == 0)
                continue;

            // Skip directives
            if (items[0].find('#') != std::string::npos)
                continue;

            switch (items.size()) {
            case 8: {
                // Prologue line

                numVar = stoi(items[1]);
                numIn = stoi(items[2]);
                numOut = stoi(items[3]);
                numAND = stoi(items[4]);
                numOR = stoi(items[5]);
                numXOR = stoi(items[6]);
                numDFF = stoi(items[7]);

                circ.m_nin = numIn;
                circ.m_nout = numOut;
                break;
            }
            case 1: {
                // Input example:    I:14
                // Output example:   O:309,  O:309:1

                // I/O wires
                split(items[0], subdelim, subitems);

                // Input wires
                if (strcmp("I", subitems[0].c_str()) == 0) {

                    idx = stoi(subitems[1]);
                    id = idx / 2;

                    w = new WI(id, is_odd(idx));
                    REQUIRE_GOOD_STATUS(circ.add_wireins(w));

                    if (subitems.size() > 2) {
                        WARNING("Invalid input wire, expecting 2 item, getting " << subitems.size() << " items");
                    }

                    circ.m_in_id_set.emplace(id);

                } else if (strcmp("O", subitems[0].c_str()) == 0) {

                    // Output wires

                    idx = stoi(subitems[1]);
                    id = idx / 2;
                    w = new WI(id, is_odd(idx));

                    if (subitems.size() == 3) {
                        val = stoi(subitems[2]);
                        w->set_val(val);
                    }

                    circ.m_out_id_set.emplace(id);
                }

                circ.m_wi_map.emplace(id, w);

                break;
            }
            case 4: {
                // Gate:       <out> <func> <in0> <in1>, e.g. 67 14 69 13

                outidx = stoi(items[0]);
                outid = outidx / 2;
                func = stoi(items[1]);
                in0idx = stoi(items[2]);
                in0id = in0idx / 2;
                in1idx = stoi(items[3]);
                in1id = in1idx / 2;

                if (func > 16) {
                    WARNING("Line " << linum << "-Invalid gate function " << func);
                    return -G_EINVAL;
                }

                if (is_odd(outidx)) {
                    WARNING("Line " << linum << "-Invalid output wire, output wire cannot have odd index");
                    return -G_EINVAL;
                }

                // Check whether a wire is used both as inverted and uninverted
                if (id_inv_map.find(in0id) != id_inv_map.end()) {
                    if (id_inv_map.find(in0id)->second != is_odd(in0idx)) {
                        WARNING("Line " << linum << "-Cannot use a wire in both inverted and uninverted state.");
                        return -G_EINVAL;
                    }
                } else {
                    id_inv_map.emplace(in0id, is_odd(in0idx));
                }

                if (id_inv_map.find(in1id) != id_inv_map.end()) {
                    if (id_inv_map.find(in1id)->second != is_odd(in1idx)) {
                        WARNING("Line " << linum << "-Cannot use a wire in both inverted and uninverted state.");
                        return -G_EINVAL;
                    }
                } else {
                    id_inv_map.emplace(in1id, is_odd(in1idx));
                }

                REQUIRE_GOOD_STATUS(circ.create_gate(outid, func, in0id, in1id, is_odd(in0idx), is_odd(in1idx)));
                break;
            }
            default:

              perror("Items have invalid length!\n");
              perror("Items:\n");
              for (int i = 0; i < items.size(); i++) {
                cerr << items[i] << endl;
              }
              cerr << "line:" << line << endl;
              abort();
            }
        }

        circ.m_ngate = circ.m_gate_map.size();

        return 0;
    }

} // namespace gashgc
