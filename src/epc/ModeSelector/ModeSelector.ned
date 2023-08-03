// Copyright 2019, John Zammit, All rights reserved.
// This file is subject to the terms and conditions defined in
// file 'LICENSE.md', which is part of this source code package.

package epc.ModeSelector;

//
// ModeSelector.
//
// The module will intercept video segment requests traversing through the
// eNodeB. When a request arrives, it performs deep packet inspection to
// extract the requests' content. This action is only perfomed if, request
// has ModeSelection flag enabled.
// An arrival of the video segment request indicates that the previous
// segment has been received in its entirety. This is added to the buffer 
// map.
// When a packet comes to the module, the segmentID for the request will
// also be searched in the map:
// If the entry is found, the selector will bounce back the message to sender,
// thereby indictating that the segment requested is available from D2D SL.
// If the entry is not found, the selector will simply send packet back.
//

simple ModeSelector{
    parameters:
     	@display("i=abstract/switch");
    gates:
        inout trafficFlowFilterGate; 
}