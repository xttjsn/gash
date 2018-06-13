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

static stack<triplet_t> m_tri_stack;

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

static int exec_sym(string circ_src, string data_src)
{
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
        m_garbler->recv_output();

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
        m_evaluator->send_output();
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

int gash_ss_recon_p2p(mpz_class share, mpz_class& ret)
{
    string share_str = share.get_str(10);
    printf("Recon p2p, share: %s\n", share_str.c_str());
    if (m_id == 0)
    {
        REQUIRE_GOOD_STATUS(tcp_send_mpz(m_ss_peer_sock, share));
        REQUIRE_GOOD_STATUS(tcp_recv_mpz(m_ss_peer_sock, ret));
    } else {
        REQUIRE_GOOD_STATUS(tcp_recv_mpz(m_ss_peer_sock, ret));
        REQUIRE_GOOD_STATUS(tcp_send_mpz(m_ss_peer_sock, share));
    }
    ret += share;
    ret %= m_config_l;
    return 0;
}

int gash_ss_recon_slave(mpz_class& share)
{
    string share_str = share.get_str(10);
    printf("Recon slave, Share: %s\n", share_str.c_str());
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

int gash_ss_rescale_p2p(mpz_class& x)
{
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
        REQUIRE_GOOD_STATUS(tcp_send_mpz(m_ss_peer_sock, v0));
        mpz_tdiv_q_2exp(v0.get_mpz_t(), rescale_r.get_mpz_t(), CONFIG_S);
        v0 %= m_config_l_s;
        v0 = -v0;
        v0 %= m_config_l;
        x = v0;
    } else {
        // 3)
        REQUIRE_GOOD_STATUS(tcp_recv_mpz(m_ss_peer_sock, v0));
        v1 += v0;
        mpz_tdiv_q_2exp(v1.get_mpz_t(), v1.get_mpz_t(), CONFIG_S);
        v1 %= m_config_l_s;
        x = v1;
    }

    gmp_printf("x: %Zd\n", x);

    return 0;
}

// TODO: make this a macro if it works
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
        m_garbler->reset_circ();
    } else {
        string out_str;
        m_evaluator->get_output(out_str);
        mpz_set_str(ret.get_mpz_t(), out_str.c_str(), 2);
        m_evaluator->reset_circ();
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
        m_garbler->reset_circ();
    } else {
        string out_str;
        m_evaluator->get_output(out_str);
        mpz_set_str(ret.get_mpz_t(), out_str.c_str(), 2);
        m_evaluator->reset_circ();
    }

    return ret;
}

mpz_class gash_ss_mul(mpz_class a, mpz_class b)
{
    mpz_class ei, fi, e, f, ci;

    // 1) Get a triplet
    triplet_t tri = gash_ss_get_next_triplet();
    gmp_printf("tri.m_u: %Zd\n", tri.m_u);
    gmp_printf("tri.m_v; %Zd\n", tri.m_v);
    gmp_printf("tri.m_z; %Zd\n", tri.m_z);

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

    gmp_printf("ci = %Zd\n", ci);

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
        m_garbler->get_output(out_str);
        mpz_set_str(ret.get_mpz_t(), out_str.c_str(), 2);
        m_garbler->reset_circ();
    } else {
        m_evaluator->get_output(out_str);
        mpz_set_str(ret.get_mpz_t(), out_str.c_str(), 2);
        m_evaluator->reset_circ();
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
        m_garbler->reset_circ();
    } else {
        string out_str;
        m_evaluator->get_output(out_str);
        mpz_set_str(ret.get_mpz_t(), out_str.c_str(), 2);
        m_evaluator->reset_circ();
    }

    return ret;
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

void secdouble::scaleup()
{
    mpz_class one = 1;
    mpz_mul_2exp(m_mpz.get_mpz_t(), one.get_mpz_t(), CONFIG_S);
    m_mpz %= m_config_l;
}

void secdouble::scaledown()
{
    gash_ss_rescale_p2p(m_mpz);
}

secdouble::secdouble(int i) {
    m_mpz = i;
    scaleup();
}

secdouble::secdouble(double d) {
    mpf_class mpf(d);
    mpf_class one = 1;

    // Scale d to d << 20
    mpf_mul_2exp(mpf.get_mpf_t(), one.get_mpf_t(), CONFIG_S);

    // Truncate the mpf and get an integer
    mpz_set_f(m_mpz.get_mpz_t(), mpf.get_mpf_t());

    // Compute the modulo
    m_mpz %= m_config_l;
}

secdouble secdouble::operator*(secdouble rhs)
{
    // Secure multiplication
    // Both *this and y are shares
    mpz_class ret = gash_ss_mul(this->m_mpz, rhs.m_mpz);
    gash_ss_rescale_p2p(ret);
    return secdouble(ret);
}

secdouble secdouble::operator*=(secdouble rhs)
{
    this->m_mpz = gash_ss_mul(this->m_mpz, rhs.m_mpz);
    gash_ss_rescale_p2p(this->m_mpz);
    return *this;
}

secdouble secdouble::operator+(secdouble rhs)
{
    secdouble ret;
    ret.m_mpz = this->m_mpz + rhs.m_mpz;
    ret.m_mpz %= m_config_l;
    return ret;
}

secdouble secdouble::operator+=(secdouble rhs)
{
    m_mpz += rhs.m_mpz;
    return *this;
}

secdouble& secdouble::operator=(secdouble rhs)
{
    m_mpz = rhs.m_mpz;
    return *this;
}

secdouble secdouble::operator/(secdouble rhs)
{
    secdouble ret;

    // Scale up again
    scaleup();
    ret.m_mpz = gash_ss_div(this->m_mpz, rhs.m_mpz);

    return ret;
}

secdouble secdouble::operator-(secdouble rhs)
{
    secdouble ret;
    ret.m_mpz = m_mpz - rhs.m_mpz;
    ret.m_mpz %= m_config_l;
    return ret;
}

secdouble secdouble::operator-=(secdouble rhs)
{
    m_mpz -= rhs.m_mpz;
    m_mpz %= m_config_l;
    return *this;
}

int secdouble::operator>(secdouble rhs)
{
    int ret = gash_ss_la(this->m_mpz, rhs.m_mpz);
    return ret;
}
