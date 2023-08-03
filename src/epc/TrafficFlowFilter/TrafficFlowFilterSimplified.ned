// Copyright 2019, John Zammit, All rights reserved.
// This file is subject to the terms and conditions defined in
// file 'LICENSE.md', which is part of this source code package.
 
package epc.TrafficFlowFilter;

simple TrafficFlowFilterSimplified{
    parameters:
        @display("i=block/filter");

        string ownerType; // must be one between ENODEB or GW
        bool fastForwarding = default(true);
    gates:
        input internetFilterGateIn;
        output gtpUserGateOut; 
        inout modeSelectorGate;
}