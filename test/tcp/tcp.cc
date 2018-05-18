/*
 * tcp_client.cc -- Unit tests for tcp client functions
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

#include "../include/common.hh"

gashgc::u16 s_port = 13476;
int peer_sock = 0;
int listen_sock = 0;
string s_ip ="127.0.0.1";

TEST_F(TCPTest, SendRecv) {

  if (fork() == 0) {
    // Child plays client
    sleep(1);

    EXPECT_EQ(0, gashgc::tcp_client_init(s_ip, s_port, peer_sock));

    /**
     * 1 byte
     *
     */
    {
      // Test sending 1 byte
      char buf[1];
      buf[0] = 'A';
      EXPECT_EQ(0, gashgc::tcp_send_bytes(peer_sock, buf, 1));

      // Test receiving 1 byte
      buf[0] = 0;
      EXPECT_EQ(0, gashgc::tcp_recv_bytes(peer_sock, buf, 1));
      EXPECT_EQ('A', buf[0]);
    }

    /**
     * LARGE_SIZE bytes
     *
     */
    {
      char buf1[LARGE_SIZE];
      char buf2[LARGE_SIZE];
      int fd = open("char.txt", O_RDONLY);
      int read_status = read(fd, buf1, LARGE_SIZE);
      if (read_status < 0) {
        perror("");
        exit(-1);
      }
      EXPECT_EQ(0, gashgc::tcp_recv_bytes(peer_sock, buf2, LARGE_SIZE));
      EXPECT_EQ(0, memcmp(buf1, buf2, LARGE_SIZE));
    }


    close(peer_sock);

  } else {
    // Parent plays server

    EXPECT_EQ(0, gashgc::tcp_server_init(s_port, listen_sock, peer_sock));

    /**
     * 1 byte
     *
     */
    {
      // Test receiving 1 byte
      char buf[1];
      EXPECT_EQ(0, gashgc::tcp_recv_bytes(peer_sock, buf, 1));
      EXPECT_EQ('A', buf[0]);

      // Test sending 1 byte
      EXPECT_EQ(0, gashgc::tcp_send_bytes(peer_sock, buf, 1));
    }


    /**
     * LARGE_SIZE bytes
     *
     */
    {
      char buf[LARGE_SIZE];
      int fd = open("char.txt", O_RDONLY);
      int read_status = read(fd, buf, LARGE_SIZE);
      if (read_status < 0) {
        perror("");
        exit(-1);
      }
      EXPECT_EQ(0, gashgc::tcp_send_bytes(peer_sock, buf, LARGE_SIZE));
    }


    close(listen_sock);
    close(peer_sock);

  }
}

int main(int argc, char* argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
