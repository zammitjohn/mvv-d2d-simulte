// Copyright 2019, John Zammit, All rights reserved.
// This file is subject to the terms and conditions defined in
// file 'LICENSE.md', which is part of this source code package.

#include "inet/applications/base/ApplicationBase.h"
#include "inet/transportlayer/contract/udp/UDPSocket.h"
#include "videoSegmentRequest_m.h"
#include <map>

using namespace inet;

/**
 * Stream VBR video streams to clients.
 *
 * Cooperates with UDPVideoStreamClient. UDPVideoStreamClient requests a stream
 * and UDPVideoStreamServer starts streaming to them. Capable of handling
 * streaming to multiple clients.
 */

class UDPVideoStreamServer : public ApplicationBase{
public:
    struct VideoStreamData{
        cMessage *timer = nullptr; // self timer msg
        L3Address clientAddr; // client address
        int clientPort = 0; // client UDP port
        long segmentSize = 0; // size of segment of video
        long bytesLeft = 0; // bytes left to transmit
        long numPkSent = 0; // number of packets sent
    };

protected:
    typedef std::map<long int, VideoStreamData> VideoStreamMap;

    // state
    VideoStreamMap streams;
    UDPSocket socket;

    // parameters
    int localPort = 0;
    int viewID = 0;
    cPar *packetInt = nullptr;
    cPar *packetLen = nullptr;
    cPar *segmentSize = nullptr;

    // statistics and counters
    unsigned int numStreams = 0;    // number of video streams served
    unsigned long numPkSent = 0;    // total number of packets sent
    static simsignal_t reqStreamBytesSignal;    // length of video streams served
    static simsignal_t sentPkSignal;

    virtual void processSegmentRequest(cMessage *msg);
    virtual void sendSegmentData(cMessage *timer);

    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void finish() override;
    virtual void handleMessageWhenUp(cMessage *msg) override;

    virtual void clearSegments();

    virtual bool handleNodeStart(IDoneCallback *doneCallback) override;
    virtual bool handleNodeShutdown(IDoneCallback *doneCallback) override;
    virtual void handleNodeCrash() override;

public:
    UDPVideoStreamServer() {}
    virtual ~UDPVideoStreamServer();
};
