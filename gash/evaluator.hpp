//
//  evaluator.hpp
//  gash
//
//  Created by Xiaoting Tang on 6/17/18.
//  Copyright © 2018 Xiaoting Tang. All rights reserved.
//

#ifndef evaluator_hpp
#define evaluator_hpp

#include "util.hpp"
#include "circuit.hpp"
#include "garbled_circuit.hpp"
#include "ot.hpp"
#include "common.hpp"

#include <iostream>
#include <algorithm>
using std::cout;
using std::endl;

class Evaluator {
public:
    
    /// The garbled circuit instance
    GC                          m_gc;
    
    /// The set of input ids
    IdSet                       m_self_in_id_set;
    IdSet                       m_peer_in_id_set;
    
    /// Network related stuff
    int                         m_peer_sock;
    int                         m_peer_ot_sock;
    string                      m_self_ip;      // For debug purpose
    string                      m_peer_ip;
    uint16_t                    m_ot_port;
    
    void init_connection() {
        tcp_client_init(m_peer_ip, PORT_GC, m_peer_sock);
    }
    
    void check_ids() {
        for (auto it = m_gc.m_in_id_set.begin(); it != m_gc.m_in_id_set.end(); ++it) {
            WI* wi = m_gc.get_wi(*it);
            if (wi->m_wire->m_val == 0 || wi->m_wire->m_val == 1) {
                m_self_in_id_set.insert(*it);
            } else {
                m_peer_in_id_set.insert(*it);
            }
        }
    }
    
    void build_garble_circuit() {
        for (auto it = m_gc.m_wi_map.begin(); it != m_gc.m_wi_map.end(); ++it) {
            WI* wi = it->second;
            GWI* gwi = new GWI(wi);
            m_gc.add_gwi(gwi);
        }
        
        for (auto it = m_gc.m_gate_map.begin(); it != m_gc.m_gate_map.end(); ++it) {
            Gate* g = it->second;
            WI* in0 = g->m_in0;
            WI* in1 = g->m_in1;
            WI* out = g->m_out;
            GWI* gin0 = m_gc.get_gwi(in0->m_wire->m_id);
            GWI* gin1 = m_gc.get_gwi(in1->m_wire->m_id);
            GWI* gout = m_gc.get_gwi(out->m_wire->m_id);
            GG* gg = new GG(g->m_func, gin0, gin1, gout);
            m_gc.add_gg(gg);
        }
        
        m_gc.m_gwi_one = m_gc.get_gwi(m_gc.m_wi_one->m_wire->m_id);
        m_gc.m_gwi_zero = m_gc.get_gwi(m_gc.m_wi_zero->m_wire->m_id);
    }
    
    void recv_egtt() {
        uint32_t id;
        GG* gg;
        uint32_t size;
        uint32_t magic_num;
        block row1;
        block row2;
        block row3;
        
        tcp_recv_bytes(m_peer_sock, (char*)&size, sizeof(uint32_t));
        assert(size == m_gc.m_gg_map.size()); // Assert that peer is sending the same number of gates
        
        for (uint32_t i = 0; i < size; ++i) {
            
            tcp_recv_bytes(m_peer_sock, (char*)&id, sizeof(uint32_t));
            tcp_recv_bytes(m_peer_sock, (char*)&magic_num, sizeof(uint32_t));
            
            // XOR gate, skip it since we need no egtt for xor gates
            if (magic_num == XORNUM) {
                
                continue;
                
            } else if (magic_num == NONXORNUM) {
                // Non-xor gates, receive egtt
                
                tcp_recv_bytes(m_peer_sock, (char*)&row1, LABELSIZE);
                tcp_recv_bytes(m_peer_sock, (char*)&row2, LABELSIZE);
                tcp_recv_bytes(m_peer_sock, (char*)&row3, LABELSIZE);
                
                gg = m_gc.get_gg(id);
                assert(gg != NULL);
                
                gg->m_egtt[1] = row1;
                gg->m_egtt[2] = row2;
                gg->m_egtt[3] = row3;
                
            } else {
                perror("Invalid magic number");
                abort();
            }
        }
    }
    
    void recv_self_lbls() {
        map<int, block> idlblmap;
        map<int, int>  in_val_map;
        GWI* gw;
        uint32_t size;
        
        // Build in_val_map
        for (auto it = m_gc.m_gwi_map.begin(); it != m_gc.m_gwi_map.end(); ++it) {
            GWI* gwi = it->second;
            if (gwi->m_wire->m_val == 0 || gwi->m_wire->m_val == 1) {
                if (gwi == m_gc.m_gwi_one || gwi == m_gc.m_gwi_zero)
                    continue;
                in_val_map.emplace(gwi->m_wire->m_id, gwi->m_wire->m_val);
            }
        }
        
        tcp_recv_bytes(m_peer_sock, (char*)&size, sizeof(uint32_t));
        
        OTParty otp;
        otp.OTRecv(m_peer_ip, PORT_OT, in_val_map, idlblmap);
        
        for (auto it = idlblmap.begin(); it != idlblmap.end(); ++it) {
            gw = m_gc.get_gwi(it->first);
            assert(gw != NULL);
            gw->set_lbl(it->second);
        }
        
    }
    
    void recv_peer_lbls() {
        uint32_t id;
        uint32_t size;
        GWI* gw;
        block lbl;
        
        tcp_recv_bytes(m_peer_sock, (char*)&size, sizeof(uint32_t));
        
        for (uint32_t i = 0; i < size; ++i) {
            
            tcp_recv_bytes(m_peer_sock, (char*)&id, sizeof(uint32_t));
            tcp_recv_bytes(m_peer_sock, (char*)&lbl, LABELSIZE);
            
            gw = m_gc.get_gwi(id);
            if (!gw) {
                perror("Cannot find value for wire id");
                abort();
            }
            
            gw->set_lbl(lbl);
        }
        
        tcp_recv_bytes(m_peer_sock, (char*)&lbl, LABELSIZE);
        m_gc.m_gwi_zero->set_lbl(lbl);
        tcp_recv_bytes(m_peer_sock, (char*)&lbl, LABELSIZE);
        m_gc.m_gwi_one->set_lbl(lbl);
    }
    
    void recv_output_map() {
        uint32_t id;
        uint32_t size;
        GWI* gw;
        block lbl0;
        block lbl1;
        
        tcp_recv_bytes(m_peer_sock, (char*)&size, sizeof(uint32_t));
        
        for (uint32_t i = 0; i < size; ++i) {
            
            tcp_recv_bytes(m_peer_sock, (char*)&id, sizeof(uint32_t));
            tcp_recv_bytes(m_peer_sock, (char*)&lbl0, LABELSIZE);
            tcp_recv_bytes(m_peer_sock, (char*)&lbl1, LABELSIZE);
            
            if (std::find(m_gc.m_out_id_vec.begin(), m_gc.m_out_id_vec.end(), id) == m_gc.m_out_id_vec.end()) {
                perror("Cannot find id in output id set");
                abort();
            }
            
            gw = m_gc.get_gwi(id);
            if (!gw) {
                perror("Cannot find value for wire id");
                abort();
            }
            
            gw->set_lbl0(lbl0);
            gw->set_lbl1(lbl1);
        }
    }
    
    void recover_output() {
        uint32_t id;
        GWI* gw;
        WI*  w;
        int val;
        
        for (auto it = m_gc.m_out_id_vec.begin(); it != m_gc.m_out_id_vec.end(); ++it) {
            
            id = *it;
            w = m_gc.m_wi_map.find(id)->second;
            gw = m_gc.get_gwi(id);
            
            // If wire is constant
            if (w->m_wire->m_val == 0 || w->m_wire->m_val == 1) {
                gw->m_wire->m_val = w->m_wire->m_val;
                continue;
            }
            
            // Otherwise
            val = gw->recover_smtc();
            if (val < 0) {
                perror("Negative value");
                abort();
            }
            
        }
    }
    
    void send_output() {
        uint32_t id;
        uint32_t size;
        int val;
        
        size = m_gc.m_out_id_vec.size();
        tcp_send_bytes(m_peer_sock, (char*)&size, sizeof(uint32_t));
        
        for (auto it = m_gc.m_out_id_vec.begin(); it != m_gc.m_out_id_vec.end(); ++it) {
            
            id = *it;
            GWI* gwi = m_gc.get_gwi(id);
            val = gwi->m_gw->m_val;
            
            tcp_send_bytes(m_peer_sock, (char*)&id, sizeof(uint32_t));
            tcp_send_bytes(m_peer_sock, (char*)&val, sizeof(int));
        }
    }
    
    void report_output() {
        uint32_t id;
        
        for (auto it = m_gc.m_out_id_vec.begin(); it != m_gc.m_out_id_vec.end(); ++it) {
            id = *it;
            GWI* gwi = m_gc.get_gwi(id);
            int val = gwi->m_gw->m_val;
            cout << id << ":" << val << endl;
        }
    }
    
    void get_output(string& str) {
        uint32_t id;
        for (auto it = m_gc.m_out_id_vec.begin(); it != m_gc.m_out_id_vec.end(); ++it) {
            id = *it;
            GWI* gwi = m_gc.get_gwi(id);
            int val = gwi->m_gw->m_val;
            str += to_string(val);
        }
        
        string cpy(str);
        std::reverse(cpy.begin(), cpy.end());
        str = cpy;
    }

    void clear() {
        m_gc.clear();
        m_self_in_id_set.clear();
        m_peer_in_id_set.clear();
        shutdown(m_peer_sock, SHUT_WR);
        close(m_peer_sock);
        m_peer_sock = INVALID_SOCKET;
    }
    
    Evaluator() {}
    Evaluator(string peer_ip) {
        m_peer_ip = peer_ip;
    }
    
    ~Evaluator() {
        clear();
    }
};

#endif /* evaluator_hpp */
