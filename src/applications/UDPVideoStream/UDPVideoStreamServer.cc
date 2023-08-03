// Copyright 2019, John Zammit, All rights reserved.
// This file is subject to the terms and conditions defined in
// file 'LICENSE.md', which is part of this source code package.

#include "UDPVideoStreamServer.h"
#include "inet/transportlayer/contract/udp/UDPControlInfo_m.h"
#include <string.h>

using namespace inet;

Define_Module(UDPVideoStreamServer);

simsignal_t UDPVideoStreamServer::reqStreamBytesSignal = registerSignal("reqStreamBytes");
simsignal_t UDPVideoStreamServer::sentPkSignal = registerSignal("sentPk");

inline std::ostream& operator<<(std::ostream& out, const UDPVideoStreamServer::VideoStreamData& d){
    out << "client=" << d.clientAddr << ":" << d.clientPort
            << "  size=" << d.segmentSize << "  pksent=" << d.numPkSent << "  bytesleft=" << d.bytesLeft;
    return out;
}

UDPVideoStreamServer::~UDPVideoStreamServer(){
    for (auto & elem : streams)
        cancelAndDelete(elem.second.timer);
}

void UDPVideoStreamServer::initialize(int stage){
    ApplicationBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL){ // Initializations that don't use or affect other modules take place
        packetInt = &par("packetInt");
        packetLen = &par("packetLen");
        segmentSize = &par("segmentSize");
        localPort = par("localPort");
        viewID = par("viewID");

        localPort = localPort + viewID;

        // statistics
        numStreams = 0;
        numPkSent = 0;

        WATCH_MAP(streams);
    }
}

void UDPVideoStreamServer::finish(){
}

void UDPVideoStreamServer::handleMessageWhenUp(cMessage *msg){
    if (msg->isSelfMessage()){
        // timer for a particular video stream expired, send packet
        sendSegmentData(msg);
    }
    else if (strcmp((msg->getName()), "VideoSegmentReq") == 0){
        // if VideoSegmentReq is received, start streaming
        processSegmentRequest(msg);
    }
    else if (msg->getKind() == UDP_I_ERROR){
        EV_WARN << "Ignoring UDP error report\n";
        delete msg;
    }
    else {
        throw cRuntimeError("Unrecognized message (%s)%s", msg->getClassName(), msg->getName());
    }
}

void UDPVideoStreamServer::processSegmentRequest(cMessage *msg){
    // register video stream...
    VideoSegmentRequest *request = check_and_cast<VideoSegmentRequest *>(msg);

    cMessage *timer = new cMessage("VideoStreamTmr");
    VideoStreamData *d = &streams[timer->getId()];
    d->timer = timer;
    d->clientAddr = request->getClientAddress();
    d->clientPort = request->getClientPort();
    d->segmentSize = (*segmentSize);
    d->bytesLeft = d->segmentSize;
    d->numPkSent = 0;
    ASSERT(d->segmentSize > 0);
    delete msg;

    numStreams++;
    emit(reqStreamBytesSignal, d->segmentSize);

    // ... then transmit first packet right away
    sendSegmentData(timer);
}

void UDPVideoStreamServer::sendSegmentData(cMessage *timer){
    auto it = streams.find(timer->getId());
    if (it == streams.end())
        throw cRuntimeError("Model error: Stream not found for timer");

    VideoStreamData *d = &(it->second);

    // generate and send a packet
    cPacket *pkt = new cPacket("VideoSegmentPkt");
    long pktLen = packetLen->longValue();

    if (pktLen > d->bytesLeft)
        pktLen = d->bytesLeft;

    pkt->setByteLength(pktLen);
    emit(sentPkSignal, pkt);
    socket.sendTo(pkt, d->clientAddr, d->clientPort);

    d->bytesLeft -= pktLen;
    d->numPkSent++;
    numPkSent++;

    // reschedule timer only if there's bytes left to send
    if (d->bytesLeft > 0){
        simtime_t interval = (*packetInt);
        scheduleAt(simTime() + interval, timer);
    }
    else {
        streams.erase(it);
        delete timer;
    }
}

void UDPVideoStreamServer::clearSegments(){
    for (auto & elem : streams)
        cancelAndDelete(elem.second.timer);
    streams.clear();
}

bool UDPVideoStreamServer::handleNodeStart(IDoneCallback *doneCallback){
    socket.setOutputGate(gate("udpOut"));
    socket.bind(localPort);
    return true;
}

bool UDPVideoStreamServer::handleNodeShutdown(IDoneCallback *doneCallback){
    clearSegments();
    //TODO if(socket.isOpened()) socket.close();
    return true;
}

void UDPVideoStreamServer::handleNodeCrash(){
    clearSegments();
}
