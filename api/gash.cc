/*
 * gash.cc -- API implementation
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

#include "gash.hh"
#include "../res/funcs.hh"
#include "../gc/tcp.hh"
#include <iostream>

using std::ofstream;
using std::fputs;
using std::rewind;
using gashgc::Garbler;
using gashgc::Evaluator;
using gashgc::tcp_server_init;
using gashgc::tcp_server_init2;
using gashgc::tcp_client_init;
using gashgc::tcp_send_mpz;
using gashgc::tcp_recv_mpz;
using gashlang::set_ofstream;

static mpz_class m_config_l;
static mpz_class m_config_l_1;
static mpz_class m_config_s;
static mpz_class m_config_k;
static mpz_class m_config_l_s;

static Garbler* m_garbler;
static Evaluator* m_evaluator;
static int m_ss_listen_sock;
static int m_ss_peer_sock;
static int m_ss_client_sock;

static int m_ss_p0_sock;
static int m_ss_p1_sock;
static char m_cname[128];
static char m_dname[128];
static int m_id = -1;

static ofstream m_circ_stream;
static ofstream m_data_stream;

static gmp_randclass gmp_prn(gmp_randinit_default);

int gash_config_init()
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
    return 0;
}

static int set_random_file()
{
    std::tmpnam(m_cname);
    std::tmpnam(m_dname);
    m_circ_stream = ofstream(m_cname, std::ios::out | std::ios::trunc);
    m_data_stream = ofstream(m_dname, std::ios::out | std::ios::trunc);
    set_ofstream(m_circ_stream, m_data_stream);
    return 0;
}

static int exec_asym(string circ_src, string data_src)
{
    // Asymmetric execution, meaning that only the evaluator gets the output
    // Write circuit and data to a temporary file
    extern FILE* yyin;
    yyin = std::tmpfile();
    if (m_id == 0) {
        yyin = fopen("./g_file", "w+");
    } else {
        yyin = fopen("./e_file", "w+");
    }
    fputs(circ_src.c_str(), yyin);
    fputs(data_src.c_str(), yyin);
    rewind(yyin);
    yyparse();

    if (m_id == 0)
    {
        m_garbler->build_circ();
        m_garbler->read_input();
        m_garbler->garble_circ();
        m_garbler->send_egtt();
        m_garbler->send_peer_lbls();
        m_garbler->send_self_lbls();
        m_garbler->send_output_map();

    } else
    {
        m_evaluator->build_circ();
        m_evaluator->read_input();
        m_evaluator->build_garbled_circuit();
        m_evaluator->recv_egtt();
        m_evaluator->recv_self_lbls();
        m_evaluator->recv_peer_lbls();
        m_evaluator->evaluate_circ();
        m_evaluator->recv_output_map();
        m_evaluator->recover_output();
    }

    return 0;
}

int gash_init_as_garbler(string peer_ip)
{
    m_id = 0;
    set_random_file();
    m_garbler = new Garbler(peer_ip, GASH_GC_PORT, GASH_OT_PORT, m_cname, m_dname);
    return 0;
}

int gash_init_as_evaluator(string peer_ip)
{
    m_id = 1;
    set_random_file();
    m_evaluator = new Evaluator(peer_ip, GASH_GC_PORT, GASH_OT_PORT, m_cname, m_dname);
    return 0;
}

int gash_connect_peer()
{
    if (m_id == 0) {
        m_garbler->init_connection();
    } else {
        m_evaluator->init_connection();
    }

    return 0;
}

int gash_ss_garbler_init(string client_ip)
{
    REQUIRE_GOOD_STATUS(tcp_server_init(GASH_SS_PORT, m_ss_listen_sock, m_ss_peer_sock));
    return tcp_client_init(client_ip, GASH_SS_CLIENT_PORT, m_ss_client_sock);
}

int gash_ss_evaluator_init(string client_ip, string peer_ip)
{
    REQUIRE_GOOD_STATUS(tcp_client_init(peer_ip, GASH_SS_PORT, m_ss_peer_sock));
    return tcp_client_init(client_ip, GASH_SS_CLIENT_PORT, m_ss_client_sock);
}

int gash_ss_client_init()
{
    return tcp_server_init2(GASH_SS_CLIENT_PORT, m_ss_listen_sock, m_ss_p0_sock, m_ss_p1_sock);
}

int gash_ss_recon_slave(mpz_class& share)
{
    string share_str = share.get_str(10);
    printf("Share: %s\n", share_str.c_str());
    return tcp_send_mpz(m_ss_client_sock, share);
}

int gash_ss_recon_master(mpz_class& ret)
{
    mpz_class share0;
    mpz_class share1;
    REQUIRE_GOOD_STATUS(tcp_recv_mpz(m_ss_p0_sock, share0));
    REQUIRE_GOOD_STATUS(tcp_recv_mpz(m_ss_p1_sock, share1));
    ret = share0 + share1;
    ret %= m_config_l;
    if (mpz_cmp(ret.get_mpz_t(), m_config_l_1.get_mpz_t()) > 0)
    {
        ret = m_config_l - ret;
    }
    return 0;
}

int gash_ss_send_share(mpz_class& x)
{
    mpz_class share0 = gmp_prn.get_z_bits(CONFIG_L - 1);
    mpz_class share1 = (x - share0);

    string share0_str = share0.get_str(10);
    string share1_str = share1.get_str(10);
    printf("Share0: %s\n", share0_str.c_str());
    printf("Share1: %s\n", share1_str.c_str());

    REQUIRE_GOOD_STATUS(tcp_send_mpz(m_ss_p0_sock, share0));
    REQUIRE_GOOD_STATUS(tcp_send_mpz(m_ss_p1_sock, share1));
    return 0;
}

int gash_ss_recv_share(mpz_class& share)
{

    REQUIRE_GOOD_STATUS(tcp_recv_mpz(m_ss_client_sock, share));

    string share_str = share.get_str(10);
    printf("Share: %s\n", share_str.c_str());
    return 0;
}

// TODO: make this a macro if it works
mpz_class gash_ss_relu(mpz_class x, int ringsize)
{
    // 1) test if asb(x) is larger than half the ring
    mpz_class absx;
    mpz_abs(absx.get_mpz_t(), x.get_mpz_t());
    if (absx > m_config_l_1 - 1)
    {
        string x_str = x.get_str(10);
        string halfring_str = m_config_l_1.get_str();
        FATAL("x is larger than the ring. x:" << x_str << "\n halfring:" << halfring_str);
    }

    // 2) convert x to decimal string
    mpz_class r;
    mpz_class ret;
    string circ_src;
    string data_src;
    string output;
    string x_str = x.get_str(10);
    string r_str;

    // 3) load relu circuit and evaluate it
    LOAD_CIRCUIT("ss_relu", ringsize, circ_src);

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
        m_garbler->reset_circ();
    } else {
        string out_str;
        m_evaluator->get_output(out_str);
        mpz_set_str(ret.get_mpz_t(), out_str.c_str(), 2);
        m_evaluator->reset_circ();
    }

    return ret;
}

mpz_class gash_ss_relugrad(mpz_class x, int ringsize)
{

    // 1) test if x is larger than half ring
    mpz_class absx;
    mpz_abs(absx.get_mpz_t(), x.get_mpz_t());
    if (absx > m_config_l_1 - 1)
    {
        string x_str = x.get_str(10);
        FATAL("x is larger than the ring. x: " << x_str);
    }

    // 2) convert x to decimal string
    mpz_class r;
    mpz_class ret;
    string circ_src;
    string data_src;
    string output;
    string x_str = x.get_str(10);
    string r_str;

    // 3) load relu circuit and evaluate it
    LOAD_CIRCUIT("ss_relu_grad", ringsize, circ_src);

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
        m_garbler->reset_circ();
    } else {
        string out_str;
        m_evaluator->get_output(out_str);
        mpz_set_str(ret.get_mpz_t(), out_str.c_str(), 2);
        m_evaluator->reset_circ();
    }

    return ret;
}
