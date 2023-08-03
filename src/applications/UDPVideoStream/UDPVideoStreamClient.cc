// Copyright 2019, John Zammit, All rights reserved.
// This file is subject to the terms and conditions defined in
// file 'LICENSE.md', which is part of this source code package.

#include "UDPVideoStreamClient.h"
#include "inet/transportlayer/contract/udp/UDPControlInfo_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include <string>
#include <math.h>

using namespace inet;

Define_Module(UDPVideoStreamClient);

simsignal_t UDPVideoStreamClient::segmentCompletionTimeSignal = registerSignal("segmentCompletionTime");

void UDPVideoStreamClient::initialize(int stage){
    ApplicationBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL){
        // Initializations that don't use or affect other modules take place here
        segmentCount = 0;
        packetCount = 0;
        bytesLeft = par("segmentSize");
        packetInt = par("packetInt");
        packetLen = par("packetLen");
        svrPort = par("serverPort");
        localPort = par("localPort");
        viewID = par("viewID");

        // inherit the value from the parent module
        clientBufferSize = &(this->getAncestorPar("bufferSize"));

        svrPort = svrPort + viewID;
        localPort = localPort + viewID;

        // Calculation of rate at which segments are received
        packetRate = bytesLeft/(packetLen/packetInt);

        // Start Application
        selfMsg = new cMessage("UDPVideoStreamStart");
    }
}

void UDPVideoStreamClient::finish(){
    ApplicationBase::finish();
}

void UDPVideoStreamClient::handleMessageWhenUp(cMessage *msg){
    if (msg->isSelfMessage()){
        if (strcmp((msg->getName()), "UDPVideoStreamContinue") == 0){
            segmentCompletionTime = simTime().dbl() - segmentRequestTime;
            emit(segmentCompletionTimeSignal, segmentCompletionTime);
        }
        mode = "CDN";
        delete msg;
        requestSegment();
    } else if (strcmp((msg->getName()), "VideoSegmentPkt") == 0){
        // process incoming packet
        receiveSegment(PK(msg));
    } else if (strcmp((msg->getName()), "VideoSegmentReq") == 0){
        // Process bounced request packet
        VideoSegmentRequest *request = check_and_cast<VideoSegmentRequest *>(msg);
        EV << "Receiving segment #" << request->getSegmentID() << " from D2D SL!" << endl;
        mode = "D2D";
        delete msg;
        // Request packet is not sent right away, but instead a self message is sent.
        // Request packet is sent when it arrives back, at +packetRate (s) simulated time.
        selfMsg = new cMessage("UDPVideoStreamContinue");
        scheduleAt(simTime()+packetRate, selfMsg);
    } else if (msg->getKind() == UDP_I_ERROR){
        EV_WARN << "Ignoring UDP error report\n";
        delete msg;
    } else {
        throw cRuntimeError("Unrecognized message (%s)%s", msg->getClassName(), msg->getName());
    }
}

void UDPVideoStreamClient::requestSegment(){
    segmentCount++;

    const char *address = par("serverAddress");
    L3Address svrAddr = L3AddressResolver().resolve(address);
    if (svrAddr.isUnspecified()){
        EV_ERROR << "Server address is unspecified, skip sending video segment request\n";
        return;
    }

    segmentRequestTime = simTime().dbl();

    // Creation of new VideoSegmentRequest packet
    VideoSegmentRequest *pk = new VideoSegmentRequest("VideoSegmentReq");
    pk->setClientAddress(L3AddressResolver().resolve((this->getParentModule())->getFullName()));
    pk->setClientPort(localPort);
    pk->setClientBufferSize(clientBufferSize->doubleValue());
    pk->setModeSelection((this->getAncestorPar("d2d")).boolValue());
    pk->setIsInit(false);
    pk->setByteLength(7); //  (32 + 13 + 4 + 4 + 1 + 1 + 1) bits
    pk->setViewID(par("viewID"));

    if (segmentCount == 1){  // Initial segment request, fetch segment wrt simTime and buffer size
        pk->setIsInit(true);
        segmentCount = (ceil((simTime().dbl() * 1000)/clientBufferSize->doubleValue()));
        pk->setSegmentID(segmentCount);
    } else {
        pk->setSegmentID(segmentCount);
    }

    EV << "Requesting video segment #" << segmentCount << " from " << svrAddr << ":" << svrPort << "\n";
    socket.sendTo(pk, svrAddr, svrPort);
}

void UDPVideoStreamClient::receiveSegment(cPacket *pkt){
    bytesLeft -= pkt->getByteLength();
    packetCount++;
    EV << "Video packet: " << packetCount << "/1000 for segment #" << segmentCount << "."
            << " Packet Information: " << UDPSocket::getReceivedPacketInfo(pkt) << endl;
    delete pkt;

    if (bytesLeft == 0){
        segmentCompletionTime = simTime().dbl() - segmentRequestTime;
        emit(segmentCompletionTimeSignal, segmentCompletionTime);
        bytesLeft = par("segmentSize");
        packetCount = 0;
        requestSegment();
    }
}

bool UDPVideoStreamClient::handleNodeStart(IDoneCallback *doneCallback){
    socket.setOutputGate(gate("udpOut"));
    socket.bind(localPort);
    simtime_t startTimePar = par("startTime");
    simtime_t startTime = std::max(startTimePar, simTime());
    scheduleAt(startTime, selfMsg);
    return true;
}

bool UDPVideoStreamClient::handleNodeShutdown(IDoneCallback *doneCallback){
    cancelEvent(selfMsg);
    //TODO if(socket.isOpened()) socket.close();
    return true;
}

void UDPVideoStreamClient::handleNodeCrash(){
    cancelEvent(selfMsg);
}

void UDPVideoStreamClient::refreshDisplay() const{
    char buf[40];

    // performing conversion of mode string to a currentMode char array for sprintf()
    int n = mode.length();
    char currentMode[n+1];
    strcpy(currentMode, mode.c_str());

    sprintf(buf, "Fetch Info: \n- segment #%i \n- view #%i \n- mode: %s", segmentCount, viewID, currentMode);
    getDisplayString().setTagArg("t", 0, buf);
}
