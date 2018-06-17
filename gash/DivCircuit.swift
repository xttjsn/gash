//
//  DivCircuit.swift
//  CircuitExecutor
//
//  Created by Xiaoting Tang on 6/16/18.
//  Copyright © 2018 Xiaoting Tang. All rights reserved.
//

import Cocoa


enum GateOp: Int {
    case opXOR = 6
    case opAND = 8
    case opOR  = 14
    case opIAND = 1
}

class DivCircuit: Circuit {
    let m_bitsize : Int
    var m_denom = 0
    var m_nume = 0
    
    init(bitsize: Int)  {
        m_bitsize = bitsize
    }
    
    func build() {
        var in0 : [WI] = [WI]()
        for i in 0...m_bitsize - 1 {
            // Input 0
            let wi = nextwi()
            wi.m_wire.m_val = getbit(n: m_denom, i: i)
            in0.append(wi)
            m_in_id_set.insert(wi.m_wire.m_id)
        }
        
        var in1: [WI] = [WI]()
        for i in 0...m_bitsize - 1{
            // Input 1
            let wi = nextwi()
            wi.m_wire.m_val = getbit(n: m_nume, i: i)
            in1.append(wi)
            m_in_id_set.insert(wi.m_wire.m_id)
        }
        
//        let out = evala_DIV(a: in0, b: in1)
        let out = evala_ADD(a: in0, b: in1)
        
        for out_w in out {
            m_out_id_vec.append(out_w.m_wire.m_id)
        }
        
    }

    func set_denom(denom: Int) {
        m_denom  = denom
    }
    
    func set_nume(nume: Int) {
        m_nume = nume
    }
}
