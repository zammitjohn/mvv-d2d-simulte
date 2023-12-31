// Copyright 2019, John Zammit, All rights reserved.
// This file is subject to the terms and conditions defined in
// file 'LICENSE.md', which is part of this source code package.

#include "TrafficFlowFilterSimplified.h"
#include "inet/networklayer/contract/ipv4/IPv4ControlInfo.h"
#include "inet/networklayer/ipv4/IPv4Datagram.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "applications/UDPVideoStream/videoSegmentRequest_m.h"

using namespace std;

Define_Module(TrafficFlowFilterSimplified);

void TrafficFlowFilterSimplified::initialize(int stage){
    // wait until all the IP addresses are configured
    if (stage != inet::INITSTAGE_NETWORK_LAYER)
        return;

    // get reference to the binder
    binder_ = getBinder();

    fastForwarding_ = par("fastForwarding");

    // reading and setting owner type
    ownerType_ = selectOwnerType(par("ownerType"));
}

EpcNodeType TrafficFlowFilterSimplified::selectOwnerType(const char * type){
    EV << "TrafficFlowFilterSimplified::selectOwnerType - setting owner type to " << type << endl;
    if(strcmp(type,"ENODEB") == 0)
        return ENB;
    else if(strcmp(type,"PGW") == 0)
        return PGW;

    error("TrafficFlowFilterSimplified::selectOwnerType - unknown owner type [%s]. Aborting...",type);
}

void TrafficFlowFilterSimplified::handleMessage(cMessage *msg){
    EV << "TrafficFlowFilterSimplified::handleMessage - Received Packet:" << endl;
    EV << "name: " << msg->getFullName() << endl;

    // receive and read IP datagram
    IPv4Datagram * datagram = check_and_cast<IPv4Datagram *>(msg);
    IPv4Address &destAddr = datagram->getDestAddress();
    IPv4Address &srcAddr = datagram->getSrcAddress();

    // Encapsulated packets obtained and VideoSegmentReq messages are type casted
    if (strcmp((msg->getName()), "VideoSegmentReq") == 0){
        VideoSegmentRequest  *segmentRequest = check_and_cast<VideoSegmentRequest *>(datagram->getEncapsulatedPacket()->getEncapsulatedPacket());

        // Packet sent to modeSelector only if the modeSelection flag is set
        if (segmentRequest->getModeSelection() == true ){
            send(datagram,"modeSelectorGate$o");
            return;
        }
    }

    // TODO check for source and dest port number
    EV << "TrafficFlowFilterSimplified::handleMessage - Received datagram : " << datagram->getName() << " - src[" << srcAddr << "] - dest[" << destAddr << "]\n";

    // run packet filter and associate a flowId to the connection (default bearer?)
    // search within tftTable the proper entry for this destination
    unsigned int tftId = findTrafficFlow(srcAddr, destAddr);   // search for the tftId in the binder
    if(tftId == UNSPECIFIED_TFT)
        error("TrafficFlowFilterSimplified::handleMessage - Cannot find corresponding tftId. Aborting...");

    // add control info to the normal ip datagram. This info will be read by the GTP-U application
    TftControlInfo * tftInfo = new TftControlInfo();
    tftInfo->setTft(tftId);
    datagram->setControlInfo(tftInfo);

    EV << "TrafficFlowFilterSimplified::handleMessage - setting tft=" << tftId << endl;

    // send the datagram to the GTP-U module
    send(datagram,"gtpUserGateOut");
}

TrafficFlowTemplateId TrafficFlowFilterSimplified::findTrafficFlow(L3Address srcAddress, L3Address destAddress){
    MacNodeId destId = binder_->getMacNodeId(destAddress.toIPv4());
    if (destId == 0)
        return -1;   // the destination is outside the LTE network, so send the packet to the PGW

    MacNodeId destMaster = binder_->getNextHop(destId);

    if (ownerType_ == ENB){
        MacNodeId srcMaster = binder_->getNextHop(binder_->getMacNodeId(srcAddress.toIPv4()));
        if (fastForwarding_ && srcMaster == destMaster)
            return 0;                 // local delivery
        return -1;   // send the packet to the PGW
    }
    return destMaster;
}
