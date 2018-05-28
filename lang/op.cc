/*
 * op.cc -- Arithmetic, Binary and comparison operations
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
#include "op.hh"
#include <algorithm>

namespace gashlang {

    using std::min;
    extern Circuit mgc;

    int evala_ADD_raw(Bundle& in0, Bundle& in1, Bundle& out, Wire*& cin)
    {
        GASSERT(in0.size() == in1.size());
        cin = zerowire();
        Wire* a;
        Wire* b;
        Wire* c;
        for (u32 i = 0; i < in0.size(); i++) {
            a = in0[i];
            b = in1[i];
            int fadd_status = evalw_FADD(a, b, cin, c);
            if (fadd_status < 0) {
                return fadd_status;
            }
            out.add(c);
        }
        return 0;
    }

    int evala_ADD(Bundle& in0, Bundle& in1, Bundle& out)
    {
        // Use raw ADD
        Wire* cin;
        return evala_ADD_raw(in0, in1, out, cin);
    }

    int evala_SUB(Bundle& in0, Bundle& in1, Bundle& out)
    {
        GASSERT(in0.size() == in1.size());
        // Negative in1
        Bundle in1_neg;

        // Take the negative of in1
        int neg_status = evala_UMINUS(in1, in1_neg);
        if (neg_status < 0)
            return neg_status;
        return evala_ADD(in0, in1_neg, out);
    }

    int evala_MUL_raw(Bundle& in0, Bundle& in1, Bundle& out)
    {
        GASSERT(in0.size() == in1.size());

        // Store the result from the previous level
        Bundle prev_res;

        // Store the current result
        Bundle res;

        // Temporary bundle
        Bundle tmp;
        Bundle tmp2;

        // Cin wire
        Wire* cin;

        u32 len = in0.size();
        for (u32 i = 0; i < len; i++) {
            // One level of multiplication
            for (u32 j = 0; j < len; j++) {
                Wire* w = nextwire();

                write_gate(opAND, in0[i], in1[j], w);

                res.add(w);
            }
            // If this is not the first level
            if (prev_res.size() > 0) {
                // tmp = res...000
                for (u32 k = 0; k < i; k++) {
                    tmp.add(zerowire());
                }
                for (u32 k = 0; k < res.size(); k++) {
                    tmp.add(res[k]);
                }

                // Add prev_res to tmp
                evala_ADD_raw(prev_res, tmp, tmp2, cin);
                prev_res = tmp2;

                // Insert cin to be the second-to-last bit, because the last bit is the
                // sign bit
                prev_res.add(cin, prev_res.size() - 1);

            } else {
                prev_res = res;
                prev_res.add(zerowire(), prev_res.size() - 1);
            }
            res = Bundle();
            tmp = Bundle();
            tmp2 = Bundle();
        }

        // Out is of size `2*len+1`
        out = prev_res;
        return 0;
    }

    int evala_MUL(Bundle& in0, Bundle& in1, Bundle& out)
    {
        // Firstly perform a "raw" multiplication
        evala_MUL_raw(in0, in1, out);
        // Remove the second-to-last bit, but preserve the sign bit
        int rm_status = out.remove(out.size() - 2);
        return rm_status;
    }

    int evala_DIV(Bundle& in0, Bundle& in1, Bundle& out)
    {
        GASSERT(in0.size() == in1.size());
        u32 len = in0.size();
        u32 biglen = len * 2 - 1;
        Wire* r;
        Bundle tmp0;
        Bundle tmp1;
        Bundle sub_res;
        for (u32 i = 0; i < len; i++) {
            if (i == 0) {
                // tmp0 = 000...in0
                for (u32 j = 0; j < len; j++) {
                    tmp0.add(in0[j]);
                }
                for (u32 j = 0; j < biglen - len; j++) {
                    tmp0.add(zerowire());
                }

                // tmp1 = 000...in1
                for (u32 j = 0; j < len; j++) {
                    tmp1.add(in1[j]);
                }
                for (u32 j = 0; j < biglen - len; j++) {
                    tmp1.add(zerowire());
                }

                int dvg_status = evala_DVG(tmp0, tmp1, sub_res, r);
                if (dvg_status < 0)
                    return dvg_status;

            } else {
                // tmp0 = 000..0 in1 000...0
                for (u32 j = 0; j < len - i - 1; j++) {
                    tmp0.add(zerowire());
                }
                for (u32 j = 0; j < len; j++) {
                    tmp0.add(in1[j]);
                }
                for (u32 j = 0; j < i; j++) {
                    tmp0.add(zerowire());
                }

                int dvg_status = evala_DVG(sub_res, tmp0, sub_res, r);
                if (dvg_status < 0)
                    return dvg_status;
            }
            out.add(r);
            tmp0 = Bundle();
            tmp1 = Bundle();
        }
        return 0;
    }

    int evala_DVG(Bundle& in0, Bundle& in1, Bundle& out, Wire*& ret)
    {
        GASSERT(in0.size() == in1.size());
        Bundle sub_res;

        int sub_status = evala_SUB(in0, in1, sub_res);
        if (sub_status < 0)
            return sub_status;

        int inv_status = evalw_INV(sub_res.back(), ret);
        if (inv_status < 0)
            return inv_status;

        out = sub_res;
        return 0;
    }

    int evala_UMINUS(Bundle& in, Bundle& out)
    {
        u32 len = in.size();
        Wire* w;
        Bundle tmp0;
        Bundle tmp1;

        // First, invert the bits
        for (u32 i = 0; i < len; ++i) {
            int inv_status = evalw_INV(in[i], w);
            if (inv_status < 0)
                return inv_status;
            tmp0.add(w);
        }

        // Construct tmp1 = 000...001
        tmp1.add(onewire());
        for (u32 i = 1; i < len; ++i) {
            tmp1.add(zerowire());
        }

        return evala_ADD(tmp0, tmp1, out);
    }

    int evalb_AND(Bundle& in0, Bundle& in1, Bundle& out)
    {
        GASSERT(in0.size() == in1.size());
        u32 len = in0.size();
        Wire* w;

        // Evaluate AND for each bit
        for (u32 i = 0; i < len; ++i) {
            int and_status = evalw_AND(in0[i], in1[i], w);
            if (and_status < 0)
                return and_status;
            out.add(w);
        }
        return 0;
    }

    int evalb_OR(Bundle& in0, Bundle& in1, Bundle& out)
    {
        GASSERT(in0.size() == in1.size());
        u32 len = in0.size();
        Wire* w;

        // Evaluate OR for each bit
        for (u32 i = 0; i < len; ++i) {
            int and_status = evalw_OR(in0[i], in1[i], w);
            if (and_status < 0)
                return and_status;
            out.add(w);
        }
        return 0;
    }

    int evalb_XOR(Bundle& in0, Bundle& in1, Bundle& out)
    {
        GASSERT(in0.size() == in1.size());
        u32 len = in0.size();
        Wire* w;

        // Evaluate XOR for each bit
        for (u32 i = 0; i < len; ++i) {
            int and_status = evalw_OR(in0[i], in1[i], w);
            if (and_status < 0)
                return and_status;
            out.add(w);
        }
        return 0;
    }
    int evalb_INV(Bundle& in, Bundle& out)
    {
        u32 len = in.size();
        Wire* w;

        // Invert each wire
        for (u32 i = 0; i < len; ++i) {
            int inv_status = evalw_INV(in[0], w);
            if (inv_status < 0)
                return inv_status;
            out.add(w);
        }
        return 0;
    }

    int evalb_SHL(Bundle& in, u32 n, Bundle& out)
    {
        // First, fill min(n, in.size()) zero wries to `out`
        for (u32 i = 0; i < min(n, in.size()); ++i) {
            out.add(zerowire());
        }

        // Then, copy in.size() - n wires to out
        for (u32 i = 0; i < in.size() - n; ++i) {
            out.add(in[i]);
        }

        return 0;
    }

    int evalb_SHR_psv(Bundle& in, u32 n, Bundle& out)
    {
        // SHR that preserves sign bit
        NOT_YET_IMPLEMENTED("evalb_SHR_psv");
        return 0;
    }

    int evalb_SHR(Bundle& in, u32 n, Bundle& out)
    {
        // SHR that doesn't preserve sign bit, all significant bits are zero

        // First, copy in.size() - n wires to `out`
        for (u32 i = 0; i < in.size() - n; ++i) {
            out.add(in[i]);
        }

        // Then, fill min(n, in.size()) zero wires to `out`
        for (u32 i = 0; i < min(n, in.size()); ++i) {
            out.add(zerowire());
        }

        return 0;
    }

    int evalc_LA(Bundle& in0, Bundle& in1, Bundle& out)
    {
        // (A > B) = A_n & B_n' |
        //           (A_n ^ B_n)' & ( A_{n-1} & B_{n-1}' ) |
        //           (A_n ^ B_n)' & (A_{n-1} ^ B_{n-1})' & A_{n-2} & B_{n-2}' |
        //           ...
        // Here & denotes AND, ^ denotes XOR, | denotes OR and ' denotes INV
        GASSERT(in0.size() == in1.size());
        u32 len = in0.size();

        // Build (A_i ^ B_i)'
        Bundle xors;
        for (u32 i = 0; i < len; ++i) {
            Wire* w = nextwire();
            Wire* w_inv;
            int xor_status = evalw_XOR(in0[i], in1[i], w);
            if (xor_status < 0)
                return xor_status;
            int inv_status = evalw_INV(w, w_inv);
            if (inv_status < 0)
                return inv_status;
            xors.add(w_inv);
        }

        Wire* ret;
        Wire* used_xor;
        Wire* tmp;
        Wire* inv_in1;
        Wire* and_in0_in1;
        Wire* and_in0_in1_xor;
        for (u32 i = 0; i < len; ++i) {
            if (i == 0) {
                used_xor = onewire();
                ret      = zerowire();
            }
            else {
                REQUIRE_GOOD_STATUS(evalw_AND(used_xor, xors[len - i], tmp));
                used_xor = tmp;
            }

            REQUIRE_GOOD_STATUS(evalw_INV(in1[len - i - 1], tmp));
            inv_in1 = tmp;

            REQUIRE_GOOD_STATUS(evalw_AND(in0[len - i - 1], inv_in1, tmp));
            and_in0_in1 = tmp;

            REQUIRE_GOOD_STATUS(evalw_AND(used_xor, and_in0_in1, tmp));
            and_in0_in1_xor = tmp;

            REQUIRE_GOOD_STATUS(evalw_OR(ret, and_in0_in1_xor, tmp));
            ret = tmp;
        }

        out.add(ret);
        return 0;
    }

    int evalc_LE(Bundle& in0, Bundle& in1, Bundle& out)
    {
        return evalc_LA(in1, in0, out);
    }

    int evalc_EQ(Bundle& in0, Bundle& in1, Bundle& out)
    {
        GASSERT(in0.size() == in1.size());
        u32 len = in0.size();
        Wire* ret = onewire();
        Wire* tmp;

        // ret = AND_i (A_i ^ B_i)'
        for (u32 i = 0; i < len; ++i) {
            Wire* w;
            Wire* w_inv;
            int xor_status = evalw_XOR(in0[i], in1[i], w);
            if (xor_status < 0)
                return xor_status;
            int inv_status = evalw_INV(w, w_inv);
            if (inv_status < 0)
                return inv_status;
            int and_status = evalw_AND(w_inv, ret, tmp);
            if (and_status < 0)
                return and_status;
            ret = tmp;
        }

        out.add(ret);
        return 0;
    }

    int evalc_LEE(Bundle& in0, Bundle& in1, Bundle& out)
    {
        Bundle ret_le;
        Bundle ret_eq;
        Wire* ret;
        int le_status = evalc_LE(in0, in1, ret_le);
        if (le_status < 0)
            return le_status;
        int eq_status = evalc_EQ(in0, in1, ret_eq);
        if (eq_status < 0)
            return eq_status;

        int or_status = evalw_OR(ret_le[0], ret_eq[0], ret);
        if (or_status < 0)
            return or_status;

        out.add(ret);
        return 0;
    }

    int evalc_LAE(Bundle& in0, Bundle& in1, Bundle& out)
    {
        return evalc_LEE(in1, in0, out);
    }

    int evalc_NEQ(Bundle& in0, Bundle& in1, Bundle& out)
    {
        Bundle ret_eq;
        Wire* ret;
        int eq_status = evalc_EQ(in0, in1, ret_eq);
        if (eq_status < 0)
            return eq_status;
        int inv_status = evalw_INV(ret_eq[0], ret);
        if (inv_status < 0)
            return inv_status;

        out.add(ret);
        return 0;
    }

    int evalo_if(Wire* cond, Bundle& then_res, Bundle& else_res, Bundle& out)
    {
        // out = (then_res & cond) | (else_res & cond')
        GASSERT(then_res.size() == else_res.size());
        u32 len = then_res.size();
        Bundle left;
        Bundle right;
        Wire* w;
        Wire* cond_inv;

        out.clear();

        for (u32 i = 0; i < len; ++i) {
            REQUIRE_GOOD_STATUS(evalw_AND(cond, then_res[i], w));
            left.add(w);
        }
        for (u32 i = 0; i < len; ++i) {
            REQUIRE_GOOD_STATUS(evalw_INV(cond, cond_inv));
            REQUIRE_GOOD_STATUS(evalw_AND(cond_inv, else_res[i], w));
            right.add(w);
        }
        for (u32 i = 0; i < len; ++i) {
            REQUIRE_GOOD_STATUS(evalw_OR(left[i], right[i], w));
            out.add(w);
        }
        return 0;
    }

    void write_gate(int op, Wire* in0, Wire* in1, Wire*& out)
    {
        // Mark both wires as used
        in0->used();
        in1->used();

        int v0 = in0->m_v;
        int v1 = in1->m_v;

        if (v0 >= 0 && v1 >= 0) {
            // If both wires are constant

            out->m_v = evalbit(op, v0, v1);
            return;

        } else if (v0 >= 0) {
            // If only in0 is constant

            switch (op) {
            case opAND:

                if (v0 == 0) {
                    out->m_v = 0;
                } else {
                    delete out;
                    out = in1;
                }
                break;

            case opOR:

                if (v0 == 0) {
                    delete out;
                    out = in1;
                } else {
                    out->m_v = 1;
                }
                break;

            case opXOR:

                delete out;
                if (v0 == 0) {
                    out = in1;
                } else {
                    evalw_INV(in1, out);
                }
                break;

            default:
                NOT_YET_IMPLEMENTED("Write gate : " << op);
                break;
            }

        } else if (in1->m_v >= 0) {
            // Only in1 is constant

            switch (op) {
            case opAND:

                if (v1 == 0) {
                    out->m_v = 0;
                } else {
                    delete out;
                    out = in0;
                }
                break;

            case opOR:

                if (v1 == 0) {
                    delete out;
                    out = in0;
                } else {
                    out->m_v = 1;
                }
                break;

            case opXOR:

                delete out;
                if (v1 == 0) {
                    out = in0;
                } else {
                    evalw_INV(in0, out);
                }
                break;

            default:
                NOT_YET_IMPLEMENTED("Write gate : " << op);
                break;
            }
        } else {
            // No wire is constant
            mgc.add_gate(op, in0, in1, out);
        }
    }

    int evalw_FADD(Wire* in0, Wire* in1, Wire*& cin, Wire*& ret)
    {
        Wire* w_xor = nextwire();
        write_gate(opXOR, in0, in1, w_xor);

        Wire* w_s = nextwire();
        write_gate(opXOR, w_xor, cin, w_s);

        Wire* w_and1 = nextwire();
        write_gate(opAND, w_xor, cin, w_and1);

        Wire* w_and2 = nextwire();
        write_gate(opAND, in0, in1, w_and2);

        Wire* w_newcin = nextwire();
        write_gate(opOR, w_and1, w_and2, w_newcin);

        cin = w_newcin;

        ret = w_s;

        return 0;
    }

    int evalw_FSUB(Wire* in0, Wire* in1, Wire*& bout, Wire*& ret)
    {
        Wire* w_xor = nextwire();
        Wire* w_one = onewire();
        write_gate(opXOR, in1, w_one, w_xor);
        return evalw_FADD(w_xor, in0, bout, ret);
    }

    int evalw_AND(Wire* in0, Wire* in1, Wire*& ret)
    {
        ret = nextwire();
        write_gate(opAND, in0, in1, ret);
        return 0;
    }

    int evalw_OR(Wire* in0, Wire* in1, Wire*& ret)
    {
        ret = nextwire();
        write_gate(opOR, in0, in1, ret);
        return 0;
    }

    int evalw_XOR(Wire* in0, Wire* in1, Wire*& ret)
    {
        ret = nextwire();
        write_gate(opXOR, in0, in1, ret);
        return 0;
    }

    int evalw_INV(Wire* in, Wire*& ret)
    {
        // If `in` has not been used
        if (!in->is_used_once()) {
            // If `in` is constant wire
            if (in->m_v >= 0) {
                in->m_v ^= 1;
                ret = in;
            } else {
                // Invert `in`'s id

                if (mgc.is_input_wire(in)) {
                    if (mgc.has_input_dup(in)) {
                        ret = mgc.get_invert_wire(in);
                    } else {
                        ret = nextwire();
                        mgc.set_input_inv_dup(in, ret);
                    }
                } else {
                    in->invert();
                    ret = in;
                }
            }
        } else {
            // `in` is input wire
            if (mgc.is_input_wire(in)) {
                // If `in` has input invert duplicate
                if (mgc.has_input_dup(in)) {
                    ret = mgc.get_invert_wire(in);
                } else {
                    // `in` has no invert duplicate
                    ret = nextwire();
                    mgc.set_input_inv_dup(in, ret);
                }

            } else {
                // If `in` is not input wire
                // If `in` already has inverted wire
                if (mgc.has_invert_wire(in)) {
                    ret = mgc.get_invert_wire(in);
                } else {
                    // If `in` does not have inverted wire

                    // If `in` is constant
                    if (in->m_v >= 0) {
                        ret = nextwire();
                        ret->m_v = in->m_v ^ 1;
                    } else {
                        // `in` is not constant, make a duplicate of the gate
                        Gate* parent_gate = in->m_parent_gate;
                        REQUIRE_NOT_NULL(parent_gate);
                        ret = nextwire();
                        ret->invert_from(in);
                        mgc.set_invert_wire(in, ret);
                        write_gate(parent_gate->m_op, parent_gate->m_in0, parent_gate->m_in1,
                            ret);
                    }
                }
            }
        }
        return 0;
    }

    int evalbit(int op, int v0, int v1)
    {
        return getbit(op, (v0 + (v1 << 1)));
    }
} // namespace gashlang
