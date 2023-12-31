Copyright 2019, John Zammit, All rights reserved.
This file is subject to the terms and conditions defined in
file 'LICENSE.md', which is part of this source code package.

# Installation
This project can be compiled on any platform supported by INET and SimuLTE Frameworks.

## Prerequisites
You should have a:
- working OMNeT++ (v5.1) installation. (Download from http://omnetpp.org)
- working INET-Framework installation (v3.5). (Download from http://inet.omnetpp.org)
- working SimuLTE-framework installation (v1.0.0). (Download from http://simulte.com)

NOTE: OMNeT++ 5.0 paired with INET 3.4 should also work, but is untested.

Make sure your OMNeT++ installation works OK (e.g. try running the samples)
and it is in the path (to test, try the command `which nedtool`). On
Windows, open a console with the `mingwenv.cmd` command. The PATH and other
variables will be automatically adjusted for you. Use this console to compile
and run INET and SimuLTE.

Install and test INET according to the installation instructions found in the archive.
Be sure to check if the INET examples are running fine before continuing.

## Building from the IDE (recommended)
1. Extract the downloaded tarball next to the INET and SimuLTE directory
   (i.e. into your workspace directory, if you are using the IDE).
2. Start the IDE, and ensure that the 'inet' project is open and correctly built.
3. Import the project using: `File` > `Import` > `General` > `Existing projects into Workspace`.
   Then select the workspace dir as the root directory, and be sure NOT to check the
   `Copy projects into workspace` box. Click Finish.
4. Build the project.
5. To run an example from the IDE, select the simulation folder under 
   `sim`, and click `Run` from the toolbar.

## Building from the command line
1. Extract the downloaded tarball next to the INET and SimuLTE directory
2. Change to the extracted directory.
3. Type `make makefiles`. This should generate the makefiles.
4. Type `make` to build the simulte executable (debug version). Use `make MODE=release`
   to build release version.
5. You can run examples by changing into a directory under `sim`, and 
   executing `./run`