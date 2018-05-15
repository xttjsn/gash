/*
 * garbler.h -- Garbler
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

#ifndef GASH_GC_GARBLER_H
#define GASH_GC_GARBLER_H

#include "../include/common.hh"
#include "garbled_circuit.hh"

namespace gashgc {

  class Garbler {
  public:

    /// The garbled circuit instance
    GC                    m_gc;

    /// The circuit instance
    Circuit               m_c;

    /// Number of inputs
    u32                   m_n_self_in;
    u32                   m_n_peer_in;

    /// Map wire id to values (0/1)
    IdValueMap            m_in_val_map;
    IdValueMap            m_out_val_map;

    /// The set of input/output ids //TODO: remove value map if don't need them
    IdSet                 m_in_id_set;
    IdSet                 m_self_in_id_set;
    IdSet                 m_peer_in_id_set;
    IdSet                 m_out_id_set;

    /// Network related stuff
    int                   m_peer_sock;
    int                   m_peer_ot_sock;
    int                   m_listen_sock;
    string                m_self_ip;
    string                m_peer_ip;
    u8                    m_port;
    u8                    m_ot_port;

    /**
     * Read input data from input data file
     *
     * @param in_file_path
     *
     * @return
     */
    int read_input(string in_file_path);

    /**
     * Garble the circuit
     *
     * @return
     */
    int garble_circ();

    /**
     * Build connection with evaluator
     *
     * @return
     */
    int init_connection();

    /**
     * Send encrypted garbled truth table to evaluator
     *
     * @return
     */
    int send_egtt();

    /**
     * Send garbler's labels, directly.
     *
     *
     * @return
     */
    int send_self_lbls();

    /**
     * Send evaluator's labels, through Oblivious Transfer
     *
     *
     * @return
     */
    int send_peer_lbls();

    /**
     * Send output map to evaluator
     *
     *
     * @return
     */
    int send_output_map();

    /**
     * Receive output from evaluator
     *
     *
     * @return
     */
    int recv_output();

    /**
     * Constructor
     *
     * @param port The gc port that this garbler listens on
     * @param circ_file_path
     * @param input_file_path
     */
    Garbler(u8 port, u8 ot_port, string circ_file_path, string input_file_path);

    /**
     * Destructor
     *
     */
    ~Garbler();
  };

}

#endif
