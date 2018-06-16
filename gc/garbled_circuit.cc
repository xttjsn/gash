/*
 * garbled_circuit.cc -- Implementation of garbled circuit
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

#include "garbled_circuit.hh"

#include "util.hh"

namespace gashgc {

    GC::GC() {}

    GG* GC::get_gg(u32 id)
    {
        auto it = m_gg_map.find(id);
        if (it != m_gg_map.end()) {
            return it->second;
        }
        return NULL;
    }

    int GC::add_gg(GG* gate)
    {
        if (has_gg(gate->get_id())) {
            return -G_EEXIST;
        }
        m_gg_map.emplace(gate->get_id(), gate);
        return 0;
    }

    bool GC::has_gg(u32 id) { return m_gg_map.find(id) != m_gg_map.end(); }

    int GC::add_gg(Circuit& circ, u32 id, block row1, block row2, block row3)
    {

        if (has_gg(id)) {
            return -G_EEXIST;
        }

        Gate* g = circ.get_gate(id);

        u32 id_0 = g->m_in0->get_id();

        u32 id_1 = g->m_in1->get_id();

        u32 id_out = g->m_out->get_id();

        GWI* in0;

        GWI* in1;

        GWI* out;

        if (!has_gwi(id_0)) {

            in0 = new GWI(g->m_in0);
            REQUIRE_GOOD_STATUS(this->add_gwi(in0));

        } else {

            in0 = m_gwi_map.find(id_0)->second;
        }

        if (!has_gwi(id_1)) {

            in1 = new GWI(g->m_in1);
            REQUIRE_GOOD_STATUS(this->add_gwi(in1));

        } else {

            in1 = m_gwi_map.find(id_1)->second;
        }

        if (!has_gwi(id_out)) {

            out = new GWI(g->m_out);
            REQUIRE_GOOD_STATUS(this->add_gwi(out));

        } else {

            out = m_gwi_map.find(id_out)->second;
        }

        GG* gg = new GG(false, in0, in1, out);
        EGTT* table = new EGTT(row1, row2, row3);
        gg->m_egtt = table;
        this->add_gg(gg);

        return 0;
    }

    GWI* GC::get_gwi(u32 i)
    {
        auto it = m_gwi_map.find(i);
        if (it != m_gwi_map.end()) {
            return it->second;
        } else {
            return NULL;
        }
    }

    int GC::add_gwi(GWI* w)
    {
        if (has_gwi(w->get_id())) {
            return -G_EEXIST;
        }
        m_gwi_map.emplace(w->get_id(), w);
        return 0;
    }

    bool GC::has_gwi(u32 id)
    {
        return m_gwi_map.find(id) != m_gwi_map.end();
    }

    int GC::set_gwl(u32 id, block label)
    {
        if (!has_gwi(id)) {
            return -G_ENOENT;
        }
        m_gwi_map.find(id)->second->set_lbl(label);
        return 0;
    }

    int GC::get_lbl(u32 id, int val, block& lbl)
    {
        GASSERT(val == 0 || val == 1);
        auto it = m_gwi_map.find(id);
        if (it != m_gwi_map.end()) {
            lbl = it->second->get_lbl_w_smtc(val);
            return 0;
        }
        return -G_ENOENT;
    }

    int GC::get_orig_lbl(u32 id, int val, block& lbl)
    {
        GASSERT(val == 0 || val == 1);
        auto it = m_gwi_map.find(id);
        if (it != m_gwi_map.end()) {
            lbl = it->second->get_orig_lbl_w_smtc(val);
            return 0;
        }
        return -G_ENOENT;
    }

    void GC::init()
    {
        srand_sse(time(NULL));
        m_R = random_block();
        set_lsb(m_R);
        cout << "R:" << block2hex(m_R) << endl;
    }

    void GC::debug_report_garbler()
    {

        GWI *in0, *in1, *out;
        GG* g;

        std::cout << "R: " << block2dec(this->m_R) << endl;

        for (auto it = m_gg_map.begin(); it != m_gg_map.end(); ++it) {

            g = it->second;

            if (!g) {
                FATAL("GG with output wire id" << g->m_out->get_id() << "is null");
            }

            in0 = g->m_in0;
            in1 = g->m_in1;
            out = g->m_out;

            cout << "Gate " << g->get_id() << ":" << endl;

            cout << "Is XOR= " << g->m_is_xor << endl;

            cout << TABx1 << "in0=" << in0->get_id() << ":" << endl
                 << endl;

            cout << TABx2 << "label0=" << block2dec(in0->get_lbl0()) << endl;
            cout << TABx2 << "label1=" << block2dec(in0->get_lbl1()) << endl;

            cout << TABx1 << "in1=" << in1->get_id() << ":" << endl;

            cout << TABx2 << "label0=" << block2dec(in1->get_lbl0()) << endl;
            cout << TABx2 << "label1=" << block2dec(in1->get_lbl1()) << endl;

            cout << TABx1 << "out=" << out->get_id() << ":" << endl;

            cout << TABx2 << "label0=" << block2dec(out->get_lbl0()) << endl;
            cout << TABx2 << "label1=" << block2dec(out->get_lbl1()) << endl;

            cout << TABx1 << "table:" << endl;

            cout << TABx2 << "row1=" << block2dec(g->m_egtt->get_row(1)) << endl;
            cout << TABx2 << "row2=" << block2dec(g->m_egtt->get_row(2)) << endl;
            cout << TABx2 << "row3=" << block2dec(g->m_egtt->get_row(3)) << endl;
        }
    }

    void GC::debug_report_evaluator()
    {

        GG* g;

        for (auto it = m_gg_map.begin(); it != this->m_gg_map.end(); ++it) {

            g = it->second;

            cout << "Gate " << g->get_id() << ":" << endl;

            cout << TABx1 << "isXOR= " << g->m_is_xor << endl;

            if (!g->m_is_xor) {

                cout << TABx2 << "table: " << endl;
                cout << TABx2 << "row1=" << block2dec(g->m_egtt->get_row(1)) << endl;
                cout << TABx2 << "row2=" << block2dec(g->m_egtt->get_row(2)) << endl;
                cout << TABx2 << "row3=" << block2dec(g->m_egtt->get_row(3)) << endl;
            }
        }
    }

    GC::~GC()
    {

        for (auto it = m_gwi_map.begin(); it != m_gwi_map.end(); ++it) {
            delete it->second;
        }

        // for (auto it = m_gg_map.begin(); it != this->m_gg_map.end(); ++it) {
        //     delete it->second;
        // }
    }

    void GWI::garble(block R)
    {
        block lbl0 = random_block();
        block lbl1 = xor_block(lbl0, R);
        set_lbl0(lbl0);
        set_lbl1(lbl1);
    }

    int GWI::get_smtc_w_lsb(int lsb)
    {

        if (lsb != 0 && lsb != 1) {
            WARNING("Invalid lsb, can only be 0 or 1");
            return -G_EINVAL;
        }

        int lbl0lsb = get_lsb(get_lbl0());
        int lbl1lsb = get_lsb(get_lbl1());

        if (lbl0lsb == lsb) {
            return m_inv ? 1 : 0;
            // return 0;
        } else if (lbl1lsb == lsb) {
            return m_inv ? 0 : 1;
            // return 1;
        } else {
            return -G_EINVAL;
        }
    }

    block GWI::get_lbl_w_smtc(int smtc)
    {
        if (smtc == 0) {
            return get_lbl0();
        } else {
            return get_lbl1();
        }
    }

    block GWI::get_orig_lbl_w_smtc(int smtc)
    {
        if (smtc == 0) {
            return get_orig_lbl0();
        } else {
            return get_orig_lbl1();
        }
    }

    block GWI::get_lbl_w_lsb(int lsb)
    {

        if (lsb != 0 && lsb != 1) {
            FATAL("Invalid lsb, can only be 0 or 1");
        }

        int lbl0lsb = get_lsb(get_lbl0());
        int lbl1lsb = get_lsb(get_lbl1());

        if (lbl0lsb == lsb) {
            return get_lbl0();
        } else if (lbl1lsb == lsb) {
            return get_lbl1();
        } else {
            FATAL("Cannot find label with lsb " << lsb << ", garbling should make sure\
      that lsb are different");
        }
    }

    int GWI::set_lbl_w_smtc(block lbl, int smtc)
    {

        if (smtc != 0 && smtc != 1) {
            WARNING("Invalid semantic " << smtc);
            return -G_EINVAL;
        }

        if (smtc == 0) {
            // m_gw->m_lbl0 = lbl;
            set_lbl0(lbl);
        } else {
            // m_gw->m_lbl1 = lbl;
            set_lbl1(lbl);
        }

        return 0;
    }

    int GWI::recover_smtc()
    {

        if (block_eq(m_gw->m_lbl, get_lbl0())) {

            // return 0 ^ get_inv();
            return 0;

        } else if (block_eq(m_gw->m_lbl, get_lbl1())) {

            // return 1 ^ get_inv();
            return 1;

        } else {

            WARNING("Invalid label, neither label0 nor label 1");

#ifdef GASH_DEBUG

            cout << "Wire id:" << get_id() << endl;
            cout << "Inverted: " << get_inv() ? "true" : "false" << endl;
            cout << "m_lbl:" << block2hex(m_gw->m_lbl) << endl;
            cout << "m_lbl0:" << block2hex(m_gw->m_lbl0) << endl;
            cout << endl;

#endif
            return -G_ENOENT;
        }

        return 0;
    }

} // namespace gashgc
