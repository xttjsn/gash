/*
 * ot.cc -- Imlementation file for OT
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

#include "ot.hh"

namespace gashgc {

  /**
   * Network Communication
   *
   */
  static USHORT m_nPort;

  /// "localhost";
  const char *m_nAddr;

  static CSocket *m_vSocket;

  /// thread id
  static u32 m_nPID;

  static u32 runs;

  static field_type m_eFType;

  static uint32_t m_nBitLength;

  static MaskingFunction *m_fMaskFct;

  static const char* m_cConstSeed[2] = {"437398417012387813714564100", "15657566154164561"};

  // Naor-Pinkas OT

  /**
   * Naor-Pinkas OT
   *
   */
  static OTExtSnd *sender;

  static OTExtRec *receiver;

  static SndThread *sndthread;

  static RcvThread *rcvthread;

  static u32 m_nNumOTThreads;

  static u32 m_nBaseOTs;

  static u32 m_nChecks;

  static bool m_bUseMinEntCorAssumption;

  static ot_ext_prot m_eProt;

  static double rndgentime;

  /**
   * Function definitions
   *
   */
  bool Init(crypto *crypt) {
    m_vSocket = new CSocket();

    return true;
  }

  bool Cleanup() {
    delete sndthread;
    delete rcvthread;
    delete m_vSocket;
    return true;
  }

  /**
   * Client starts connecting
   *
   *
   * @return
   */
  bool Connect() {

    bool bFail = false;

    uint64_t lTO = CONNECT_TIMEO_MILISEC;

#ifndef BATCH

    cout << "Connecting to party " << !m_nPID << ": " << m_nAddr << ", "
         << m_nPort << endl;

#endif

    for (int k = 0; k >= 0; k--) {
      for (int i = 0; i < RETRY_CONNECT; i++) {

        if (!m_vSocket->Socket()) {

          printf("Socket failure: ");
          goto connect_failure;

        }

        if (m_vSocket->Connect(m_nAddr, m_nPort, lTO)) {

          // send pid when connected
          m_vSocket->Send(&k, sizeof(int));

#ifndef BATCH
          cout << " (" << !m_nPID << ") (" << k << ") connected" << endl;
#endif

          if (k == 0) {

            cout << "connected" << endl;
            return TRUE;

          } else {
            break;
          }

          SleepMiliSec(10);
          m_vSocket->Close();

        }

        SleepMiliSec(20);

        if (i + 1 == RETRY_CONNECT)
          goto server_not_available;
      }
    }
  server_not_available:

    printf("Server not available: ");

  connect_failure:

    cout << " (" << !m_nPID << ") connection failed" << endl;

    return FALSE;
  }

  /**
   * Server start listening
   *
   *
   * @return
   */
  bool Listen() {

#ifndef BATCH
    cout << "Listening: " << m_nAddr << ":" << m_nPort
         << ", with size: " << m_nNumOTThreads << endl;
#endif

    if (!m_vSocket->Socket()) {

      goto listen_failure;

    }

    if (!m_vSocket->Bind(m_nPort, m_nAddr))

      goto listen_failure;

    if (!m_vSocket->Listen())

      goto listen_failure;

    for (int i = 0; i < 1; i++) // twice the actual number, due to double sockets for OT
      {
        CSocket sock;

        // cout << "New round! " << endl;

        if (!m_vSocket->Accept(sock)) {

          cerr << "Error in accept" << endl;
          goto listen_failure;

        }

        UINT threadID;
        sock.Receive(&threadID, sizeof(int));

        if (threadID >= 1) {

          sock.Close();
          i--;
          continue;

        }

#ifndef BATCH
        cout << " (" << m_nPID << ") (" << threadID << ") connection accepted"
             << endl;
#endif
        // locate the socket appropriately
        m_vSocket->AttachFrom(sock);
        sock.Detach();
      }

#ifndef BATCH
    cout << "Listening finished" << endl;
#endif
    return TRUE;

  listen_failure:

    cout << "Listen failed" << endl;
    return FALSE;
  }

  /**
   * Initialize the OT sender
   *
   * @param address
   * @param port
   * @param crypt
   * @param glock
   */
  void InitOTSender(const char* address, int port, crypto* crypt, CLock *glock)
  {

#ifdef OTTiming

    timespec np_begin, np_end;

#endif

    m_nPort = (USHORT) port;
    m_nAddr = address;

    //Initialize values
    Init(crypt);

    //Server listen
    Listen();

    sndthread = new SndThread(m_vSocket, glock);
    rcvthread = new RcvThread(m_vSocket, glock);

    rcvthread->Start();
    sndthread->Start();

    switch (m_eProt) {

    case ALSZ:
      sender =
          new ALSZOTExtSnd(crypt, rcvthread, sndthread, m_nBaseOTs, m_nChecks);
      break;

    case IKNP:
      sender = new IKNPOTExtSnd(crypt, rcvthread, sndthread);
      break;

    case NNOB:
      sender = new NNOBOTExtSnd(crypt, rcvthread, sndthread);
      break;

    case KK:
      sender = new KKOTExtSnd(crypt, rcvthread, sndthread);
      break;

    default:
      sender =
          new ALSZOTExtSnd(crypt, rcvthread, sndthread, m_nBaseOTs, m_nChecks);
      break;
    }

    if(m_bUseMinEntCorAssumption)
      sender->EnableMinEntCorrRobustness();

    sender->ComputeBaseOTs(m_eFType);
  }

  /**
   * Initialize OT receiver
   *
   * @param address
   * @param port
   * @param crypt
   * @param glock
   */
  void InitOTReceiver(const char* address, int port, crypto* crypt, CLock *glock)
  {

    m_nPort = (USHORT) port;
    m_nAddr = address;

    //Initialize values
    Init(crypt);

    //Client connect
    Connect();

    sndthread = new SndThread(m_vSocket, glock);
    rcvthread = new RcvThread(m_vSocket, glock);

    rcvthread->Start();
    sndthread->Start();

    switch (m_eProt) {

    case ALSZ:
      receiver =
          new ALSZOTExtRec(crypt, rcvthread, sndthread, m_nBaseOTs, m_nChecks);
      break;

    case IKNP:
      receiver = new IKNPOTExtRec(crypt, rcvthread, sndthread);
      break;

    case NNOB:
      receiver = new NNOBOTExtRec(crypt, rcvthread, sndthread);
      break;

    case KK:
      receiver = new KKOTExtRec(crypt, rcvthread, sndthread);
      break;

    default:
      receiver =
          new ALSZOTExtRec(crypt, rcvthread, sndthread, m_nBaseOTs, m_nChecks);
      break;
    }

    if(m_bUseMinEntCorAssumption)
      receiver->EnableMinEntCorrRobustness();

    receiver->ComputeBaseOTs(m_eFType);
  }

  /**
   * Send BitVector X through OT
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
  bool ObliviouslySend(CBitVector **X, int numOTs, int bitlength,
                       uint32_t nsndvals, snd_ot_flavor stype,
                       rec_ot_flavor rtype, crypto *crypt) {
    bool success = FALSE;

    m_vSocket->ResetSndCnt();
    m_vSocket->ResetRcvCnt();

    timespec ot_begin, ot_end;

    clock_gettime(CLOCK_MONOTONIC, &ot_begin);

    // Execute OT sender routine
    success = sender->send(numOTs, bitlength, nsndvals, X, stype, rtype,
                           m_nNumOTThreads, m_fMaskFct);

    clock_gettime(CLOCK_MONOTONIC, &ot_end);

#ifndef BATCH

    printf("Time spent:\t%f\n", getMillies(ot_begin, ot_end) + rndgentime);
    cout << "Sent:\t\t" << m_vSocket->getSndCnt() << " bytes" << endl;
    cout << "Received:\t" << m_vSocket->getRcvCnt() <<" bytes" << endl;

#else

    cout << getMillies(ot_begin, ot_end) + rndgentime << "\t"
         << m_vSocket->getSndCnt() << "\t" << m_vSocket->getRcvCnt() << endl;

#endif

    return success;

  }

  /**
   * Send choices through OT
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
  bool ObliviouslyReceive(CBitVector *choices, CBitVector *ret, int numOTs,
                          int bitlength, uint32_t nsndvals, snd_ot_flavor stype,
                          rec_ot_flavor rtype, crypto *crypt) {
    bool success = FALSE;

    m_vSocket->ResetSndCnt();
    m_vSocket->ResetRcvCnt();


    timespec ot_begin, ot_end;
    clock_gettime(CLOCK_MONOTONIC, &ot_begin);
    // Execute OT receiver routine
    success = receiver->receive(numOTs, bitlength, nsndvals, choices, ret,
                                stype, rtype, m_nNumOTThreads, m_fMaskFct);

    clock_gettime(CLOCK_MONOTONIC, &ot_end);

#ifndef BATCH
    printf("Time spent:\t%f\n", getMillies(ot_begin, ot_end) + rndgentime);

    cout << "Sent:\t\t" << m_vSocket->getSndCnt() << " bytes" << endl;
    cout << "Received:\t" << m_vSocket->getRcvCnt() <<" bytes" << endl;
#else
    cout << getMillies(ot_begin, ot_end) + rndgentime << "\t"
         << m_vSocket->getSndCnt() << "\t" << m_vSocket->getRcvCnt() << endl;
#endif

    return success;
  }

  /**
   * Send labels through OT
   *
   * @param peer_addr
   * @param peer_ot_port
   * @param label0s
   * @param label1s
   */
  int OTSend(string peer_addr, int peer_ot_port, vector<block> &label0s,
              vector<block> &label1s) {

    GASSERT(label0s.size() == label1s.size());

#ifdef GASH_OT_DEBUG

    cout << "GASH_OT_DEBUG is set." << endl;

#endif

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

#ifdef GASH_OT_DEBUG
    cout << "Parameters for sender:\n"
              << "numOTs:" << numOTs << "\n"
              << "bitlength:" << bitlength << "\n"
              << "nsndvals:" << nsndvals << "\n"
              << "m_eFType:" << m_eFType << "\n"
              << "m_nSecParam:" << m_nSecParam << "\n"
              << "m_nNumOTThreads:" << m_nNumOTThreads << "\n"
              << "m_nBaseOTs:" << m_nBaseOTs << "\n"
              << "m_nChecks:" << m_nChecks << "\n"
              << "m_nPID:" << m_nPID << "\n"
              << "m_nAddr:" << m_nAddr << "\n"
              << "m_nPort:" << m_nPort << "\n";
#endif

    crypto *crypt = new crypto(m_nSecParam, (uint8_t *)m_cConstSeed[m_nPID]);

    CLock *glock = new CLock();

    InitOTSender(peer_addr.c_str(), peer_ot_port, crypt, glock);

    CBitVector delta; // The R

    CBitVector **X = (CBitVector **)malloc(sizeof(CBitVector *) * nsndvals);

    m_fMaskFct = new XORMasking(bitlength, delta);

    // creates delta as an array with "numOTs" entries of "bitlength" bit-values
    // and fills delta with random values
    delta.Create(numOTs, bitlength);

    BYTE **bytes = (BYTE **)malloc(sizeof(BYTE *) * nsndvals);

    for (int i = 0; i < nsndvals; ++i) {

      X[i] = new CBitVector();
      X[i]->Create(numOTs, bitlength);

      int size = m_nLabel * LABELSIZE;
      bytes[i] = new BYTE[size];
      memset(bytes[i], 0, size);

      if (i == 0) {
        REQUIRE_GOOD_STATUS(label_vec_marshal(label0s, (char*)bytes[i]));
      }
      else {
        REQUIRE_GOOD_STATUS(label_vec_marshal(label1s, (char*)bytes[i]));
      }

      X[i]->SetBytes(bytes[i], 0, size);
    }

    ObliviouslySend(X, numOTs, bitlength, nsndvals, stype, rtype, crypt);

    Cleanup();
    delete crypt;
    delete glock;

    for (int i = 0; i < nsndvals; ++i) {
      delete[] bytes[i];
    }

    free(bytes);

    return 0;
  }

  /**
   * Receive labels through OT
   *
   * @param peer_addr
   * @param peer_ot_port
   * @param ulong
   * @param selects
   * @param ulong
   * @param inputLabel
   *
   * @return 0 if success, negative errno if failure
   */
  int OTRecv(string peer_addr, int peer_ot_port, map<ulong, int> &selects,
              map<ulong, block> &inputLabel) {

#ifdef GASH_OT_DEBUG

    cout << "GASH_OT_DEBUG is set." << endl;

#endif

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

    cout << "Parameters for recver:\n"
              << "numOTs:" << numOTs << "\n"
              << "bitlength:" << bitlength << "\n"
              << "nsndvals:" << nsndvals << "\n"
              << "m_eFType:" << m_eFType << "\n"
              << "m_nSecParam:" << m_nSecParam << "\n"
              << "m_nNumOTThreads:" << m_nNumOTThreads << "\n"
              << "m_nBaseOTs:" << m_nBaseOTs << "\n"
              << "m_nChecks:" << m_nChecks << "\n"
              << "m_nPID:" << m_nPID << "\n"
              << "m_nAddr:" << m_nAddr << "\n"
              << "m_nPort:" << m_nPort << "\n";

    crypto *crypt = new crypto(m_nSecParam, (uint8_t *)m_cConstSeed[m_nPID]);

    CLock *glock = new CLock();

    InitOTReceiver(peer_addr.c_str(), peer_ot_port, crypt, glock);

    CBitVector choices, response;

    m_fMaskFct = new XORMasking(bitlength);

    // Fill choices with values in selects
    choices.Create(numOTs * ceil_log2(nsndvals), crypt);

    vector<int> selects_vec;

    // We are taking advantage of the fact that `selects` is ordered
    for (auto it = selects.begin(); it != selects.end(); ++it) {

#ifdef GASH_DEBUG

      cout << "Pushing back the bit for wire: " << it->first
                << "bit: " << it->second;

#endif

      selects_vec.push_back(it->second);
    }

    // 1 bit per selection, there might be some empty bits at the last byte
    u32 size = (u32)ceil((float)selects_vec.size() / 8.f);

    // Alloc and initialize choice_bytes
    BYTE choice_bytes[size];
    memset(choice_bytes, 0, size);
    REQUIRE_GOOD_STATUS(select2bytes(selects_vec, (char*)choice_bytes));

    choices.SetBytes(choice_bytes, 0, size);

    // Pre-generate the respose vector for the results
    response.Create(numOTs, bitlength);

    response.Reset();

    ObliviouslyReceive(&choices, &response, numOTs, bitlength, nsndvals, stype,
                       rtype, crypt);

    BYTE *response_bytes = new BYTE[selects.size() * LABELSIZE];
    response.GetBytes(response_bytes, 0, selects.size() * LABELSIZE);

    int i = 0;

    for (auto it = selects.begin(); it != selects.end(); ++it, ++i) {

      u32 id = it->first;
      block label;

      REQUIRE_GOOD_STATUS(byte2label((char*)response_bytes, LABELSIZE * i, label));
      inputLabel.emplace(id, label);

#ifdef GASH_DEBUG

      cout << "OT Receiving label for wire: " << id << endl
                << "Label: " << block2hex(label) << endl;

#endif

    }

    Cleanup();
    delete crypt;
    delete glock;

    return 0;
  }

} // namespace gashgc
