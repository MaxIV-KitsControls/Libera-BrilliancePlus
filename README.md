Libera Brilliance Plus
======================

Tango DeviceServer of the Libera Brilliance Plus.

Cloning this repository
-----------------------

This repository has a dependency on lib-itech-liberadebian in the Devices-COMMON project: you need to clone
 lib-itech-liberadebian and install the debian packages it contains on the system that will be used to compile the
 device server.


Building LiberaBrilliancePlus
-----------------------------

Prerequisites:

- compilation must happen on Ubuntu 10.04
- the debian packages in the repository lib-itech-liberadebian must be installed on the system that compiles the device server
- install QMake (the project is a QMake solution): sudo apt-get install qt4-qmake
- install ANT (manages all the build, including debian packages): sudo apt-get install ant

In order to produce the final debian package:

    cd src
    ant

If you want to produce a versioned build (the version is reported inthe final package):

    cd src
    ant -Dbuild-number=XX.XX

replace XX.XX with the version, e.g.:

    cd src
    ant -Dbuild.number=1.10




