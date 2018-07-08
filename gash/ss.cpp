//
//  ss.cpp
//  gash
//
//  Created by Xiaoting Tang on 6/20/18.
//  Copyright © 2018 Xiaoting Tang. All rights reserved.
//

#include "ss.hpp"
#include "garbler.hpp"
#include "evaluator.hpp"
#include "func.hpp"
#include "tcp.hpp"

using std::ofstream;
using std::fputs;
using std::rewind;

static mpz_class m_config_l;
static mpz_class m_config_l_1;
static mpz_class m_config_s;
static mpz_class m_config_k;
static mpz_class m_config_l_s;

static Garbler m_garbler;
static Evaluator m_evaluator;
static int m_ss_listen_sock;
static int m_ss_peer_sock;
static int m_ss_client_sock;

static int m_ss_p0_sock;
static int m_ss_p1_sock;
static char m_cname[128];
static char m_dname[128];
static int m_id = -1;

static stack<triplet_t> m_tri_stack;
static gmp_randclass gmp_prn(gmp_randinit_default);

// Initialization
void gash_config_init()
{
    mpz_class one = 1;
    m_config_l = 1;
    m_config_l_1 = 1;
    m_config_s = 1;
    m_config_k = 1;
    m_config_l_s = 1;
    mpz_mul_2exp(m_config_l.get_mpz_t(), one.get_mpz_t(), CONFIG_L);
    mpz_mul_2exp(m_config_l_1.get_mpz_t(), one.get_mpz_t(), CONFIG_L - 1);
    mpz_mul_2exp(m_config_s.get_mpz_t(), one.get_mpz_t(), CONFIG_S);
    mpz_mul_2exp(m_config_k.get_mpz_t(), one.get_mpz_t(), CONFIG_K);
    mpz_mul_2exp(m_config_l_s.get_mpz_t(), one.get_mpz_t(), CONFIG_L_S);
    gmp_prn.seed(time(NULL));
}

static void exec_asym(string circ_src, string data_src)
{
    // Asymmetric execution, meaning that only the evaluator gets the output
    // Write circuit and data to a temporary file
    extern FILE* yyin;
    yyin = std::tmpfile();
    fputs(circ_src.c_str(), yyin);
    fputs(data_src.c_str(), yyin);
    rewind(yyin);
    
    
    if (m_id == 0)
    {
        set_garbler(&m_garbler);
        yyparse();
        m_garbler.check_ids();
        m_garbler.m_gc.garble();
        m_garbler.init_connection();
        m_garbler.send_egtt();
        m_garbler.send_self_lbls();
        m_garbler.send_peer_lbls();
        m_garbler.send_output_map();
    } else
    {
        set_evaluator(&m_evaluator);
        m_evaluator.check_ids();
        m_evaluator.build_garble_circuit();
        m_evaluator.init_connection();
        m_evaluator.recv_egtt();
        m_evaluator.recv_peer_lbls();
        m_evaluator.recv_self_lbls();
        m_evaluator.recv_output_map();
        m_evaluator.m_gc.evaluate();
        m_evaluator.recover_output();
    }
    parse_clean();
}

static void exec_sym(string circ_src, string data_src)
{
    // Asymmetric execution, meaning that only the evaluator gets the output
    // Write circuit and data to a temporary file
    extern FILE* yyin;
    yyin = std::tmpfile();
    fputs(circ_src.c_str(), yyin);
    fputs(data_src.c_str(), yyin);
    rewind(yyin);
    
    if (m_id == 0)
    {
        set_garbler(&m_garbler);
        yyparse();
        m_garbler.check_ids();
        m_garbler.m_gc.garble();
        m_garbler.init_connection();
        m_garbler.send_egtt();
        m_garbler.send_self_lbls();
        m_garbler.send_peer_lbls();
        m_garbler.send_output_map();
        m_garbler.recv_output();
    } else
    {
        set_evaluator(&m_evaluator);
        m_evaluator.check_ids();
        m_evaluator.build_garble_circuit();
        m_evaluator.init_connection();
        m_evaluator.recv_egtt();
        m_evaluator.recv_peer_lbls();
        m_evaluator.recv_self_lbls();
        m_evaluator.recv_output_map();
        m_evaluator.m_gc.evaluate();
        m_evaluator.recover_output();
        m_evaluator.send_output();
    }
    parse_clean();
}


// Use calls
void gash_init_as_garbler(string peer_ip) {
    m_id = 0;
    m_garbler = Garbler();
    m_garbler.m_peer_ip = peer_ip;
}
void gash_init_as_evaluator(string peer_ip) {
    m_id = 1;
    m_evaluator = Evaluator(peer_ip);
}

void gash_connect_peer() {
    if (m_id == 0) {
        m_garbler.init_connection();
    } else {
        m_evaluator.init_connection();
    }
}

void gash_ss_garbler_init(string client_ip) {
    tcp_server_init(GASH_SS_PORT, m_ss_listen_sock, m_ss_peer_sock);
    tcp_client_init(client_ip, GASH_SS_CLIENT_PORT, m_ss_client_sock);
}

void gash_ss_evaluator_init(string client_ip, string peer_ip) {
    tcp_client_init(peer_ip, GASH_SS_PORT, m_ss_peer_sock);
    tcp_client_init(client_ip, GASH_SS_CLIENT_PORT, m_ss_client_sock);
}

void gash_ss_client_init() {
    tcp_server_init2(GASH_SS_CLIENT_PORT, m_ss_listen_sock, m_ss_p0_sock, m_ss_p1_sock);
}

void gash_ss_recon_p2p(mpz_class share, mpz_class& ret) {
    string share_str = share.get_str(10);
    printf("Recon p2p, share: %s\n", share_str.c_str());
    if (m_id == 0)
    {
        tcp_send_mpz(m_ss_peer_sock, share);
        tcp_recv_mpz(m_ss_peer_sock, ret);
    } else {
        tcp_recv_mpz(m_ss_peer_sock, ret);
        tcp_send_mpz(m_ss_peer_sock, share);
    }
    ret += share;
    ret %= m_config_l;
}

void gash_ss_recon_slave(mpz_class& share) {
    string share_str = share.get_str(10);
    printf("Recon slave, Share: %s\n", share_str.c_str());
    tcp_send_mpz(m_ss_client_sock, share);
}

void gash_ss_recon_master(mpz_class& ret) {
    mpz_class share0;
    mpz_class share1;
    tcp_recv_mpz(m_ss_p0_sock, share0);
    tcp_recv_mpz(m_ss_p1_sock, share1);
    ret = share0 + share1;
    ret %= m_config_l;
    if (mpz_cmp(ret.get_mpz_t(), m_config_l_1.get_mpz_t()) > 0)
    {
        ret = m_config_l - ret;
    }
}

void gash_ss_send_share(mpz_class& x) {
    mpz_class share0 = gmp_prn.get_z_bits(CONFIG_L - 1);
    mpz_class share1 = (x - share0);
    
    string share0_str = share0.get_str(10);
    string share1_str = share1.get_str(10);
    printf("Share0: %s\n", share0_str.c_str());
    printf("Share1: %s\n", share1_str.c_str());
    
    tcp_send_mpz(m_ss_p0_sock, share0);
    tcp_send_mpz(m_ss_p1_sock, share1);
}

void gash_ss_recv_share(mpz_class& share) {
    tcp_recv_mpz(m_ss_client_sock, share);
    
    string share_str = share.get_str(10);
    printf("Share: %s\n", share_str.c_str());
}

void gash_ss_rescale_p2p(mpz_class& x) {
    mpz_class rescale_r;
    mpz_class v0;
    mpz_class v1;
    
    if (m_id == 0)
        v0 = x;
    else
        v1 = x;
    
    // 1) Get random mask
    rescale_r = gmp_prn.get_z_bits(CONFIG_L + CONFIG_K);
    if (m_id == 0) {
        // 2)
        v0 += rescale_r;
        tcp_send_mpz(m_ss_peer_sock, v0);
        mpz_tdiv_q_2exp(v0.get_mpz_t(), rescale_r.get_mpz_t(), CONFIG_S);
        v0 %= m_config_l_s;
        v0 = -v0;
        v0 %= m_config_l;
        x = v0;
    } else {
        // 3)
        tcp_recv_mpz(m_ss_peer_sock, v0);
        v1 += v0;
        mpz_tdiv_q_2exp(v1.get_mpz_t(), v1.get_mpz_t(), CONFIG_S);
        v1 %= m_config_l_s;
        x = v1;
    }
    
}

void gash_ss_generate_triplet()
{
    mpz_class u, v, z;
    for (int i = 0; i < TRIPLET_BATCH_SZ; ++i) {
        u = gmp_prn.get_z_bits(20);
        v = gmp_prn.get_z_bits(20);
        z = u * v;
        z %= m_config_l;
        
        m_tri_stack.push({u, v, z});
    }
}

void gash_ss_share_triplet_master()
{
    while (!m_tri_stack.empty())
    {
        triplet_t tri = m_tri_stack.top();
        gash_ss_send_share(tri.m_u);
        gash_ss_send_share(tri.m_v);
        gash_ss_send_share(tri.m_z);
        m_tri_stack.pop();
    }
}

void gash_ss_share_triplet_slave()
{
    mpz_class u, v, z;
    for (int i = 0; i < TRIPLET_BATCH_SZ; ++i) {
        gash_ss_recv_share(u);
        gash_ss_recv_share(v);
        gash_ss_recv_share(z);
        m_tri_stack.push({u, v, z});
    }
}

triplet_t gash_ss_get_next_triplet()
{
    if (m_tri_stack.empty())
    {
        FATAL("Not enough triplet");
    }
    triplet_t tri = m_tri_stack.top();
    m_tri_stack.pop();
    return tri;
}

// Circuit functions
mpz_class gash_ss_relu(mpz_class x)
{
    //////////////// No longer needed since circuit value will wrap around automatically //////////
    // 1) test if asb(x) is larger than half the ring
    // mpz_class absx;
    // mpz_abs(absx.get_mpz_t(), x.get_mpz_t());
    // if (absx > m_config_l_1 - 1)
    // {
    //     string x_str = x.get_str(10);
    //     string halfring_str = m_config_l_1.get_str();
    //     FATAL("x is larger than the ring. x:" << x_str << "\n halfring:" << halfring_str);
    // }
    /////////////////////////////////////////////////////////////////////////////////////////////
    
    // 2) convert x to decimal string
    mpz_class r;
    mpz_class ret;
    string circ_src;
    string data_src;
    string output;
    string x_str = x.get_str(10);
    string r_str;
    
    // 3) load relu circuit and evaluate it
    LOAD_CIRCUIT("ss_relu", CONFIG_L, circ_src);
    
    if (m_id == 0) {
        r = gmp_prn.get_z_bits(CONFIG_L - 1);
        r_str = r.get_str(10);
        LOAD_DATA("in0", x_str, data_src);
        LOAD_DATA("r", r_str, data_src);
    }
    else if (m_id == 1) {
        LOAD_DATA("in1", x_str, data_src);
    }
    else {
        FATAL("Invalid id, must be 0 or 1");
    }
    
    exec_asym(circ_src, data_src);
    
    // 4) build a mpz_class from the output string
    if (m_id == 0) {
        mpz_set_str(ret.get_mpz_t(), r_str.c_str(), 10);
        m_garbler.clear();
    } else {
        string out_str;
        m_evaluator.get_output(out_str);
        mpz_set_str(ret.get_mpz_t(), out_str.c_str(), 2);
        m_evaluator.clear();
    }
    
    return ret;
}

mpz_class gash_ss_relugrad(mpz_class x)
{
    
    // 2) convert x to decimal string
    mpz_class r;
    mpz_class ret;
    string circ_src;
    string data_src;
    string output;
    string x_str = x.get_str(10);
    string r_str;
    
    // 3) load relu circuit and evaluate it
    LOAD_CIRCUIT("ss_relu_grad", CONFIG_L, circ_src);
    
    if (m_id == 0) {
        r = gmp_prn.get_z_bits(CONFIG_L - 1);
        r_str = r.get_str(10);
        LOAD_DATA("in0", x_str, data_src);
        LOAD_DATA("r", r_str, data_src);
    }
    else if (m_id == 1) {
        LOAD_DATA("in1", x_str, data_src);
    }
    else {
        FATAL("Invalid id, must be 0 or 1");
    }
    
    exec_asym(circ_src, data_src);
    
    // 4) build a mpz_class from the output string
    if (m_id == 0) {
        mpz_set_str(ret.get_mpz_t(), r_str.c_str(), 10);
        m_garbler.clear();
    } else {
        string out_str;
        m_evaluator.get_output(out_str);
        mpz_set_str(ret.get_mpz_t(), out_str.c_str(), 2);
        m_evaluator.clear();
    }
    
    return ret;
}

mpz_class gash_ss_mul(mpz_class a, mpz_class b)
{
    mpz_class ei, fi, e, f, ci;
    
    // 1) Get a triplet
    triplet_t tri = gash_ss_get_next_triplet();
    
    // 2)
    ei = a - tri.m_u;
    fi = b - tri.m_v;
    
    // 3)
    gash_ss_recon_p2p(ei, e);
    gash_ss_recon_p2p(fi, f);
    
    // 4)
    if (m_id == 0)
        ci = f * a + e * b + tri.m_z;
    else
        ci = - e * f + f * a + e * b + tri.m_z;
    
    return ci;
}

int gash_ss_la(mpz_class a, mpz_class b)
{
    mpz_class ret;
    string circ_src;
    string data_src;
    string output;
    string a_str = a.get_str(10);
    string b_str = b.get_str(10);
    
    // 3) load relu circuit and evaluate it
    LOAD_CIRCUIT("ss_la", CONFIG_L, circ_src);
    
    if (m_id == 0) {
        LOAD_DATA("a0", a_str, data_src);
        LOAD_DATA("b0", b_str, data_src);
    }
    else if (m_id == 1) {
        LOAD_DATA("a1", a_str, data_src);
        LOAD_DATA("b1", b_str, data_src);
    }
    else {
        FATAL("Invalid id, must be 0 or 1");
    }
    
    exec_sym(circ_src, data_src);
    
    // 4) build a mpz_class from the output string
    string out_str;
    if (m_id == 0) {
        m_garbler.get_output(out_str);
        mpz_set_str(ret.get_mpz_t(), out_str.c_str(), 2);
        m_garbler.clear();
    } else {
        m_evaluator.get_output(out_str);
        mpz_set_str(ret.get_mpz_t(), out_str.c_str(), 2);
        m_evaluator.clear();
    }
    
    return mpz_get_si(ret.get_mpz_t());
}

mpz_class gash_ss_div(mpz_class a, mpz_class b)
{
    mpz_class r;
    mpz_class ret;
    string circ_src;
    string data_src;
    string output;
    string r_str;
    string a_str = a.get_str(10);
    string b_str = b.get_str(10);
    
    // 3) load relu circuit and evaluate it
    LOAD_CIRCUIT("ss_div", CONFIG_L, circ_src);
    
    if (m_id == 0) {
        r = gmp_prn.get_z_bits(CONFIG_L - 1);
        r_str = r.get_str(10);
        LOAD_DATA("r", r_str, data_src);
        LOAD_DATA("a0", a_str, data_src);
        LOAD_DATA("b0", b_str, data_src);
    }
    else if (m_id == 1) {
        LOAD_DATA("a1", a_str, data_src);
        LOAD_DATA("b1", b_str, data_src);
    }
    else {
        FATAL("Invalid id, must be 0 or 1");
    }
    
    exec_asym(circ_src, data_src);
    
    // 4) build a mpz_class from the output string
    string out_str;
    if (m_id == 0) {
        mpz_set_str(ret.get_mpz_t(), r_str.c_str(), 10);
        m_garbler.clear();
    } else {
        string out_str;
        m_evaluator.get_output(out_str);
        mpz_set_str(ret.get_mpz_t(), out_str.c_str(), 2);
        m_evaluator.clear();
    }
    
    return ret;
}

mpz_class gash_ss_exp(mpz_class x) {
    mpz_class r;
    mpz_class ret;
    string circ_src;
    string data_src;
    string output;
    string r_str;
    string x_str = x.get_str(10);
    
    // 3) load relu circuit and evaluate it
    LOAD_CIRCUIT("ss_exp", CONFIG_L, circ_src);
    
    if (m_id == 0) {
        r = gmp_prn.get_z_bits(CONFIG_L - 1);
        r_str = r.get_str(10);
        LOAD_DATA("r", r_str, data_src);
        LOAD_DATA("x0", x_str, data_src);
    }
    else if (m_id == 1) {
        LOAD_DATA("x1", x_str, data_src);
    }
    else {
        FATAL("Invalid id, must be 0 or 1");
    }
    
    exec_asym(circ_src, data_src);
    
    // 4) build a mpz_class from the output string
    string out_str;
    if (m_id == 0) {
        mpz_set_str(ret.get_mpz_t(), r_str.c_str(), 10);
        m_garbler.clear();
    } else {
        string out_str;
        m_evaluator.get_output(out_str);
        mpz_set_str(ret.get_mpz_t(), out_str.c_str(), 2);
        m_evaluator.clear();
    }
    
    return ret;
}
