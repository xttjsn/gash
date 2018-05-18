/*
 * tcp_server.cc -- Unit tests for tcp server functions
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
int listen_sock = 0;
int peer_sock = 0;

TEST_F(TCPTest, Init)
{
  EXPECT_EQ(0, gashgc::tcp_server_init(s_port, listen_sock, peer_sock));
  printf("client_sock:%d\n", peer_sock);
  close(listen_sock);
  close(peer_sock);
}


TEST_F(TCPTest, RecvSend) {

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

int main(int argc, char* argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
