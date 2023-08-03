// Copyright 2019, John Zammit, All rights reserved.
// This file is subject to the terms and conditions defined in
// file 'LICENSE.md', which is part of this source code package.

#include "ModeSelector.h"

#include "inet/networklayer/contract/ipv4/IPv4ControlInfo.h"
#include "inet/networklayer/ipv4/IPv4Datagram.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "common/lterecorder.h"
#include "inet/networklayer/ipv4/IPv4Datagram.h"

using namespace inet;
Define_Module(ModeSelector);

inline std::ostream& operator<<(std::ostream& out, const ModeSelector::requestData& d){
    out << "client=" << d.address << ":" << d.port
            << "  segment=" << d.seg_ID << "  view=" << d.view_ID << "  time=" << d.requestTime;
    return out;
}

ModeSelector::~ModeSelector(){
    bufferMap.clear(); // Clear all the elements from the map container
}

void ModeSelector::initialize(int stage){
    if (stage == INITSTAGE_LOCAL){ // Initializations that don't use or affect other modules take place
        segmentRequestID = -1;
        redirectCounter = 0;
        WATCH_MAP(bufferMap);
    }
}

void ModeSelector::handleMessage(cMessage *msg){

    if (msg->isSelfMessage()){
        DeleteBufferEntry *selfMsg = check_and_cast<DeleteBufferEntry *>(msg);
        EV << "Deleting buffer map entry with ID #" << selfMsg->getEntryID() << endl;
        bufferMap.erase(selfMsg->getEntryID()); // erasing buffer map entry by key, segmentRequestID
        delete msg;

    } else {

        // Type cast and decapsulate
        IPv4Datagram *datagram = check_and_cast<IPv4Datagram *>(msg);
        VideoSegmentRequest *request = check_and_cast<VideoSegmentRequest *>(datagram->getEncapsulatedPacket()->decapsulate());
        EV << "Received request for segment #" << request->getSegmentID() << " for view #"  << (request->getViewID())
                            << " from: "  << request->getClientAddress() << ":" << request->getClientPort() << endl;

        // Setting videoSegmentRequest flag
        request->setModeSelection(false);

        // Adding previous segment request to the map; indicating that it had been completed
        if (request->getIsInit() == false){ // Initial requests are not added to the buffer map
            EV << "Adding segment #" << (request->getSegmentID()-1) << " for view #"  << (request->getViewID())
                                << " from: "  << request->getClientAddress() << ":" << request->getClientPort()
                                << " to buffer map" << endl;
            segmentRequestID++;

            requestData *d = &bufferMap[segmentRequestID];
            d->address = request->getClientAddress();
            d->port = request->getClientPort();
            d->seg_ID = (request->getSegmentID()-1);
            d->view_ID = request->getViewID();
            d->requestTime = request->getCreationTime();

            // Schedule Self Message to delete buffer map entry
            DeleteBufferEntry *selfMsg = new DeleteBufferEntry;
            selfMsg->setEntryID(segmentRequestID);
            EV << "Scheduling to delete buffer map entry in " << (request->getCreationTime())+((request->getClientBufferSize())/1000) << " seconds" << endl;
            scheduleAt((request->getCreationTime())+((request->getClientBufferSize())/1000), selfMsg); // dividing for ms to s
        }

        // Create a map iterator and point to end of map
        std::map<int, requestData>::iterator it = bufferMap.end();

        // Iterate over the map using Iterator in descending order (most recent used)
        while (it != bufferMap.begin())
        {
            requestData *mapEntryData = &(it->second);

            // Checking if buffer map contains any entry which satisfies the following:
            if ((mapEntryData->seg_ID == request->getSegmentID()) && // A) Matching Segment ID
                    (mapEntryData->view_ID == request->getViewID())){ // B) Matching View ID

                EV << "BUFFER MAP MATCH! " << mapEntryData->address
                        << " has the requested segment #" << mapEntryData->seg_ID << " for view #"
                        << mapEntryData->view_ID << " in cache!" << endl;
                redirectCounter++;

                datagram->getEncapsulatedPacket()->encapsulate(request);
                datagram->setDestinationAddress(datagram->getSourceAddress()); // bounce segment request message
                send(datagram,"trafficFlowFilterGate$o");
                return;
            }
            it--;
        }

        // Encapsulate and send
        datagram->getEncapsulatedPacket()->encapsulate(request);
        send(datagram,"trafficFlowFilterGate$o");
    }
}

void ModeSelector::finish(){
}
