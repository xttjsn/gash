/*
 * common.hh -- Common headers and miscellaneous stuff useful in testing
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

#ifndef GASH_TEST_COMMON_H
#define GASH_TEST_COMMON_H

#include <gtest/gtest.h>
#include <cstdio>
#include <fcntl.h>
#include "../../include/common.hh"
#include "../../lang/gash_lang.hh"
#include "../../gc/tcp.hh"

#define LARGE_SIZE 100000

class CMPLTest : public ::testing::Test {
protected:
  virtual void SetUp() {

    char* tmp_circ_name = tmpnam(NULL);
    char* tmp_data_name = tmpnam(NULL);

    m_circ_stream = ofstream(tmp_circ_name, std::ios::out | std::ios::trunc);
    m_data_stream = ofstream(tmp_data_name, std::ios::out | std::ios::trunc);

  }

  virtual void TearDown() {
    gashlang::parse_clean();
  }

  ofstream m_circ_stream;
  ofstream m_data_stream;
};

class TCPTest : public ::testing::Test {
protected:
    virtual void SetUp() {
    }

    virtual void TearDown() {
    }

};

#endif
