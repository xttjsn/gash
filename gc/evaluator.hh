/*
 * evaluator.hh -- Evaluator
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
#ifndef GASH_GC_EVALUATOR_H
#define GASH_GC_EVALUATOR_H

#include "../include/common.hh"
#include "garbled_circuit.hh"

namespace gashgc {

  class Evaluator {
  public:

    /// The garbled circuit instance
    GC                    m_gc;

    /// The circuit instance
    Circuit               m_c;

    /// Number of inputs
    u32                   m_n_self_in;
    u32                   m_n_peer_in;

    /// Map wire id to values
    IdValueMap            m_in_val_map;
    IdValueMap            m_out_val_map;

    /// The set of input ids
    IdSet                 m_self_in_id_set;
    IdSet                 m_peer_in_id_set;

    /// Network related stuff
    int                   m_peer_sock;
    int                   m_peer_ot_sock;
    string                m_self_ip;      // For debug purpose
    string                m_peer_ip;
    u16                   m_port;
    u16                   m_ot_port;

    /// Input file paths
    string                m_circ_fpath;
    string                m_input_fpath;

    /**
     * Read circuit file and build a circuit
     *
     *
     * @return
     */
    int build_circ();

    /**
     * Read input data from input data file
     *
     * @param in_file_path
     *
     * @return 0 if read_input succeeds, otherwise errno is returned
     */
    int read_input();

    /**
     * Build a garbled version of the circuit
     * Of course, no label is known, the purpose of this function is only to build
     * the structure of the garbled circuit
     *
     * @return 0 if success, negative errno if failure
     */
    int build_garbled_circuit();

    /**
     * Evaluate the circuit
     *
     *
     * @return 0 if succeeds, otherwise errno is returned
     */
    int evaluate_circ();

    /**
     * Initialize connections
     *
     *
     * @return 0 if success, otherwise errno is returned
     */
    int init_connection();

    /**
     * Receive encrypted garbled truth table from garbler
     *
     *
     * @return 0 if success, otherwise errno is returned
     */
    int recv_egtt();

    /**
     * Receive the labels corresponding to the self input
     *
     *
     * @return 0 if success, otherwise errno is returned
     */
    int recv_self_lbls();

    /**
     * Receive the labels corresponding to the peer's input
     *
     *
     * @return 0 if success, otherwise errno is returned
     */
    int recv_peer_lbls();

    /**
     * Receive output map from garbler
     *
     *
     * @return 0 if success, otherwise errno is returned
     */
    int recv_output_map();

    /**
     * Map output labels to output in bits
     *
     *
     * @return 0 if success, otherwise errno is returned
     */
    int recover_output();

    /**
     * Send output in bits to the garbler
     *
     *
     * @return 0 if success, otherwise errno is returned
     */
    int send_output();

    /**
     * Print output bits to stdout
     *
     *
     * @return
     */
    int report_output();

      /**
       * Get output as binary string
       *
       * @param str
       *
       * @return
       */
      int get_output(string& str);

      /**
       * Reset everything except TCP related information
       *
       *
       * @return
       */
      int reset_circ();

      /**
       * Default constructor
       *
       */
      Evaluator() {}

      /**
     * Constructor
     *
     * @param peer_ip Ip address of garbler
     * @param port The port used for sending/recving info (other than OT)
     * @param ot_port The port used by OTExtension to conduct OT
     * @param circ_file_path Circuit file path
     * @param input_file_path Input data file path
     */
      Evaluator(string peer_ip, u16 port, u16 ot_port, string circ_file_path, string input_file_path);

      /**
     * Destructor
     *
     */
      ~Evaluator();
  };


}

#endif
