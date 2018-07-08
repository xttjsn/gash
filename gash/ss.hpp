//
//  ss.hpp
//  gash
//
//  Created by Xiaoting Tang on 6/20/18.
//  Copyright © 2018 Xiaoting Tang. All rights reserved.
//

#ifndef ss_hpp
#define ss_hpp

#include <stdio.h>
#include "common.hpp"
#include "garbler.hpp"
#include "evaluator.hpp"
#include "lang.hpp"
#include <gmpxx.h>
#include <stack>

using std::stack;

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
void gash_config_init();

// Use calls
void gash_init_as_garbler(string peer_ip);
void gash_init_as_evaluator(string peer_ip);
void gash_connect_peer();
void gash_ss_garbler_init(string client_ip);
void gash_ss_evaluator_init(string client_ip, string peer_ip);
void gash_ss_client_init();
void gash_ss_recon_p2p(mpz_class share, mpz_class& ret);
void gash_ss_recon_slave(mpz_class& share);
void gash_ss_recon_master(mpz_class& ret);
void gash_ss_send_share(mpz_class& x);
void gash_ss_recv_share(mpz_class& share);
void gash_ss_rescale_p2p(mpz_class& x);
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
mpz_class gash_ss_exp(mpz_class x);
/* mpz_class gash_ss_approx_exp(mpz_class x); */

// Secdouble
class secdouble
{
public:
    mpz_class m_mpz;
    secdouble(){}
    secdouble(int i) {
        m_mpz = i;
        scaleup();
    }
    secdouble(double d);
    secdouble(mpz_class mpz) : m_mpz(mpz) {}
    
    void scaleup();
    void scaledown() {
        gash_ss_rescale_p2p(m_mpz);
    }
    
    inline secdouble& operator*(double& y) {
        // Simple multiplication, y is assumed to have no scaling factor
        m_mpz *= y;
        return *this;
    }
    inline secdouble operator+(secdouble rhs);
    inline secdouble operator+=(secdouble rhs) {
        m_mpz += rhs.m_mpz;
        return *this;
    }
    inline secdouble operator*(secdouble rhs) {
        // Secure multiplication
        // Both *this and y are shares
        mpz_class ret = gash_ss_mul(this->m_mpz, rhs.m_mpz);
        gash_ss_rescale_p2p(ret);
        return secdouble(ret);
    }
    inline secdouble operator*=(secdouble rhs) {
        this->m_mpz = gash_ss_mul(this->m_mpz, rhs.m_mpz);
        gash_ss_rescale_p2p(this->m_mpz);
        return *this;
    }
    inline secdouble operator/(secdouble rhs) {
        secdouble ret;
        
        // Scale up again
        scaleup();
        ret.m_mpz = gash_ss_div(this->m_mpz, rhs.m_mpz);
        
        return ret;
    }
    inline secdouble operator-(secdouble rhs);
    inline secdouble operator-=(secdouble rhs);
    int operator>(secdouble rhs) {
        int ret = gash_ss_la(this->m_mpz, rhs.m_mpz);
        return ret;
    }
    inline secdouble& operator=(secdouble rhs) {
        m_mpz = rhs.m_mpz;
        return *this;
    }
};


#endif /* ss_hpp */
