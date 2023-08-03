// Copyright 2019, John Zammit, All rights reserved.
// This file is subject to the terms and conditions defined in
// file 'LICENSE.md', which is part of this source code package.

package corenetwork.nodes;

import inet.applications.contract.IUDPApp;
import inet.mobility.contract.IMobility;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.contract.IRoutingTable;
import inet.networklayer.contract.INetworkLayer;
import inet.transportlayer.udp.UDP;
import lte.stack.phy.ILteNic;
import lte.stack.phy.LteNicUe;

// 
// User Equipment Module
//
module Ue
{
    parameters:
        @networkNode();
        @display("i=device/pocketpc;bgb=897.162,516.83203");

        //# Mobility
        string mobilityType = default("StationaryMobility");

        //# Apps
        int numUdpApps = default(0);

        //# Node specs
        string nodeType = "UE";  // DO NOT CHANGE
        int masterId;
        int macNodeId = default(0); // TODO: this is not a real parameter
        int macCellId = default(0); // TODO: this is not a real parameter

        //# Buffer Size
        double bufferSize @unit(ms) = default(10s); // length of buffer per app in milliseconds

        //# D2D capability
        bool d2d = default(false);  // switch for D2D mode selection

        //# Network Layer specs
        string networkLayerType = default("IPv4NetworkLayer");
        string routingTableType = default("IPv4RoutingTable");
        *.interfaceTableModule = default(absPath(".interfaceTable"));
        *.routingTableModule = default(absPath(".routingTable"));

    gates:
        input radioIn @directIn;     // connection to master

    submodules:
        interfaceTable: InterfaceTable {
            @display("p=50,342.3077;is=s");
        }
        // routing table
        routingTable: <routingTableType> like IRoutingTable if routingTableType != "" {
            parameters:
                @display("p=50,413.07693;is=s");
        }
        mobility: <mobilityType> like IMobility {
            @display("p=50,473.84616;is=s");
        }
        udpApp[numUdpApps]: <> like IUDPApp {
            @display("p=249.038,90.654,row");
        }
        udp: UDP if numUdpApps>0 {
            @display("p=249.516,196.272");
        }
        nic: LteNicUe {
            nodeType = nodeType;
            d2dCapable = false;
            @display("p=250,407");
        }
        // network layer
        networkLayer: <networkLayerType> like INetworkLayer {
            parameters:
                @display("p=249.516,296.496");
        }
    connections allowunconnected:
        //# 
        //# Internal UDP applications connections with IP stack
        //#

        for i=0..numUdpApps-1 {
            udpApp[i].udpOut --> udp.appIn++;
            udpApp[i].udpIn <-- udp.appOut++;
        }

        udp.ipOut --> networkLayer.transportIn++ if numUdpApps>0;
        udp.ipIn <-- networkLayer.transportOut++ if numUdpApps>0;

        nic.radioIn <-- radioIn;

        networkLayer.ifOut++ --> nic.upperLayerIn;
        networkLayer.ifIn++ <-- nic.upperLayerOut;
}
