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

#define GASH_GC_PORT  47768
#define GASH_OT_PORT  48901
#define GASH_SS_PORT  48998
#define GASH_SS_CLIENT_PORT  49878

#define CONFIG_L 64
#define CONFIG_S 20
#define CONFIG_K 40
#define CONFIG_L_S 20

// Initialization
int gash_config_init();

// Use calls
int gash_init_as_garbler(string peer_ip);
int gash_init_as_evaluator(string peer_ip);
int gash_connect_peer();
int gash_ss_garbler_init(string client_ip);
int gash_ss_evaluator_init(string client_ip, string peer_ip);
int gash_ss_client_init();
int gash_ss_recon_slave(mpz_class& share);
int gash_ss_recon_master(mpz_class& ret);
int gash_ss_send_share(mpz_class& x);
int gash_ss_recv_share(mpz_class& share);

// Circuit functions
mpz_class gash_ss_relu(mpz_class x, int ringsize);
mpz_class gash_ss_relugrad(mpz_class x, int ringsize);

#endif
