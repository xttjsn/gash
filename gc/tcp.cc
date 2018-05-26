/*
 * tcp.cc -- Implementation of tcp
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

#include "tcp.hh"
#include <unistd.h>

namespace gashgc {

    /**
   * Server init
   *
   */
    int tcp_server_init(u16 port, int& listen_sock, int& peer_sock)
    {
        int result;
        int opt_val = 1;
        struct sockaddr_in address;

        listen_sock = socket(AF_INET, SOCK_STREAM, 0);

        memset(&address, 0, sizeof(address));

        address.sin_family = AF_INET;
        address.sin_port = htons(port);
        address.sin_addr.s_addr = INADDR_ANY;

        setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val));
        setsockopt(listen_sock, SOL_TCP, TCP_NODELAY, &opt_val, sizeof(opt_val));

        result = bind(listen_sock, (struct sockaddr*)&address, sizeof(address));
        if (result != 0) {
            perror("bind() failed.");
            WARNING("bind() failed.");
            return -G_ETCP;
        }

        result = listen(listen_sock, 5);
        if (result != 0) {
            perror("listen() failed.");
            WARNING("listen() fail.");
            return -G_ETCP;
        }

        // Accept
        size_t size = sizeof(address);
        peer_sock = accept(listen_sock, (struct sockaddr*)&address, (socklen_t*)&size);
        if (peer_sock < 0) {
            perror("accept() failed");
            WARNING("accept() failed");
            return -G_ETCP;
        }

        return 0;
    }

    /**
   * Client init
   *
   */
    int tcp_client_init(string ip, u16 port, int& sock)
    {
        struct addrinfo* res;
        struct sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_port = htons(port);

        int result = getaddrinfo(ip.c_str(), NULL, NULL, &res);

        if (result != 0) {

            WARNING("Peer hostname invalid.");
        }

        freeaddrinfo(res);

        inet_pton(AF_INET, ip.c_str(), &(address.sin_addr));

        sock = socket(AF_INET, SOCK_STREAM, 0);

        int one = 1;
        setsockopt(sock, SOL_TCP, TCP_NODELAY, &one, sizeof(one));

        int nretry = 100;
        float period = 0.3;

        while (nretry > 0 && (result = ::connect(sock, (struct sockaddr*)&address, sizeof(address))) != 0) {
            perror("Connect() failed, try again later");
            nretry--;
            sleep(period);
        }

        if (result < 0) {
            perror("Connect() failed, no more tries");
            abort();
        }

        return 0;
    }

    /**
   * Send bytes
   *
   */
    int tcp_send_bytes(int socket, char* src, u32 size)
    {

        GASSERT(size > 0);

        int send_status;

        if ((send_status = send(socket, &size, sizeof(u32), 0)) != sizeof(u32)) {

            WARNING("sendBytes error: Unable to send size " << size << "\n");

            return -G_ETCP;
        }

        send_status = 0;

        u32 sent_amt = 0;

        while (sent_amt < size) { // We could allow sending less than `aligned_size`, but this is safer

            send_status = send(socket, src + sent_amt, size - sent_amt, 0);

            if (send_status < 0) {

                WARNING("sendBytes error: Unable to send src\n");

                return -G_ETCP;
            }

            sent_amt += send_status;
        }

        return 0;
    }

    /**
   * Receive bytes
   *
   */
    int tcp_recv_bytes(int socket, char* dest, u32 size)
    {

        int recv_status = 0;

        REQUIRE_GOOD_STATUS(recv(socket, &size, sizeof(u32), 0));

        GASSERT(size > 0);

        int recv_amt = 0;

        while (recv_amt < (int)size) {

            recv_status = recv(socket, dest + recv_amt, size - recv_amt, 0);

            if (recv_status < 0) {

                WARNING("sendBytes error: Unable to receive size " << size << "\n");

                return -G_ETCP;
            }

            recv_amt += recv_status;
        }

        return 0;
    }

} // namespace gashgc
