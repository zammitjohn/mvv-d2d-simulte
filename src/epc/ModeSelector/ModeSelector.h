// Copyright 2019, John Zammit, All rights reserved.
// This file is subject to the terms and conditions defined in
// file 'LICENSE.md', which is part of this source code package.

#include <string.h>
#include <omnetpp.h>

#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/transportlayer/contract/udp/UDPSocket.h"
#include "applications/UDPVideoStream/videoSegmentRequest_m.h"
#include "deleteBufferEntry_m.h"

using namespace inet;

class ModeSelector : public cSimpleModule{

public:
    struct requestData{
        L3Address address; // Requester's address
        int port; // Requester's port
        int seg_ID; // Segment being requested
        int view_ID; // View being requested
        simtime_t requestTime; // Time when the segment completes transmission
    };

protected:
    typedef std::map<int, requestData> VideoSegmentMap;
    VideoSegmentMap bufferMap;
    UDPSocket socket;

    // statistics and counters
    int segmentRequestID;
    int redirectCounter; // for statistical purposes (TODO) and socket binding

    virtual int numInitStages() const { return inet::NUM_INIT_STAGES; }
    void initialize(int stage);
    void handleMessage(cMessage *msg);

public:
    ~ModeSelector();
    virtual void finish();
};
