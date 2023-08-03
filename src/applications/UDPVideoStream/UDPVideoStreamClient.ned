// Copyright 2019, John Zammit, All rights reserved.
// This file is subject to the terms and conditions defined in
// file 'LICENSE.md', which is part of this source code package.

package applications.UDPVideoStream;
import inet.applications.contract.IUDPApp;

//
// Video streaming client.
//
// This module sends one video segment request to serverAddress:serverPort at time startTime
// and receives packets of CBR from ~UDPVideoStreamServer server.
//
// Statistics:
//
// endToEndDelay: statistic of delay between segment request and its completion
//
// @see ~UDPVideoStreamServer
//

simple UDPVideoStreamClient like IUDPApp{
    parameters:
        int localPort = default(7000);  // local port, must match UDPVideoStreamServer!
        volatile string serverAddress = default("server");  // server address
        volatile int serverPort = default(7000);  // server port, must match UDPVideoStreamServer!
        volatile double startTime @unit(s) = default(1s);
        volatile int viewID = default(1); // view being requested
        volatile double packetInt @unit(s) = default(5ms); // interval between sending video stream packets, must match UDPVideoStreamServer!
        volatile int packetLen @unit(B) = default(1000B);  // length of a video packet in bytes, must match UDPVideoStreamServer!
        volatile int segmentSize @unit(B) = default(1MB);  // length of full a video stream in bytes, must match UDPVideoStreamServer!
        
        @display("i=block/app2");
         
        @signal[segmentCompletionTime](type="double");
        @statistic[endToEndDelay](title="end-to-end delay"; source="segmentCompletionTime"; record=vector,stats; interpolationmode=none);
        
    gates:
        input udpIn @labels(UDPControlInfo/up);
        output udpOut @labels(UDPControlInfo/down);
}