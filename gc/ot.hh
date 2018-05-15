/*
 * ot.h -- Oblivious Transfer Header file
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

#ifndef GASH_OT_H
#define GASH_OT_H

#include "../include/common.hh"
#include "OTExtension/ENCRYPTO_utils/cbitvector.h"
#include "OTExtension/ENCRYPTO_utils/channel.h"
#include "OTExtension/ENCRYPTO_utils/crypto/crypto.h"
#include "OTExtension/ENCRYPTO_utils/parse_options.h"
#include "OTExtension/ENCRYPTO_utils/rcvthread.h"
#include "OTExtension/ENCRYPTO_utils/sndthread.h"
#include "OTExtension/ENCRYPTO_utils/socket.h"
#include "OTExtension/ENCRYPTO_utils/timer.h"
#include "OTExtension/ENCRYPTO_utils/typedefs.h"
#include "OTExtension/ot/alsz-ot-ext-rec.h"
#include "OTExtension/ot/alsz-ot-ext-snd.h"
#include "OTExtension/ot/iknp-ot-ext-rec.h"
#include "OTExtension/ot/iknp-ot-ext-snd.h"
#include "OTExtension/ot/kk-ot-ext-rec.h"
#include "OTExtension/ot/kk-ot-ext-snd.h"
#include "OTExtension/ot/nnob-ot-ext-rec.h"
#include "OTExtension/ot/nnob-ot-ext-snd.h"
#include "OTExtension/ot/xormasking.h"

namespace gashgc {

typedef vector<block> LabelVec;

BOOL Init(crypto *crypt);

BOOL Cleanup();

BOOL Connect();

BOOL Listen();

void InitOTSender(const char *address, int port, crypto *crypt);

void InitOTReceiver(const char *address, int port, crypto *crypt);

BOOL ObliviouslyReceive(CBitVector *choices, CBitVector *ret, int numOTs,
                        int bitlength, uint32_t nsndvals, snd_ot_flavor stype,
                        rec_ot_flavor rtype, crypto *crypt);

BOOL ObliviouslySend(CBitVector **X, int numOTs, int bitlength,
                     uint32_t nsndvals, snd_ot_flavor stype,
                     rec_ot_flavor rtype, crypto *crypt);

void OTSend(string peer_addr, int peer_ot_port, vector<block> &label0s,
            vector<block> &label1s);

void OTRecv(string peer_addr, int peer_ot_port, map<ulong, int> &selects,
            map<ulong, block> &inputLabel);

} // namespace gashgc

#endif
