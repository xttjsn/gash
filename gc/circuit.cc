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

namespace gashgc {

  /**
   * Circuit
   *
   */

  Gate* Circuit::get_gate(u32 i) {
    return m_gate_map[i];
  }

  WireInstance* Circuit::get_wireins(u32 i) {
    return m_wireins_map[i];
  }

  int Circuit::add_wireins(WireInstance* w) {
    if (m_wireins_map.find(w->get_id()) != m_wireins_map.end()) {
      return -G_EEXIST;
    }
    m_wireins_map.emplace(w->get_id(), w);
    return 0;
  }

  int Circuit::add_gate(Gate* g) {
    if (has_gate(g->get_id())) {
      return -G_EEXIST;
    }
    m_gate_map.emplace(g->get_id(), g);
    return 0;
  }

  bool Circuit::has_gate(u32 id) {
    return m_gate_map.find(id) != m_gate_map.end();
  }

  void build_circ(string circ_file_path, Circuit& circ) {

    ifstream file(circ_file_path);

    if (!file.is_open()) {
      FATAL("Unable to open circuit file " << circ_file_path);
    }

    string line;
    int func;
    u32 linum;
    u32 numVar = 0, numIn = 0, numOut = 0, numAND = 0, numOR = 0, numXOR = 0, numDFF = 0;
    u32 idx;
    u32 id;
    vector<string> items;
    vector<string> subitems;
    const char* delim = " ";
    const char* subdelim = ":";
    WI *w;

    while (getline(file, line)) {

      linum++;
      split(line, delim, items);

      // Skip directives
      if (items[0].find('#') != std::string::nops)
        continue;

      switch (items.size()) {
      case 8:
        {
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
      case 2:
        {

          // I:13:1

          // I/O wires
          split(items[0], subdelim, subitems);

          // Input wires
          if (strcmp("I", subitems[0].c_str()) == 0) {

            idx = stoi(items[1]);
            id  = id / 2;

            w = new WI(id, !is_odd(idx));
            REQUIRE_GOOD_STATUS(circ.add_wireins(w));

            if (subitems.size() > 2) {
              WARNING("Invalid input wire, expecting 2 item, getting " << subitems.item() << " items");
            }

          } else {

            // Output wires

            idx = stoi(items[1]);
            id  = id / 2;
            w = new WI(id, !is_odd(idx));

            if (subitems.size() == 3) {
              val = stoi(items[2]);
              w->set_val(val);

            }

          }

        }

      }

    }

  }

}
