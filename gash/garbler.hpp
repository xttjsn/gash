//
//  garbler.hpp
//  CircuitExecutor
//
//  Created by Xiaoting Tang on 6/17/18.
//  Copyright © 2018 Xiaoting Tang. All rights reserved.
//

#ifndef garbler_hpp
#define garbler_hpp

#include <stdio.h>
#include "garbled_circuit.hpp"
#include "tcp.hpp"
#include "ot.hpp"

class Garbler {
public:
    GC m_gc;
    
    IdSet m_self_in_id_set;
    IdSet m_peer_in_id_set;
    IdValMap m_in_val_map;
    IdValMap m_out_val_map;

    
    /// Network related stuff
    int m_peer_sock;
    int m_peer_ot_sock;
    int m_listen_sock;
    string m_self_ip;
    string m_peer_ip;
    uint16_t m_port;
    uint16_t m_ot_port;
    
    void init_connection() {
        tcp_server_init(m_port, m_listen_sock, m_peer_sock);
    }
    
    void send_egtt() {
        int id;
        GG* gg;
        int size;
        
        size = (int)m_gc.m_gg_map.size();
        tcp_send_bytes(m_peer_sock, (char*)&size, sizeof(int));
        
        for (auto it = m_gc.m_gg_map.begin(); it != m_gc.m_gg_map.end(); ++it) {
            
            id = it->first;
            gg = it->second;
            
            tcp_send_bytes(m_peer_sock, (char*)&id, sizeof(int));
            
            int tag;
            // If it is an xor gate, just tell the peer by sending this magic number
            if (gg->m_is_xor) {
                tag = XORNUM;
                tcp_send_bytes(m_peer_sock, (char*)&tag, sizeof(int));
                
            } else {
                // If not, send rows in the EGTT
                tag = NONXORNUM;
                tcp_send_bytes(m_peer_sock, (char*)&tag, sizeof(int));
                tcp_send_bytes(m_peer_sock, (char*)(gg->m_egtt + 1), LABELSIZE);
                tcp_send_bytes(m_peer_sock, (char*)(gg->m_egtt + 2), LABELSIZE);
                tcp_send_bytes(m_peer_sock, (char*)(gg->m_egtt + 3), LABELSIZE);
            }
        }
    }
    
    void send_self_lbls() {
        int size;
        int id;
        int val;
        block lbl;
        
        size = m_self_in_id_set.size();
        tcp_send_bytes(m_peer_sock, (char*)&size, sizeof(int));
        
        for (auto it = m_self_in_id_set.begin(); it != m_self_in_id_set.end(); ++it) {
            
            id = *it;
            auto val_it = m_in_val_map.find(id);
            assert(val_it != m_in_val_map.end());
            val = val_it->second;
            assert(val == 0 || val == 1);
            
            m_gc.get_lbl(id, val, lbl);
            
            tcp_send_bytes(m_peer_sock, (char*)&id, sizeof(int));
            tcp_send_bytes(m_peer_sock, (char*)&lbl, LABELSIZE);
        }
    }
    
    void send_peer_lbls() {
        int id;
        int size;
        block lbl0;
        block lbl1;
        LabelVec lbl0vec;
        LabelVec lbl1vec;
        
        size = m_peer_in_id_set.size();
        tcp_send_bytes(m_peer_sock, (char*)&size, sizeof(int));
        
        for (auto it = m_peer_in_id_set.begin(); it != m_peer_in_id_set.end(); ++it) {
            
            id = *it;
            
            m_gc.get_lbl(id, 0, lbl0);
            m_gc.get_lbl(id, 1, lbl1);
            
            lbl0vec.emplace_back(lbl0);
            lbl1vec.emplace_back(lbl1);
        }
        
        // Call OTSend
        OTParty otp;
        otp.OTSend(m_peer_ip, m_ot_port, lbl0vec, lbl1vec);
        
    }
    
    void send_output_map() {
        int id;
        int size;
        block lbl0;
        block lbl1;
        WI* w;
        
        size = m_gc.m_out_id_vec.size();
        for (auto it = m_gc.m_out_id_vec.begin(); it != m_gc.m_out_id_vec.end(); ++it) {
            
            id = *it;
            w = m_gc.m_wi_map.find(id)->second;
            if (w->m_wire->m_val == 0 || w->m_wire->m_val == 1) {
                // Constant output
                size--;
            }
        }
        
        tcp_send_bytes(m_peer_sock, (char*)&size, sizeof(int));
        for (auto it = m_gc.m_out_id_vec.begin(); it != m_gc.m_out_id_vec.end(); ++it) {
            
            id = *it;
            w = m_gc.m_wi_map.find(id)->second;
            if (w->m_wire->m_val == 0 || w->m_wire->m_val == 1) {
                // Constant output
                continue;
            }
            
            // Non-constant output
            m_gc.get_orig_lbl(id, 0, lbl0);
            m_gc.get_orig_lbl(id, 1, lbl1);
            
            tcp_send_bytes(m_peer_sock, (char*)&id, sizeof(int));
            tcp_send_bytes(m_peer_sock, (char*)&lbl0, LABELSIZE);
            tcp_send_bytes(m_peer_sock, (char*)&lbl1, LABELSIZE);
        }
    }
    
    void recv_output() {
        int id;
        int size;
        int val;
        
        tcp_recv_bytes(m_peer_sock, (char*)&size, sizeof(int));
        if (size != m_gc.m_out_id_vec.size()) {
            perror("Output size inconsistent");
            abort();
        }
        
        for (int i = 0; i < size; ++i) {
            tcp_recv_bytes(m_peer_sock, (char*)&id, sizeof(int));
            tcp_recv_bytes(m_peer_sock, (char*)&val, sizeof(int));
            m_out_val_map.emplace(id, val);
        }
    }
    
    void get_output(string& str) {
        int id;
        for (auto it = m_gc.m_out_id_vec.begin(); it != m_gc.m_out_id_vec.end(); ++it) {
            id = *it;
            str += std::to_string(m_out_val_map.find(id)->second);
        }
        
        string cpy(str);
        std::reverse(cpy.begin(), cpy.end());
        str = cpy;
    }
    
    void report_output() {
        int id;
        for (auto it = m_gc.m_out_id_vec.begin(); it != m_gc.m_out_id_vec.end(); ++it) {
            id = *it;
            printf("%d:%d\n", id, m_out_val_map.find(id)->second);
        }
        
    }

    void reset_circ() {
        m_gc = GC();
        m_in_val_map.clear();
        m_out_val_map.clear();
        m_self_in_id_set = IdSet();
        m_peer_in_id_set = IdSet();
    }

    Garbler() {}
    
    Garbler(string peer_ip, uint16_t port, uint16_t ot_port) {
        m_peer_ip = peer_ip;
        m_port = port;
        m_ot_port = ot_port;
    }
    
    ~Garbler() {
        shutdown(m_listen_sock, SHUT_WR);
        close(m_listen_sock);
        
        shutdown(m_peer_sock, SHUT_WR);
        close(m_peer_sock);
    }
};

#endif /* garbler_hpp */
