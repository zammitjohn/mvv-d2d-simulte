// Copyright 2019, John Zammit, All rights reserved.
// This file is subject to the terms and conditions defined in
// file 'LICENSE.md', which is part of this source code package.

#include "inet/applications/base/ApplicationBase.h"
#include "inet/transportlayer/contract/udp/UDPSocket.h"
#include "videoSegmentRequest_m.h"
#include <string>

using namespace inet;

/**
 * A "Realtime" Video Stream client application.
 *
 * Basic video stream application. Clients sends a 7 Byte request to server
 * to start receiving segments via UDP packets.
 */

class UDPVideoStreamClient : public ApplicationBase{
protected:
    // state
    UDPSocket socket;
    cMessage *selfMsg = nullptr;
    std::string mode = "N/A"; // mode state used in refreshDisplay(), modified per segment request

    // parameters
    int svrPort = 0;
    int localPort = 0;
    int viewID = 0;
    long bytesLeft; // bytes left to receive
    double packetInt; // interval at which packets are received by client
    double packetLen; // size of each packet
    double packetRate; // rate at which segments are received
    cPar *clientBufferSize = nullptr; // buffer size, obtained from parent

    // statistics and counters
    static simsignal_t segmentCompletionTimeSignal;
    int segmentCount; // number of requested segments
    int packetCount; // number of received packets
    double segmentRequestTime = 0; // time in seconds, when latest segment is requested, updated for each request
    double segmentCompletionTime; // time in seconds, when latest segment is fetched entirely, updated for each segment

protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void finish() override;
    virtual void handleMessageWhenUp(cMessage *msg) override;

    virtual void requestSegment();
    virtual void receiveSegment(cPacket *msg);
    virtual void refreshDisplay() const;

    // ApplicationBase:
    virtual bool handleNodeStart(IDoneCallback *doneCallback) override;
    virtual bool handleNodeShutdown(IDoneCallback *doneCallback) override;
    virtual void handleNodeCrash() override;

public:
    UDPVideoStreamClient() {}
};
