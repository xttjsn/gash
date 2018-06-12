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
#include "../../gc/garbled_circuit.hh"
#include "../../gc/util.hh"
#include "../../gc/aes.hh"
#include "../../gc/ot.hh"
#include "../../gc/garbler.hh"
#include "../../gc/evaluator.hh"

#define LARGE_SIZE 100000

namespace gashgc {
  extern block AESkey;
}

using gashgc::aes_decrypt128;
using gashgc::aes_encrypt128;
using gashgc::AESkey;
using gashgc::EGTT;
using gashgc::block2hex;
using gashgc::block_eq;
using gashgc::decrypt;
using gashgc::encrypt;
using gashgc::eval_bgate;
using gashgc::get_lsb;
using gashgc::new_tweak;
using gashgc::random_block;
using gashgc::set_lsb;
using gashgc::xor_block;
using gashgc::tcp_report;
using gashgc::find_n_replace;

using gashgc::OTParty;
using gashgc::Garbler;
using gashgc::Evaluator;

typedef gashgc::GarbledCircuit GC;
typedef gashgc::GarbledGate GG;
typedef gashgc::block block;
typedef gashgc::GarbledWireInstance GWI;
typedef gashgc::WireInstance WI;

class CMPLTest : public ::testing::Test {
protected:
  virtual void SetUp() {
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

class GRBLTest : public ::testing::Test {
protected:
  virtual void SetUp() {
    m_gc.init();
  }

  virtual void TearDown() {
  }

  GC m_gc;

};

class OTTest : public ::testing::Test {
protected:
  virtual void SetUp() {

  }

  virtual void TearDown() {

  }
};

class EXECTest: public ::testing::Test {
protected:
  virtual void SetUp() {

  }

  virtual void TearDown() {
    gashlang::parse_clean();
  }

  ofstream m_circ_stream;
  ofstream m_data_stream;
};

class APITest : public ::testing::Test {
protected:
  virtual void SetUp() {

  }

  virtual void TearDown() {

  }
};

#endif
