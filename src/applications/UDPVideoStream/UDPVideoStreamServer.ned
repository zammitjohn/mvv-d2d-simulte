// Copyright 2019, John Zammit, All rights reserved.
// This file is subject to the terms and conditions defined in
// file 'LICENSE.md', which is part of this source code package.

package applications.UDPVideoStream;
import inet.applications.contract.IUDPApp;

//
// Video stream server. To be used with ~UDPVideoStreamClient.
//
// The server will wait for incoming video streaming requests.
// When a request arrives, it draws a random video stream size using
// the segmentSize parameter, and starts streaming to the client.
// During streaming, it will send UDP packets of size packetLen at every
// sendInterval, until segmentSize is reached. The parameters packetLen
// and sendInterval can be set to constant values to create CBR traffic,
// or to random values (e.g. sendInterval=uniform(1e-6, 1.01e-6)) to
// accomodate jitter.
//
// The server can serve several clients, and several streams per client.
//
// Statistics:
//
// reqStreamBytes: statistic of bytelength of requested video streams.
// sentPkBytes: statistic of sent packets and sent bytes.
//
// @see ~UDPVideoStreamClient
//

simple UDPVideoStreamServer like IUDPApp{
    parameters:
        int localPort = default(7000); // port to listen on
        int viewID = default(1); // view being served
        volatile double packetInt @unit(s) = default(5ms); // interval between sending video stream packets, must match UDPVideoStreamClient!
        volatile int packetLen @unit(B) = default(1000B);  // length of a video packet in bytes, must match UDPVideoStreamClient!
        volatile int segmentSize @unit(B) = default(1MB);  // length of full a video stream in bytes, must match UDPVideoStreamClient!
        
        @display("i=abstract/db");
        
        @signal[sentPk](type=cPacket);
        @signal[reqStreamBytes](type=long);
        @statistic[reqStreamBytes](title="requested stream bytes"; record=count,sum,vector; interpolationmode=none);
        @statistic[sentPk](title="packets sent"; source=sentPk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        
    gates:
        input udpIn @labels(UDPControlInfo/up);
        output udpOut @labels(UDPControlInfo/down);
}