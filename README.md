Copyright 2019, John Zammit, All rights reserved.
This file is subject to the terms and conditions defined in
file 'LICENSE.md', which is part of this source code package.

# mvv-d2d-simulte: A simulation for multi-view video streaming over LTE-A D2D networks using SimulTE
To help understand the properties of the proposed architecture in [Efficient Transmission of Live
Multi-View Video in LTE-A Cellular Networks using D2D Cooperation](https://ieeexplore.ieee.org/document/9140665)
under the controlled and repeatable settings, the design was implemented using the OMNeT++ Discrete Event
Simulator. The INET and SimuLTE frameworks were used to conduct the evaluation.

## Installation
Refer to [installation instructions](./INSTALL.md).
The current main/head version requires:
OMNeT++ 5.1, INET 3.5.0, SimuLTE v1.0.0
Tested on Fedora 25 Workstation

## Miscellaneous
In SimuLTE, the PDCP-RRC module is the connection point between NIC and LTE-IP modules. 
It assigns and creates a Connection Identifier (CID) per connection. The functionality 
of creation of separate CIDs was disabled in our simulations due to a misalignment 
discovered between RLC and MAC layers at the UE, caused when multiple simultaneous 
uplink packets are sent simultaneously. To remove this functionality:

1. Navigate to the directory `simulte/src/stack/pdcp_rrc/layer/`
2. Open `LtePdcpRrc.cc` in any text editor
3. Replace lines 118 and 119 with the following: `if (false)`

This issue has been reported to the authors of [SimuLTE](https://github.com/inet-framework/simulte).