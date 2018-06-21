//
//  ot.hpp
//  CircuitExecutor
//
//  Created by Xiaoting Tang on 6/17/18.
//  Copyright © 2018 Xiaoting Tang. All rights reserved.
//

#ifndef ot_hpp
#define ot_hpp

#include <stdio.h>
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
#include "util.hpp"

typedef vector<block> LabelVec;

class OTParty {
public:
    USHORT m_nPort;
    const char* m_nAddr;
    CSocket* m_vSocket;
    int m_nPID;
    field_type m_eFType;
    int m_nBitLength;
    MaskingFunction* m_fMaskFct;
    const char* m_cConstSeed[2] = { "437398417012387813714564100", "15657566154164561" };
    OTExtSnd* m_sender;
    OTExtRec* m_receiver;
    SndThread* m_sndthread;
    RcvThread* m_rcvthread;
    int m_nNumOTThreads;
    int m_nBaseOTs;
    int m_nChecks;
    bool m_bUseMinEntCorAssumption;
    ot_ext_prot m_eProt;
    double m_rndgentime;
    
    void Init()
    {
        m_vSocket = new CSocket();
    }
    
    void Cleanup()
    {
        m_vSocket->Close();
        delete m_sndthread;
        delete m_rcvthread;
        delete m_vSocket;
    }
    
    int Listen() {
        if (!m_vSocket->Socket()) {
            perror("Socket() failed.");
            abort();
        }
        
        if (!m_vSocket->Bind(m_nPort, m_nAddr)) {
            perror("Bind failed.");
            abort();
        }
        
        if (!m_vSocket->Listen()) {
            perror("Listen failed.");
            abort();
        }
        
        for (int i = 0; i < 1; i++) // twice the actual number, due to double sockets for OT
        {
            CSocket sock;
            
            if (!m_vSocket->Accept(sock)) {
                perror("Accept failed.");
                abort();
            }
            
            UINT threadID;
            sock.Receive(&threadID, sizeof(int));
            
            if (threadID >= 1) {
                
                sock.Close();
                i--;
                continue;
            }
            
            // locate the socket appropriately
            m_vSocket->Close();
            m_vSocket->AttachFrom(sock);
            sock.Detach();
        }
        
        return 0;
    }
    
    int Connect() {
        bool bFail = false;
        
        uint64_t lTO = CONNECT_TIMEO_MILISEC;
        
        for (int k = 0; k >= 0; k--) {
            for (int i = 0; i < RETRY_CONNECT; i++) {
                
                if (!m_vSocket->Socket()) {
                    
                    printf("Socket failure: ");
                    cerr << " (" << !m_nPID << ") connection failed" << endl;
                    abort();
                }
                
                if (m_vSocket->Connect(m_nAddr, m_nPort, lTO)) {
                    
                    // send pid when connected
                    m_vSocket->Send(&k, sizeof(int));
                    
                    if (k == 0) {
                        
                        cout << "connected" << endl;
                        return 1;
                        
                    } else {
                        break;
                    }
                    
                    SleepMiliSec(10);
                    m_vSocket->Close();
                }
                
                SleepMiliSec(20);
                
                if (i + 1 == RETRY_CONNECT) {
                    
                    printf("Server not available: ");
                    cerr << " (" << !m_nPID << ") connection failed" << endl;
                    abort();
                }
            }
        }
        
        return 0;
    }
    
    int InitOTSender(const char* addr, int port, crypto* crypt, CLock* glock) {
        m_nPort = (USHORT)port;
        m_nAddr = addr;
        
        Init();
        
        Listen();
        
        m_sndthread = new SndThread(m_vSocket, glock);
        m_rcvthread = new RcvThread(m_vSocket, glock);
        
        m_rcvthread->Start();
        m_sndthread->Start();
        
        switch (m_eProt) {
                
            case ALSZ:
                m_sender = new ALSZOTExtSnd(crypt, m_rcvthread, m_sndthread, m_nBaseOTs, m_nChecks);
                break;
                
            case IKNP:
                m_sender = new IKNPOTExtSnd(crypt, m_rcvthread, m_sndthread);
                break;
                
            case NNOB:
                m_sender = new NNOBOTExtSnd(crypt, m_rcvthread, m_sndthread);
                break;
                
            case KK:
                m_sender = new KKOTExtSnd(crypt, m_rcvthread, m_sndthread);
                break;
                
            default:
                m_sender = new ALSZOTExtSnd(crypt, m_rcvthread, m_sndthread, m_nBaseOTs, m_nChecks);
                break;
        }
        
        if (m_bUseMinEntCorAssumption)
            m_sender->EnableMinEntCorrRobustness();
        
        m_sender->ComputeBaseOTs(m_eFType);
        
        return 0;
    }
    
    int InitOTReceiver(const char* addr, int port, crypto* crypt, CLock* glock) {
        m_nPort = (USHORT)port;
        m_nAddr = addr;
        
        Init();
        
        Connect();
        
        m_sndthread = new SndThread(m_vSocket, glock);
        m_rcvthread = new RcvThread(m_vSocket, glock);
        
        m_rcvthread->Start();
        m_sndthread->Start();
        
        switch (m_eProt) {
                
            case ALSZ:
                m_receiver = new ALSZOTExtRec(crypt, m_rcvthread, m_sndthread, m_nBaseOTs, m_nChecks);
                break;
                
            case IKNP:
                m_receiver = new IKNPOTExtRec(crypt, m_rcvthread, m_sndthread);
                break;
                
            case NNOB:
                m_receiver = new NNOBOTExtRec(crypt, m_rcvthread, m_sndthread);
                break;
                
            case KK:
                m_receiver = new KKOTExtRec(crypt, m_rcvthread, m_sndthread);
                break;
                
            default:
                m_receiver = new ALSZOTExtRec(crypt, m_rcvthread, m_sndthread, m_nBaseOTs, m_nChecks);
                break;
        }
        
        if (m_bUseMinEntCorAssumption)
            m_receiver->EnableMinEntCorrRobustness();
        
        m_receiver->ComputeBaseOTs(m_eFType);
        return 0;
    }
    
    bool ObliviousSend(CBitVector** X, int numOTs, int bitlength, uint32_t nsndvals,
                       snd_ot_flavor stype, rec_ot_flavor rtype, crypto* crypt) {
        bool success = FALSE;
        
        m_vSocket->ResetSndCnt();
        m_vSocket->ResetRcvCnt();
        
        timespec ot_begin, ot_end;
        
        clock_gettime(CLOCK_MONOTONIC, &ot_begin);
        
        // Execute OT sender routine
        success = m_sender->send(numOTs, bitlength, nsndvals, X, stype, rtype,
                                 m_nNumOTThreads, m_fMaskFct);
        
        clock_gettime(CLOCK_MONOTONIC, &ot_end);
        
#ifndef BATCH
        
        printf("Time spent:\t%f\n", getMillies(ot_begin, ot_end) + rndgentime);
        cout << "Sent:\t\t" << m_vSocket->getSndCnt() << " bytes" << endl;
        cout << "Received:\t" << m_vSocket->getRcvCnt() << " bytes" << endl;
        
#else
        
        cout << getMillies(ot_begin, ot_end) + m_rndgentime << "\t"
        << m_vSocket->getSndCnt() << "\t" << m_vSocket->getRcvCnt() << endl;
        
#endif
        
        return success;
    }
    
    bool ObliviousReceive(CBitVector* choices, CBitVector* ret, int numOTs,
                          int bitlength, uint32_t nsndvals, snd_ot_flavor stype,
                          rec_ot_flavor rtype, crypto* crypt) {
        bool success = FALSE;
        
        m_vSocket->ResetSndCnt();
        m_vSocket->ResetRcvCnt();
        
        timespec ot_begin, ot_end;
        clock_gettime(CLOCK_MONOTONIC, &ot_begin);
        // Execute OT receiver routine
        success = m_receiver->receive(numOTs, bitlength, nsndvals, choices, ret,
                                      stype, rtype, m_nNumOTThreads, m_fMaskFct);
        
        clock_gettime(CLOCK_MONOTONIC, &ot_end);
        
#ifndef BATCH
        printf("Time spent:\t%f\n", getMillies(ot_begin, ot_end) + rndgentime);
        
        cout << "Sent:\t\t" << m_vSocket->getSndCnt() << " bytes" << endl;
        cout << "Received:\t" << m_vSocket->getRcvCnt() << " bytes" << endl;
#else
        cout << getMillies(ot_begin, ot_end) + m_rndgentime << "\t"
        << m_vSocket->getSndCnt() << "\t" << m_vSocket->getRcvCnt() << endl;
#endif
        
        return success;
    }

    int OTSend(string peer_addr, int peer_ot_port, vector<block>& label0s,
               vector<block>& label1s) {
        assert(label0s.size() == label1s.size());
        
        uint32_t m_nLabel = label0s.size();
        uint64_t numOTs = m_nLabel;
        
        uint32_t bitlength = LABELSIZE * 8;
        uint32_t nsndvals = 2;
        
        m_eFType = ECC_FIELD;
        
        uint32_t m_nSecParam = 128;
        
        m_nNumOTThreads = 1;
        
        snd_ot_flavor stype = Snd_OT;
        rec_ot_flavor rtype = Rec_OT;
        
        // The following two integers are only useful for ALSZ
        m_nBaseOTs = 190;
        m_nChecks = 380;
        
        m_bUseMinEntCorAssumption = false;
        
        m_eProt = IKNP;
        
        m_nAddr = peer_addr.c_str();
        m_nPort = peer_ot_port;
        m_nPID = 0;
        
        crypto* crypt = new crypto(m_nSecParam, (uint8_t*)m_cConstSeed[m_nPID]);
        
        CLock* glock = new CLock();
        
        InitOTSender(peer_addr.c_str(), peer_ot_port, crypt, glock);
        
        // TODO: if we don't need delta, delete it
        CBitVector delta; // The R
        
        CBitVector** X = (CBitVector**)malloc(sizeof(CBitVector*) * nsndvals);
        
        m_fMaskFct = new XORMasking(bitlength, delta);
        
        delta.Create(numOTs, bitlength);
        
        BYTE** bytes = (BYTE**)malloc(sizeof(BYTE*) * nsndvals);
        
        for (int i = 0; i < nsndvals; ++i) {
            
            X[i] = new CBitVector();
            X[i]->Create(numOTs, bitlength);
            
            int size = m_nLabel * LABELSIZE;
            bytes[i] = new BYTE[size];
            memset(bytes[i], 0, size);
            
            if (i == 0) {
                label_vec_marshal(label0s, (char*)bytes[i]);
            } else {
                label_vec_marshal(label1s, (char*)bytes[i]);
            }
            
            X[i]->SetBytes(bytes[i], 0, size);
        }
        
        cout << "Sending " << label0s.size() << " labels" << endl;
        
        ObliviousSend(X, numOTs, bitlength, nsndvals, stype, rtype, crypt);
        
        cout << "------ OT sender ------" << endl;
        cout << "Send amount: " << m_vSocket->getSndCnt() << endl;
        cout << "Recv amount: " << m_vSocket->getRcvCnt() << endl;
        
        Cleanup();
        delete crypt;
        delete glock;
        
        for (int i = 0; i < nsndvals; ++i) {
            delete[] bytes[i];
        }
        
        free(bytes);
        
        return 0;
    }
    
    int OTRecv(string peer_addr, int peer_ot_port, map<int, int>& selects,
               map<int, block>& res_idlbl_map) {
        uint64_t numOTs = selects.size();
        
        uint32_t bitlength = LABELSIZE * 8;
        uint32_t nsndvals = 2;
        
        m_eFType = ECC_FIELD;
        
        uint32_t m_nSecParam = 128;
        
        m_nNumOTThreads = 1;
        
        snd_ot_flavor stype = Snd_OT;
        rec_ot_flavor rtype = Rec_OT;
        
        // The following two integers are only useful for ALSZ
        m_nBaseOTs = 190;
        m_nChecks = 380;
        
        m_bUseMinEntCorAssumption = false;
        
        m_eProt = IKNP;
        
        m_nAddr = peer_addr.c_str();
        m_nPort = peer_ot_port;
        m_nPID = 1;
        
        crypto* crypt = new crypto(m_nSecParam, (uint8_t*)m_cConstSeed[m_nPID]);
        
        CLock* glock = new CLock();
        
        InitOTReceiver(peer_addr.c_str(), peer_ot_port, crypt, glock);
        
        CBitVector choices, response;
        
        m_fMaskFct = new XORMasking(bitlength);
        
        vector<int> selects_vec;
        
        // We are taking advantage of the fact that `selects` is ordered
        for (auto it = selects.begin(); it != selects.end(); ++it) {
            
            selects_vec.push_back(it->second);
        }
        
        // 1 bit per selection, there might be some empty bits at the last byte
        int size = (int)ceil(((float)selects_vec.size()) / 8.f);
        cout << "Receiving using " << size << " bytes" << endl;
        
        // Alloc and initialize choice_bytes
        BYTE choice_bytes[size];
        memset(choice_bytes, 0, size);
        select2bytes(selects_vec, (char*)choice_bytes);
        
        // Change size back to the number of effective bits in the byte array
        size = selects_vec.size();
        
        cout << "Receving using " << size << " selection bits" << endl;
        
        // Use the SetBits to precisely setting the bits
        choices.Create(size, crypt);
        choices.SetBits(choice_bytes, 0, size);
        
        // Pre-generate the respose vector for the results
        response.Create(numOTs, bitlength);
        
        response.Reset();
        
        ObliviousReceive(&choices, &response, numOTs, bitlength, nsndvals, stype,
                         rtype, crypt);
        
        BYTE* response_bytes = new BYTE[selects.size() * LABELSIZE];
        response.GetBytes(response_bytes, 0, selects.size() * LABELSIZE);
        
        int i = 0;
        
        for (auto it = selects.begin(); it != selects.end(); ++it, ++i) {
            
            int id = it->first;
            block label;
            
            byte2label((char*)response_bytes, LABELSIZE * i, label);
            res_idlbl_map.emplace(id, label);
        }
        
        cout << "------ OT receiver ------" << endl;
        cout << "Send amount: " << m_vSocket->getSndCnt() << endl;
        cout << "Recv amount: " << m_vSocket->getRcvCnt() << endl;
        
        Cleanup();
        delete crypt;
        delete glock;
        
        return 0;
    }
    
};

#endif /* ot_hpp */
