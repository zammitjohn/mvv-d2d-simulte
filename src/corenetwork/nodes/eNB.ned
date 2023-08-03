// Copyright 2019, John Zammit, All rights reserved.
// This file is subject to the terms and conditions defined in
// file 'LICENSE.md', which is part of this source code package.

package corenetwork.nodes;

import inet.linklayer.ppp.PPPInterface;
import inet.mobility.static.StationaryMobility;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.ipv4.IPv4RoutingTable;
import inet.networklayer.contract.IRoutingTable;
import inet.networklayer.contract.INetworkLayer;
import inet.applications.contract.IUDPApp;
import inet.transportlayer.udp.UDP;
import lte.stack.phy.ILteNic;
import lte.stack.phy.LteNicEnb;
import lte.corenetwork.deployer.LteDeployer;
import lte.epc.gtp.GtpUserSimplified;
import lte.epc.gtp.GtpUserX2;

import epc.TrafficFlowFilter.TrafficFlowFilterSimplified;
import epc.ModeSelector.ModeSelector;

// 
// eNodeB Module
//
module eNB
{
    parameters:
        @networkNode();
        @display("i=device/antennatower;bgb=814,437");

        //# Node specs
        string nodeType = "ENODEB";        // DO NOT CHANGE
        int macNodeId = default(0); // TODO: this is not a real parameter
        int macCellId = default(0);  // TODO: this is not a real parameter
        double txPower @unit(mw) = default(100mw);

        //# Network Layer specs
        string networkLayerType = default("IPv4NetworkLayer");
        string routingTableType = default("IPv4RoutingTable");
        *.interfaceTableModule = default(absPath(".interfaceTable"));
        *.routingTableModule = default(routingTableType != "" ? absPath(".routingTable") : "");

        //# Num apps
        int numUdpApps = default(0);  // no of UDP apps. Specify the app types in INI file with udpApp[0..1].typename="UDPVideoStreamCli" syntax
        int numX2Apps = default(0);   // no of X2 apps. Specify the app types in INI file with x2App[0..1].typename="X2AppClient" syntax

    gates:
        inout ppp;                // connection to the Core Network
        input radioIn @directIn;  // connection to the radio interface
        inout x2[];               // connection to the X2 interface

    submodules:

        interfaceTable: InterfaceTable {
            @display("p=49.08,260.73752;is=s");
        }
        // TODO: should not include a routing table
        // routing table
        routingTable: <routingTableType> like IRoutingTable if routingTableType != "" {
            parameters:
                @display("p=49.08,331.29;is=s");
        }
        mobility: StationaryMobility {
            @display("p=49.08,398.775;is=s");
        }
        // NOTE: LteDeployer must be created first
        deployer: LteDeployer {
            @display("p=49.08,155.42;is=s");
        }

        //#
        //# NIC modules
        //#
        nic: LteNicEnb {
            nodeType = nodeType;
            d2dCapable = false;
            @display("p=274.03,367.0775");
        }
        pppIf: PPPInterface {
            @display("p=501,363");
        }
        //#
        //# Network layer module
        //#
        networkLayer: <networkLayerType> like INetworkLayer {
            parameters:
                @display("p=501,269;q=queue");
        }

        //#
        //# Transport layer modules and corresponding application modules
        //#
        // =============== UDP ============== //
        udpApp[numUdpApps]: <> like IUDPApp {
            @display("p=555,48,row");
        }
        udp: UDP {
            @display("p=501,148");
        }

        //# 
        //# Simplified EPC
        //# 
        trafficFlowFilter: TrafficFlowFilterSimplified {
            ownerType = nodeType;
            @display("p=386.505,47.035");
        }
        gtpUser: GtpUserSimplified {
            @display("p=386.505,147.24");
        }

        gtpUserX2: GtpUserX2 {
            @display("p=386.505,367.0775");
        }

        ModeSelector: ModeSelector {
            @display("p=274.03,47.035");
        }
    connections allowunconnected:

        //#
        //# LTE stack to PHY and network layer
        //#        
        nic.radioIn <-- radioIn;
        nic.upperLayerOut --> trafficFlowFilter.internetFilterGateIn;
        nic.upperLayerIn <-- gtpUser.pppGate;
        trafficFlowFilter.gtpUserGateOut --> gtpUser.trafficFlowFilterGate;
        gtpUser.udpOut --> udp.appIn++;
        gtpUser.udpIn <-- udp.appOut++;


        //#
        //# PPP interface to network layer connections
        //#    
        pppIf.phys <--> ppp;
        pppIf.upperLayerOut --> networkLayer.ifIn++;
        pppIf.upperLayerIn <-- networkLayer.ifOut++;


        gtpUserX2.lteStackOut --> nic.x2$i++;
        gtpUserX2.lteStackIn <-- nic.x2$o++;
        gtpUserX2.udpOut --> udp.appIn++;
        gtpUserX2.udpIn <-- udp.appOut++;

        for i=0..numUdpApps-1 {
            udpApp[i].udpOut --> udp.appIn++;
            udpApp[i].udpIn <-- udp.appOut++;
        }
        //# 
        //# Transport layer to network layer connections
        //#
        udp.ipOut --> networkLayer.transportIn++;
        udp.ipIn <-- networkLayer.transportOut++;
        //# 
        //# Mode Selector to network layer
        //#
        ModeSelector.trafficFlowFilterGate <--> trafficFlowFilter.modeSelectorGate;
}
