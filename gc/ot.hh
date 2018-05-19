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
#include "util.hh"

namespace gashgc {

    typedef vector<block> LabelVec;

    class OTParty {
    public:
        USHORT m_nPort;
        const char* m_nAddr;
        CSocket* m_vSocket;
        u32 m_nPID;
        field_type m_eFType;
        u32 m_nBitLength;
        MaskingFunction* m_fMaskFct;
        const char* m_cConstSeed[2] = { "437398417012387813714564100", "15657566154164561" };

        OTExtSnd* m_sender;
        OTExtRec* m_receiver;
        SndThread* m_sndthread;
        RcvThread* m_rcvthread;
        u32 m_nNumOTThreads;
        u32 m_nBaseOTs;
        u32 m_nChecks;
        bool m_bUseMinEntCorAssumption;
        ot_ext_prot m_eProt;
        double m_rndgentime;

        /**
        * Init the socket
        *
        */
        void Init()
        {
            m_vSocket = new CSocket();
        }

        /**
        * Clean up threads and socket
        *
        */
        void Cleanup()
        {
            delete m_sndthread;
            delete m_rcvthread;
            delete m_vSocket;
        }

        /**
         * Listen for receiver to connect, called by sender
         *
         *
         * @return 0 if success, negative errno if failure
         */
        int Listen();

        /**
         * Connect to sender, called by receiver
         *
         *
         * @return 0 if success, negative errno if failure
         */
        int Connect();

        /**
         * Init sender specific stuff
         *
         * @param addr
         * @param port
         * @param crypt
         * @param glock
         *
         * @return
         */
        int InitOTSender(const char* addr, int port, crypto* crypt, CLock* glock);

        /**
         * Init receiver specific stuff
         *
         * @param addr
         * @param port
         * @param crypt
         * @param glock
         *
         * @return
         */
        int InitOTReceiver(const char* addr, int port, crypto* crypt, CLock* glock);

        /**
         * Actually call the OT send routine
         *
         * @param X
         * @param numOTs
         * @param bitlength
         * @param nsndvals
         * @param stype
         * @param rtype
         * @param crypt
         *
         * @return
         */
        bool ObliviousSend(CBitVector** X, int numOTs, int bitlength, uint32_t nsndvals,
                           snd_ot_flavor stype, rec_ot_flavor rtype, crypto* crypt);

        /**
         *
         *
         * @param choices
         * @param ret
         * @param numOTs
         * @param bitlength
         * @param nsndvals
         * @param stype
         * @param rtype
         * @param crypt
         *
         * @return
         */
        bool ObliviousReceive(CBitVector* choices, CBitVector* ret, int numOTs,
                                int bitlength, uint32_t nsndvals, snd_ot_flavor stype,
                                rec_ot_flavor rtype, crypto* crypt);

        /**
         * The send interface to gashgc
         *
         * @param peer_addr
         * @param peer_ot_port
         * @param label0s
         * @param label1s
         *
         * @return
         */
        int OTSend(string peer_addr, int peer_ot_port, vector<block>& label0s,
                   vector<block>& label1s);


        /**
         * The receive interface to gashgc
         *
         * @param peer_addr
         * @param peer_ot_port
         * @param u32
         * @param selects
         * @param u32
         * @param res_idlbl_map
         *
         * @return
         */
        int OTRecv(string peer_addr, int peer_ot_port, map<u32, int>& selects,
                   map<u32, block>& res_idlbl_map);

    };

} // namespace gashgc

#endif
