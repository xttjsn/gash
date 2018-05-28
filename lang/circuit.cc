/*
 * circuit.cc -- Implementation of circuit related classes
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

namespace gashlang {

    static u32 wid = 0;

    /**
 * Several helper functions for Wire
 *
 */

    inline bool is_odd(int n)
    {
        return !(n % 2 == 0);
    }

    inline int evenify(int n)
    {
        return is_odd(n) ? n - 1 : n;
    }

    /**
 * Wire related functions
 */
    void Wire::invert()
    {
        if (is_odd(this->m_id)) {
            this->m_id--;
        } else {
            this->m_id++;
        }
    }

    void Wire::set_id_even()
    {
        if (is_odd(this->m_id)) {
            this->m_id--;
        }
    }

    void Wire::set_id_odd()
    {
        if (!is_odd(this->m_id)) {
            this->m_id++;
        }
    }

    void Wire::invert_from(Wire* w)
    {
        if (is_odd(w->m_id)) {
            this->set_id_even();
        } else {
            this->set_id_odd();
        }
    }

    void wput(Wire* w)
    {
        if (--w->m_refcount == 0) {
            delete w;
        }
    }

    void wref(Wire* w)
    {
        ++w->m_refcount;
    }

    Wire* onewire()
    {
        return new Wire(++(++wid), 1);
    }

    Wire* zerowire()
    {
        return new Wire(++(++wid), 0);
    }

    Wire* nextwire()
    {
        return new Wire(++(++wid));
    }

    /**
 * Bunble related functions
 */
    Bundle::Bundle(vector<Wire*>& wires) : m_isconst(false)
    {
        m_wires = wires;
    }

    Bundle::Bundle(Wire* w) : m_isconst(false)
    {
        m_wires.push_back(w);
    }

    Bundle::Bundle(u32 len) : m_isconst(false)
    {
        for (u32 i = 0; i < len; i++) {
            Wire* w = nextwire();
            m_wires.push_back(w);
        }
    }

    void Bundle::add(Wire* w)
    {
        m_wires.push_back(w);
        m_wires_map.insert(make_pair(w->m_id, w));
        wref(w);
    }

    void Bundle::add(Wire* w, u32 i)
    {
        GASSERT(i <= size());
        // Cannot add duplicate wire
        GASSERT(m_wires_map.find(w->m_id) == m_wires_map.end());
        m_wires.insert(m_wires.begin() + i, w);
        m_wires_map.insert(make_pair(w->m_id, w));
        wref(w);
    }

    bool Bundle::hasWire(u32 i)
    {
        return m_wires_map.find(i) != m_wires_map.end();
    }

    Wire* Bundle::getWire(u32 i)
    {
        if (m_wires_map.find(i) == m_wires_map.end()) {
            return NULL;
        }
        return m_wires_map.find(i)->second;
    }

    Wire* Bundle::back()
    {
        return m_wires.back();
    }

    Wire* Bundle::operator[](u32 i)
    {
        return m_wires[i];
    }

    int Bundle::remove(u32 i)
    {
        if (i >= size()) {
            return -EINVAL;
        }
        Wire* w = m_wires[i];
        m_wires.erase(m_wires.begin() + i);
        m_wires_map.erase(w->m_id);
        wput(w);
        return 0;
    }

    u32 Bundle::size()
    {
        return m_wires.size();
    }

    int Bundle::copyfrom(Bundle& src, u32 start, u32 srcstart, u32 size)
    {
        if (size > this->size() - start) {
            std::cerr << "This bundle has not enough room to copy from another bundle."
                      << std::endl;
            return 1;
        }
        if (src.size() - srcstart < size) {
            std::cerr << "The source bundle has not enough wires." << std::endl;
            return 1;
        }
        for (u32 i = 0; i < size; ++i) {
            this->m_wires[i + start] = src[i + srcstart];
        }
        return 0;
    }

    void Bundle::clear()
    {
        while (this->m_wires.size() > 0) {
            Wire* w = this->m_wires.back();
            this->m_wires.pop_back();
            this->m_wires_map.erase(w->m_id);
            wput(w);
        }
        m_isconst = false;
    }

    void Bundle::clear_val()
    {
        for (auto it = this->m_wires.begin(); it != this->m_wires.end(); ++it) {
            (*it)->m_v = -1;
        }
    }

    void Bundle::emit(ostream& outstream)
    {
        for (auto it = this->m_wires.begin(); it != this->m_wires.end(); ++it) {
            Wire* w = *it;
            if (w->m_v >= 0) {
                outstream << evenify(w->m_id) << '(' << w->m_v << ')' << endl;
            } else {
                outstream << evenify(w->m_id) << endl;
            }
        }
    }

    void num2bundle_n(i64 v, Bundle& bret, u32 n)
    {
        u32 i = 0;
        bret.clear();
        while (i < 64 && i < n) {
            bret.add(getbit(v, i) == 1 ? onewire() : zerowire());
            i++;
        }
        bret.m_isconst = true;
    }

    void num2bundle(i64 v, Bundle& bret)
    {
        num2bundle_n(v, bret, 64);
    }

    void Prologue::emit(ostream& outstream)
    {
        outstream << "circ" << ' ' << numVAR << ' ' << numIN << ' ' << numOUT << ' '
                  << numAND << ' ' << numOR << ' ' << numXOR << ' ' << numDFF << endl;
    }

    void Gate::emit(ostream& outstream)
    {
        if (m_in0->m_v >= 0) { // A constant wire
            if (m_in1->m_v >= 0) {
                outstream << evenify(m_out->m_id) << ' ' << m_op << ' ' << m_in0->m_id
                          << '(' << m_in0->m_v << ')' << ' ' << m_in1->m_id << '('
                          << m_in1->m_v << ')' << std::endl;
            } else {
                outstream << evenify(m_out->m_id) << ' ' << m_op << ' ' << m_in0->m_id
                          << '(' << m_in0->m_v << ')' << ' ' << m_in1->m_id << std::endl;
            }
        } else {
            if (m_in1->m_v >= 0) {
                outstream << evenify(m_out->m_id) << ' ' << m_op << ' ' << m_in0->m_id
                          << ' ' << m_in1->m_id << '(' << m_in1->m_v << ')' << std::endl;
            } else {
                outstream << evenify(m_out->m_id) << ' ' << m_op << ' ' << m_in0->m_id
                          << ' ' << m_in1->m_id << std::endl;
            }
        }
    }

    void GateList::emit(ostream& outstream)
    {
        for (auto it = m_gates.begin(); it != m_gates.end(); it++) {
            (*it)->emit(outstream);
        }
    }

    /**
 * Circuit implementation
 */

    Circuit::Circuit(ostream& circ_stream, ostream& data_stream)
    {
        m_circ_stream = &circ_stream;
        m_data_stream = &data_stream;
    }

    void Circuit::write_inwires() {
        Wire*       w;
        u32         id;
        ostream&     stream = *m_circ_stream;

        for (int i = 0; i < m_in.size(); ++i) {
            w = m_in[i];
            id = w->m_id;
            stream << "I:" << evenify(id) << endl;
        }
    }

    void Circuit::write_outwires() {
        Wire*       w;
        u32         id;
        ostream&     stream = *m_circ_stream;

        for (int i = 0; i < m_out.size(); ++i) {
            w = m_out[i];
            id = w->m_id;
            if (w->m_v == 0 || w->m_v == 1)
              stream << "O:" << evenify(id) << ':' << w->m_v << endl;
            else
              stream << "O:" << evenify(id) << endl;
        }
    }

    void Circuit::write()
    {
        m_prologue.emit(*m_circ_stream);
        write_inwires();
        write_outwires();
        m_gates.emit(*m_circ_stream);
        write_input();
    }

    void Circuit::write_input()
    {

        Wire*        w;
        int          val;
        u32          id;
        ostream&     stream = *m_data_stream;

        for (int i = 0; i < m_in.size(); ++i) {
          w = m_in[i];
          id = w->m_id;
          val = w->m_v;
          if (val == 0 || val == 1)
            stream << evenify(id) << ' ' << val << endl;
        }

    }

    void Circuit::add_input_wire(Wire* w)
    {
        m_in.add(w);
        m_wires.emplace(w->m_id, w);
        m_prologue.numIN++;
    }

    bool Circuit::has_input_dup(Wire* w)
    {
        return m_input_dup.find(w->m_id) != m_input_dup.end();
    }

    bool Circuit::is_input_wire(Wire* w)
    {
        Wire* w_another;
        for (u32 i = 0; i < m_in.size(); ++i) {
            w_another = m_in[i];
            // Do some consistency check along the way
            if (w->m_id == w_another->m_id) {
                /* WARNING( */
                /* "Two wire pointers point to the same wire instance. ID:" << w->m_id); */
                if (w->m_v != w->m_v) {
                    FATAL(
                        "Two wire pointers point to the same wire instance but have "
                        "different value. ID:"
                        << w->m_id);
                }
                return true;
            }
        }
        return false;
    }

    void Circuit::set_input_inv_dup(Wire* w, Wire* w_dup)
    {
        u32 id_original = w->m_id;
        u32 id_duplicate = w_dup->m_id;
        if (m_input_dup.find(id_original) != m_input_dup.end()) {
            FATAL("Input wire with id " << id_original << " is already_duplicated.");
        }
        m_input_dup.emplace(id_original, id_duplicate);
        m_input_dup.emplace(id_duplicate, id_original);
        m_wires.insert(pair<u32, Wire*>(id_duplicate, w_dup));
        set_invert_wire(w, w_dup);
        add_input_wire(w_dup);
    }

    bool Circuit::has_invert_wire(Wire* w)
    {
        return m_wire_inverts.find(w->m_id) != m_wire_inverts.end();
    }

    Wire* Circuit::get_invert_wire(Wire* w)
    {
        Wire* w_inv;
        if (has_invert_wire(w))
            w_inv = m_wires.find(m_wire_inverts.find(w->m_id)->second)->second;
        return w_inv;
    }

    // TODO: change FATAL to warning, add checks in caller instead
    int Circuit::set_invert_wire(Wire* w, Wire* w_inv)
    {
        if (has_invert_wire(w)) {
            FATAL("Wire " << w->m_id << "already has inverted wire.");
        }
        if (has_invert_wire(w_inv)) {
            FATAL("Wire " << w_inv->m_id << "already has inverted wire.");
        }
        m_wire_inverts.emplace(w->m_id, w_inv->m_id);
        m_wire_inverts.emplace(w_inv->m_id, w->m_id);

        return 0;
    }

    void Circuit::add_gate(int op, Wire* in0, Wire* in1, Wire* out)
    {
        Gate* g = new Gate(op, in0, in1, out);
        m_gates.add(g);
        m_wires.emplace(out->m_id, out);
        switch (op) {
        case opAND:
            m_prologue.numAND++;
            break;
        case opOR:
            m_prologue.numOR++;
            break;
        case opXOR:
            m_prologue.numXOR++;
            break;
        case opDFF:
            m_prologue.numDFF++;
            break;
        }
    }

    void print_idw_map(map<u32, Wire*> wires_map) {
        for (auto it = wires_map.begin(); it != wires_map.end(); ++it) {
            cout << it->first << ':' << it->second << endl;
        }
    }

} // namespace gashlang
