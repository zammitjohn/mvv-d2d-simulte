// Copyright 2019, John Zammit, All rights reserved.
// This file is subject to the terms and conditions defined in
// file 'LICENSE.md', which is part of this source code package.

//
// modeSelector sends scheduled DeleteBufferEntry to itself to delete buffer map entryID according to client buffer
//

message DeleteBufferEntry {
        double entryID;
}