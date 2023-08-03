// Copyright 2019, John Zammit, All rights reserved.
// This file is subject to the terms and conditions defined in
// file 'LICENSE.md', which is part of this source code package.

import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.networklayer.ipv4.RoutingTableRecorder;
import inet.node.ethernet.Eth10G;
import inet.node.ethernet.Eth10M;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import lte.corenetwork.binder.LteBinder;
import lte.world.radio.LteChannelControl;
import lte.epc.PgwStandardSimplified;
import corenetwork.nodes.Ue;
import corenetwork.nodes.eNB;

network SingleCell
{
    parameters:
        int numUe = default(1);
        @display("i=block/network2;bgb=547,352;bgi=background/uom;bgl=2");
    types:
    submodules:
        channelControl: LteChannelControl {
            @display("p=46.153847,206.92308;is=s");
        }
        routingRecorder: RoutingTableRecorder {
            @display("p=46.92308,243.07693;is=s");
        }
        configurator: IPv4NetworkConfigurator {
            @display("p=46.92308,283.84616");
            config = xmldoc("config_ipv4.xml");
        }
        binder: LteBinder {
            @display("p=46.92308,323.07693;is=s");
        }
        server: StandardHost {
            @display("p=100,172;is=n;i=device/server");
        }
        router: Router {
            @display("p=172,100;i=device/smallrouter");
        }
        pgw: PgwStandardSimplified {
            nodeType = "PGW";
            @display("p=310,43;is=l");
        }
        eNodeB: eNB {
            @display("p=311,129;is=vl");
        }
        ue[numUe]: Ue {
            @display("p=450,195");
        }
    connections:
        server.pppg++ <--> Eth10G <--> router.pppg++;
        router.pppg++ <--> Eth10G <--> pgw.filterGate;
        pgw.pppg++ <--> Eth10G <--> eNodeB.ppp;
}
