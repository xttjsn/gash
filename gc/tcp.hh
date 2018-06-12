/*
 * tcp.h -- Header for TCP send and recv
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
#ifndef GASH_GC_TCP_H
#define GASH_GC_TCP_H

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>

#include "../include/common.hh"

namespace gashgc {

    /**
   * Server initialization: create socket, bind, listen, and accept
   *
   * @param port
   * @param listen_sock
   * @param peer_sock
   */
    int tcp_server_init(u16 port, int& listen_sock, int& peer_sock);

    /**
     * Init with two peers
     *
     * @param port
     * @param listen_sock
     * @param p0_sock
     * @param p1_sock
     *
     * @return
     */
    int tcp_server_init2(u16 port, int& listen_sock, int& p0_sock, int& p1_sock);

    /**
   * Client initialization: create socket and connect
   *
   * @param ip
   * @param port
   * @param sock
   */
    int tcp_client_init(string ip, u16 port, int& sock);

    /**
   * Send `size` bytes over socket
   *
   * @param socket
   * @param src
   * @param size
   *
   * @return
   */
    int tcp_send_bytes(int socket, char* src, u32 size);

    /**
   * Receive `size` bytes over socket
   *
   * @param socket
   * @param dest
   * @param size
   *
   * @return
   */
    int tcp_recv_bytes(int socket, char* dest, u32 size);

    /**
     * Send mpz
     *
     * @param sock
     * @param mpz
     *
     * @return
     */
    int tcp_send_mpz(int sock, mpz_class& mpz);

    /**
     * Recv mpz
     *
     * @param sock
     * @param mpz
     *
     * @return
     */
    int tcp_recv_mpz(int sock, mpz_class& mpz);

    /**
   * Report send and recv statistics
   *
   *
   * @return
   */
    int tcp_report();
} // namespace gashgc

#endif
