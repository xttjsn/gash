//
//  Circuit.swift
//  CircuitExecutor
//
//  Created by Xiaoting Tang on 6/16/18.
//  Copyright © 2018 Xiaoting Tang. All rights reserved.
//

import Cocoa

typealias WI = WireInstance
typealias W = Wire

func is_odd(n: Int) -> Bool {
    return n % 2 != 0
}

func getbit(n: Int, i: Int) -> Int {
    return ((n >> i) & 1)
}

func eval(f: Int, a: Int, b: Int) -> Int {
    return getbit(n: f, i: (a + (b << 1)))
}

class Wire: NSObject {
    var m_id = -1
    var m_val = -1
    
    init(id: Int) {
        m_id = id;
    }
    
    init(id: Int, val: Int) {
        m_id = id
        m_val = val
    }
}

class WireInstance: NSObject {
    var m_wire: Wire
    var m_inv: Bool
    var m_inv_count = 0
    init(wire: Wire, inv: Bool) {
        m_wire = wire
        m_inv = inv
    }
    init(id: Int, inv: Bool) {
        m_wire = Wire(id: id, val: -1)
        m_inv = inv
    }
    
    func invert() {
        m_inv = !m_inv
        if m_inv_count != 0 {
            print("Wire is inverted twice!")
            fatalError()
        }
        m_inv_count += 1
    }
}

class Gate: NSObject {
    var m_func: Int
    var m_in0: WireInstance
    var m_in1: WireInstance
    var m_out: WireInstance
    
    init(funct: Int, out: WI, in0: WI, in1: WI) {
        m_func = funct; m_out = out; m_in0 = in0; m_in1 = in1
    }
    
    func get_id() -> Int {
        return m_out.m_wire.m_id
    }
}

class Circuit: NSObject {
    var m_wi_map: [Int: WI] = [:]
    var m_gate_map: [Int: Gate] = [:]
    var m_in_id_set = Set<Int>()
    var m_out_id_set = Set<Int>()
    var m_out_id_vec = [Int]()
    var m_wi_inv_map: [Int: Int] = [:]
    var m_wcount = 0
    
    func get_wi(id: Int) -> WI? {
        return m_wi_map[id]
    }
    
    func get_gate(id: Int) -> Gate? {
        return m_gate_map[id]
    }
    
    func add_wi(wi: WI) {
        if m_wi_map.keys.contains(wi.m_wire.m_id) {
            print("Duplicate wire addition")
            fatalError();
        }
        m_wi_map[wi.m_wire.m_id] = wi
    }
    
    func add_gate(g: Gate) {
        if m_gate_map.keys.contains(g.get_id()) {
            print("Duplicate gate addition")
            fatalError()
        }
        m_gate_map[g.get_id()] = g
    }
    
    func has_gate(id: Int) -> Bool {
        return m_gate_map.keys.contains(id)
    }
    
    func create_gate(funct: Int, outid: Int, in0id: Int, in1id: Int, in0inv: Bool, in1inv: Bool) {
        if has_gate(id: outid) {
            print("Gate already exists")
            fatalError()
        }
        
        var out = get_wi(id: outid)
        let in0 = get_wi(id: in0id)
        let in1 = get_wi(id: in1id)
        
        if out == nil {
            out = WI(id: outid, inv: false)
            add_wi(wi: out!)
        }
        
        if in0 == nil || in1 == nil {
            fatalError()
        }
        
        if in0!.m_inv != in0inv {
            in0!.invert()
        }
        
        if in1!.m_inv != in1inv {
            in1!.invert()
        }
        
        let g = Gate(funct: funct, out: out!, in0: in0!, in1: in1!)
        add_gate(g: g)
    }
    
    func build_from_file(circ_fpath: String) {
        if let sreader = StreamReader(path: circ_fpath) {
            defer {
                sreader.close()
            }
            while let line = sreader.nextLine() {
                if line.contains("#") {
                    continue
                }
                var items = line.components(separatedBy: " ")
                
                if items.count == 1 {
                    var subitems = items[0].components(separatedBy: ":")
                    
                    
                    if subitems[0] == "I" {
                        // Input wire
                        if let idx = Int(subitems[1])  {
                            let id = idx / 2
                            let wi = WI(id: id, inv: is_odd(n: idx))
                            add_wi(wi: wi)
                            m_in_id_set.insert(id)
                            
                        } else {
                            print("Invalid input number")
                            fatalError()
                        }
                        
                    } else if subitems[0] == "O" {
                        // Output wire
                        if let idx = Int(subitems[1])  {
                            let id = idx / 2
                            let wi = WI(id: id, inv: is_odd(n: idx))
                            
                            if subitems.count == 3 {
                                if let val = Int(subitems[2]) {
                                    assert(val == 0 || val == 1)
                                    wi.m_wire.m_val = val
                                } else {
                                    print("Invalid value")
                                    fatalError()
                                }
                            }
                            
                            add_wi(wi: wi)
                            m_out_id_set.insert(id)
                            m_out_id_vec.append(id)
                            
                        } else {
                            print("Invalid input number")
                            fatalError()
                        }
                    }
                }
                else if items.count == 4 {
                    let funct = Int(items[1])!
                    let outidx = Int(items[0])!
                    let outid = outidx / 2
                    let in0idx = Int(items[2])!
                    let in0id = in0idx / 2
                    let in1idx = Int(items[3])!
                    let in1id  = in1idx / 2
                    
                    assert(0 < funct && funct < 16)
                    assert(!is_odd(n: outidx))
                    
                    create_gate(funct: funct, outid: outid, in0id: in0id, in1id: in1id, in0inv: is_odd(n: in0idx), in1inv: is_odd(n: in1idx))
                }
                else {
                    continue
                }
            }
        } else {
            print("File " + circ_fpath + " does not exists")
            fatalError()
        }
    }

    func read_input(data_fpath: String) {
        if let sreader = StreamReader(path: data_fpath) {
            defer {
                sreader.close()
            }
            while let line = sreader.nextLine() {
                if line.contains("#") {
                    continue
                }
                var items = line.components(separatedBy: " ")
                
                if items.count == 2 {
                    let id = Int(items[0])! / 2
                    let val = Int(items[1])!
                    assert(val == 0 || val == 1)
                    
                    if let wi = get_wi(id: id) {
                        wi.m_wire.m_val = val
                    } else {
                        print("Wire instance not found!")
                        fatalError()
                    }
                }
            }
        }
    }

    func execute() {
        let m_gate_sorted_map = m_gate_map.sorted(by: { $0.key < $1.key })
        for (_, g) in m_gate_sorted_map {
            let in0 = g.m_in0
            let in1 = g.m_in1
            let out = g.m_out
            let funct = g.m_func
            if (in0.m_wire.m_val < 0 || in1.m_wire.m_val < 0) {
                print("Wire has negative value")
                fatalError()
            }
            out.m_wire.m_val = eval(f: funct,
                                    a: in0.m_inv ? in0.m_wire.m_val ^ 1 : in0.m_wire.m_val,
                                    b: in1.m_inv ? in1.m_wire.m_val ^ 1 : in1.m_wire.m_val)
            
            print("out wire %d val:  %d\n", out.m_wire.m_id, out.m_wire.m_val);
            
        }
        m_wcount = 0
    }
    
    func get_output_str() -> String {
        var out = ""
        for id in m_out_id_vec {
            let wi = get_wi(id: id)!
            let v = wi.m_inv ? wi.m_wire.m_val ^ 1 : wi.m_wire.m_val
            out += String(v)
        }
        out = String(out.reversed())
        return out
    }
    
    func nextwi() -> WI {
        m_wcount += 1
        let wi = WI(id: m_wcount, inv: false)
        add_wi(wi: wi)
        return wi
    }
    
    func zerowi() -> WI {
        let wi = nextwi()
        wi.m_wire.m_val = 0
        return wi
    }
    
    func onewi() -> WI {
        let wi = nextwi()
        wi.m_wire.m_val = 1
        return wi
    }
    
    func write_gate(op: GateOp, a: WI, b: WI, c: WI) {
        let g = Gate(funct: op.rawValue, out: c, in0: a, in1: b)
        add_gate(g: g)
    }
    
    func evalw_XOR(a: WI, b: WI) -> WI {
        let ret = nextwi()
        write_gate(op: .opXOR, a: a, b: b, c: ret)
        return ret
    }
    
    func evalw_AND(a: WI, b: WI) -> WI {
        let ret = nextwi()
        write_gate(op: .opAND, a: a, b: b, c: ret)
        return ret
    }
    
    func evalw_OR(a: WI, b: WI) -> WI {
        let ret = nextwi()
        write_gate(op: .opOR, a: a, b: b, c: ret)
        return ret
    }
    
    func evalw_IAND(a: WI, b: WI) -> WI {
        let ret = nextwi()
        write_gate(op: .opIAND, a: a, b: b, c: ret)
        return ret
    }
    
    func evalw_INV(a: WI) -> WI {
        if m_wi_inv_map.keys.contains(a.m_wire.m_id) {
            return get_wi(id: m_wi_inv_map[a.m_wire.m_id]!)!
        } else {
            
            // Input wire
            if m_in_id_set.contains(a.m_wire.m_id) {
                
                let w_inv = nextwi()
                w_inv.m_wire.m_val = 1 ^ a.m_wire.m_val
                m_wi_inv_map[a.m_wire.m_id] = w_inv.m_wire.m_id
                m_wi_inv_map[w_inv.m_wire.m_id] = a.m_wire.m_id
                return w_inv
                
            } else {
                if let g = get_gate(id: a.m_wire.m_id) {
                    let new_out = nextwi()
                    new_out.m_wire.m_val = a.m_wire.m_val
                    new_out.m_inv = !a.m_inv
                    let new_g = Gate(funct: g.m_func, out: new_out, in0: g.m_in0, in1: g.m_in1)
                    add_gate(g: new_g)
                    m_wi_inv_map[g.m_out.m_wire.m_id] = new_out.m_wire.m_id
                    m_wi_inv_map[new_out.m_wire.m_id] = g.m_out.m_wire.m_id
                    return new_out
                }
                    // Constant wires
                else {
                    if (a.m_wire.m_val == 0) {
                        return onewi()
                    } else {
                        return zerowi()
                    }
                }
            }
        }
    }
    
    func evalw_FADD(a: WI, b: WI, cin: inout WI) -> WI {
        let w_xor = evalw_XOR(a: a, b: b)
        let w_sum = evalw_XOR(a: w_xor, b: cin)
        let w_and1 = evalw_AND(a: w_xor, b: cin)
        let w_and2 = evalw_AND(a: a, b: b)
        let w_newcin = evalw_OR(a: w_and1, b: w_and2)
        cin = w_newcin
        return w_sum
    }
    
    func evalc_LA(a: [WI], b: [WI]) -> WI {
        assert(a.count == b.count)
        assert(a.count > 1)
        var xors = [WI]()
        for i in 0...a.count - 1 {
            let w = evalw_XOR(a: a[i], b: b[i])
            let w_inv = evalw_INV(a: w)
            xors.append(w_inv)
        }
        
        var ret : WI
        let inv_a = evalw_INV(a: a.last!)
        ret = evalw_AND(a: inv_a, b: b.last!)
        
        var used_xor = onewi()
        let iand_a_b = evalw_IAND(a: a.last!, b: b.last!)
        let and_a_b  = evalw_AND(a: a.last!, b: b.last!)
        
        for i in 1...a.count - 1 {
            let inv_b = evalw_INV(a: b[a.count - i - 1])
            let and_a_ib = evalw_AND(a: a[a.count - i - 1], b: inv_b)
            let and_a_ib_xor = evalw_AND(a: and_a_ib, b: used_xor)
            let irow = evalw_AND(a: and_a_ib_xor, b: iand_a_b)
            ret = evalw_OR(a: ret, b: irow)
            
            let inv_a = evalw_INV(a: a[a.count - i - 1])
            let and_ia_b = evalw_AND(a: b[a.count - i - 1], b: inv_a)
            let and_ia_b_xor = evalw_AND(a: and_ia_b, b: used_xor)
            let jrow = evalw_AND(a: and_ia_b_xor, b: and_a_b)
            ret = evalw_OR(a: ret, b: jrow)
            
            used_xor = evalw_AND(a: used_xor, b: xors[a.count - i - 1])
        }
        
        return ret
    }
    
    func evalc_EQ(a: [WI], b: [WI]) -> WI {
        assert(a.count == b.count)
        var ret = onewi()
        for i in 0...a.count - 1 {
            let w = evalw_XOR(a: a[i], b: b[i])
            let w_inv = evalw_INV(a: w)
            ret = evalw_AND(a: w_inv, b: ret)
        }
        return ret
    }
    
    func evalc_LAE(a: [WI], b: [WI]) -> WI {
        let w_la = evalc_LA(a: a, b: b)
        let w_eq = evalc_EQ(a: a, b: b)
        let w_lae = evalw_OR(a: w_la, b: w_eq)
        return w_lae
    }
    
    func evala_ADD(a: [WI], b: [WI]) -> [WI] {
        assert(a.count == b.count)
        var w_cin = zerowi()
        var out = [WI]()
        for i in 0...a.count-1 {
            let w_a = a[i]
            let w_b = b[i]
            let w_sum = evalw_FADD(a: w_a, b: w_b, cin: &w_cin)
            out.append(w_sum)
        }
        return out
    }
    
    func evala_UMINUS(a: [WI]) -> [WI] {
        var tmp0 = [WI]()
        var tmp1 = [WI]()
        for i in 0...a.count-1 {
            let w_inv = evalw_INV(a: a[i])
            tmp0.append(w_inv)
        }
        
        tmp1.append(onewi())
        for _ in 1...a.count-1 {
            tmp1.append(zerowi())
        }
        
        return evala_ADD(a: tmp0, b: tmp1)
    }
    
    func evala_SUB(a: [WI], b: [WI]) -> [WI] {
        let b_neg = evala_UMINUS(a: b)
        return evala_ADD(a: a, b: b_neg)
    }
    
    func evala_DVG(a: [WI], b: [WI]) -> ([WI], WI) {
        assert(a.count == b.count)
        let a_LAE_b = evalc_LAE(a: a, b: b)
        let a_LE_b  = evalw_INV(a: a_LAE_b)
        let sub = evala_SUB(a: a, b: b)
        
        var out = [WI]()
        for i in 0...a.count - 1 {
            let w = evalw_AND(a: a_LAE_b, b: sub[i])
            let w_orig = evalw_AND(a: a_LE_b, b: a[i])
            let w_final = evalw_OR(a: w, b: w_orig)
            out.append(w_final)
        }
        
        return (out, a_LAE_b)
    }
    
    func evala_DIV(a: [WI], b: [WI]) -> [WI] {
        assert(a.count == b.count)
        var out = [WI]()
        let len = a.count
        let w_sign_different = evalw_XOR(a: a.last!, b: b.last!)
        let w_sign_same = evalw_INV(a: w_sign_different)
        let w_a_negative = a.last!
        let w_b_negative = b.last!
        let w_a_positive = evalw_INV(a: w_a_negative)
        let w_b_positive = evalw_INV(a: w_b_negative)
        
        var a_inv = evala_UMINUS(a: a)
        var b_inv = evala_UMINUS(a: b)
        
        var active_a = [WI]()
        for i in 0...len-1 {
            let w = evalw_AND(a: a[i], b: w_a_positive)
            let w_inv = evalw_AND(a: a_inv[i], b: w_a_negative)
            let w_final = evalw_OR(a: w, b: w_inv)
            active_a.append(w_final)
        }
        
        var active_b = [WI]()
        for i in 0...len-1 {
            let w = evalw_AND(a: b[i], b: w_b_positive)
            let w_inv = evalw_AND(a: b_inv[i], b: w_b_negative)
            let w_final = evalw_OR(a: w, b: w_inv)
            active_b.append(w_final)
        }
        
        var sub_res = [WI]()
        for _ in 1...len - 1 {
            sub_res.append(zerowi())
        }
        
        var tmp = [WI]()
        for i in 0...len - 1 {
            tmp.append(active_a[len - i - 1])
            for j in 0...len - 2 {
                tmp.append(sub_res[j])
            }
            
            sub_res.removeAll()
            let r : WI
            (sub_res, r) = evala_DVG(a: tmp, b: active_b)
            out.insert(r, at: 0)
            tmp.removeAll()
        }
        
        // Change sign back
        let out_inv = evala_UMINUS(a: out)
        for i in 0...out.count - 1  {
            let w = evalw_AND(a: out[i], b: w_sign_same)
            let w_inv = evalw_AND(a: out_inv[i], b: w_sign_different)
            let w_final = evalw_OR(a: w, b: w_inv)
            out[i] = w_final
        }
        return out
    }
}
