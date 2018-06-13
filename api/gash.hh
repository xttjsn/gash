/*
 * gash.hh -- API headers
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

#ifndef GASH_API_HH
#define GASH_API_HH

#include <gmpxx.h>
#include "../include/common.hh"
#include "../gc/garbler.hh"
#include "../gc/evaluator.hh"
#include "../lang/gash_lang.hh"
#include <stack>

using std::stack;


#define GASH_GC_PORT  47768
#define GASH_OT_PORT  48901
#define GASH_SS_PORT  48998
#define GASH_SS_CLIENT_PORT  49878

#define CONFIG_L 64
#define CONFIG_S 20
#define CONFIG_K 40
#define CONFIG_L_S 20

#define TRIPLET_BATCH_SZ 1000

typedef struct triplet {
    mpz_class m_u;
    mpz_class m_v;
    mpz_class m_z;
} triplet_t;


// Initialization
int gash_config_init();

// Use calls
int gash_init_as_garbler(string peer_ip);
int gash_init_as_evaluator(string peer_ip);
int gash_connect_peer();
int gash_ss_garbler_init(string client_ip);
int gash_ss_evaluator_init(string client_ip, string peer_ip);
int gash_ss_client_init();
int gash_ss_recon_p2p(mpz_class share, mpz_class& ret);
int gash_ss_recon_slave(mpz_class& share);
int gash_ss_recon_master(mpz_class& ret);
int gash_ss_send_share(mpz_class& x);
int gash_ss_recv_share(mpz_class& share);
int gash_ss_rescale_p2p(mpz_class& x);
void gash_ss_generate_triplet();
void gash_ss_share_triplet_master();
void gash_ss_share_triplet_slave();
triplet_t gash_ss_get_next_triplet();

// Circuit functions
mpz_class gash_ss_mul(mpz_class a, mpz_class b);
mpz_class gash_ss_div(mpz_class a, mpz_class b);
int gash_ss_la(mpz_class a, mpz_class b);
mpz_class gash_ss_relu(mpz_class x);
mpz_class gash_ss_relugrad(mpz_class x);
/* mpz_class gash_ss_approx_exp(mpz_class x); */

// Secdouble
class secdouble
{
public:
    mpz_class m_mpz;
    secdouble(){}
    secdouble(int i);
    secdouble(double d);
    secdouble(mpz_class mpz) : m_mpz(mpz) {}

    void scaleup();
    void scaledown();

    secdouble& operator*(double& y) {
        // Simple multiplication, y is assumed to have no scaling factor
        m_mpz *= y;
        return *this;
    }
    secdouble operator+(secdouble rhs);
    secdouble operator+=(secdouble rhs);
    secdouble operator*(secdouble rhs);  // Secure multiplication
    secdouble operator*=(secdouble rhs);
    secdouble operator/(secdouble rhs);
    secdouble operator-(secdouble rhs);
    secdouble operator-=(secdouble rhs);
    int operator>(secdouble rhs);
    secdouble& operator=(secdouble rhs);

};

#endif
