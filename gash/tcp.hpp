//
//  tcp.hpp
//  CircuitExecutor
//
//  Created by Xiaoting Tang on 6/17/18.
//  Copyright © 2018 Xiaoting Tang. All rights reserved.
//

#ifndef tcp_hpp
#define tcp_hpp


#include <stdio.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string>
#include <gmpxx.h>

using std::string;

#ifndef SOL_TCP
#define SOL_TCP IPPROTO_TCP
#endif

void tcp_server_init(uint16_t port, int& listen_sock, int& peer_sock);
void tcp_server_init2(uint16_t port, int& listen_sock, int& p0_sock, int& p1_sock);
void tcp_client_init(string ip, uint16_t port, int& sock);
int tcp_send_bytes(int socket, char* src, uint32_t size);
int tcp_recv_bytes(int socket, char* dest, uint32_t size);
void tcp_send_mpz(int sock, mpz_class& mpz);
void tcp_recv_mpz(int sock, mpz_class& mpz);

#endif /* tcp_hpp */
