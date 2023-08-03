// Copyright 2019, John Zammit, All rights reserved.
// This file is subject to the terms and conditions defined in
// file 'LICENSE.md', which is part of this source code package.

//
// UDPVideoStreamClient sends VideoSegmentRequest packets to UDPVideoStreamServer to fetch new video segments
//

cplusplus {{
        #include "inet/networklayer/common/L3Address.h"
}};

struct inet::L3Address;

packet VideoSegmentRequest {
        inet::L3Address clientAddress; // Requester's IP address
        int clientPort; // Requester's Port
        double clientBufferSize; // Requester's Buffer size 
        int segmentID; // Segment being requested
        int viewID; // View being requested
        bool modeSelection; // Flag to indicate if packet should go through modeSelector module
        bool isInit; // Flag to indicate if client is sending the first segment request
}