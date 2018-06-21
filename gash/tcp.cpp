//
//  tcp.cpp
//  gash
//
//  Created by Xiaoting Tang on 6/18/18.
//  Copyright © 2018 Xiaoting Tang. All rights reserved.
//

#include <stdio.h>
#include "tcp.hpp"

static uint32_t ac_sent_amt = 0;
static uint32_t ac_recv_amt = 0;

void tcp_server_init(uint16_t port, int& listen_sock, int& peer_sock) {
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
        abort();
    }
    
    result = listen(listen_sock, 5);
    if (result != 0) {
        perror("listen() failed.");
        abort();
    }
    
    // Accept
    size_t size = sizeof(address);
    peer_sock = accept(listen_sock, (struct sockaddr*)&address, (socklen_t*)&size);
    if (peer_sock < 0) {
        perror("accept() failed");
        abort();
    }
}

void tcp_server_init2(uint16_t port, int& listen_sock, int& p0_sock, int& p1_sock) {
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
        abort();
    }
    
    result = listen(listen_sock, 5);
    if (result != 0) {
        perror("listen() failed.");
        abort();
    }
    
    // Accept
    size_t size = sizeof(address);
    p0_sock = accept(listen_sock, (struct sockaddr*)&address, (socklen_t*)&size);
    if (p0_sock < 0) {
        perror("accept() failed");
        abort();
    }
    
    p1_sock = accept(listen_sock, (struct sockaddr*)&address, (socklen_t*)&size);
    if (p1_sock < 0) {
        perror("accept() failed");
        abort();
    }
}

void tcp_client_init(string ip, uint16_t port, int& sock) {
    struct addrinfo* res;
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    
    int result = getaddrinfo(ip.c_str(), NULL, NULL, &res);
    
    if (result != 0) {
        perror("Peer hostname invalid.");
        abort();
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
}

int tcp_send_bytes(int socket, char* src, uint32_t size) {
    assert(size > 0);
    
    int send_status;
    if ((send_status = send(socket, &size, sizeof(uint32_t), 0)) != sizeof(uint32_t)) {
        perror("sendBytes error: Unable to send size\n");
        abort();
    }
    
    send_status = 0;
    uint32_t sent_amt = 0;
    while (sent_amt < size) { // We could allow sending less than `aligned_size`, but this is safer
        send_status = send(socket, src + sent_amt, size - sent_amt, 0);
        if (send_status < 0) {
            perror("sendBytes error: Unable to send src\n");
            abort();
        }
        sent_amt += send_status;
    }
    
    ac_sent_amt += sent_amt;
    return 0;
}

int tcp_recv_bytes(int socket, char* dest, uint32_t size) {
    int recv_status = 0;
    
    recv(socket, &size, sizeof(uint32_t), 0);
    assert(size > 0);
    int recv_amt = 0;
    while (recv_amt < (int)size) {
        
        recv_status = recv(socket, dest + recv_amt, size - recv_amt, 0);
        if (recv_status < 0) {
            perror("sendBytes error: Unable to receive size");
            abort();
        }
        recv_amt += recv_status;
    }
    ac_recv_amt += recv_amt;
    return 0;
}

void tcp_send_mpz(int sock, mpz_class& mpz) {
    string buf = mpz.get_str(10);
    int size = buf.size();
    char cbuf[size];
    memcpy(cbuf, buf.c_str(), size + 1);
    
    tcp_send_bytes(sock, (char*)&size, sizeof(size));
    tcp_send_bytes(sock, cbuf, size);
}

void tcp_recv_mpz(int sock, mpz_class& mpz) {
    int size;
    tcp_recv_bytes(sock, (char*)&size, sizeof(size));
    
    char cbuf[size+1];
    memset(cbuf, 0, size+1);
    tcp_recv_bytes(sock, cbuf, size);
    
    mpz_set_str(mpz.get_mpz_t(), cbuf, 10);
}

void tcp_report() {
    printf("Accumulated send amount:%d\n", ac_sent_amt);
    printf("Accumulated recv amount:%d\n", ac_recv_amt);
}
