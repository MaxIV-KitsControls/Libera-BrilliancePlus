Libera Brilliance Plus
======================

Tango DeviceServer of the Libera Brilliance Plus.

Cloning this repository
-----------------------

This repository has a dependency on itech proprietary library 


Building LiberaBrilliancePlus
-----------------------------

### Prerequisites
- ITech Libera Brilliance Plus Libraries 2.8
	libliberamci.so.2.8
	libliberaistd.so.2.8
	libliberaisig.so.2.8
	libliberainet.so.2.8
- Tango 8
	libtango.so.8
	liblog4tango.so.5
	libomniORB4.so.1
	libomniDynamic4.so.1
	libomnithread.so.3
	libzmq.so.3
	libCOS4.so.1


### Compilation
To compile
``` shell
cd src
make clean
make
```

### Packaging
To make the debian package, just call the following command from the project directory
``` shell
dpkg-buildpackage -us -uc
```
### At Solaris (Branch cosylab)
Prerequisites:

- compilation must happen on Ubuntu 10.04
- the itech development debian packages in the repository must be installed on the system that compiles the device server
- install QMake (the project is a QMake solution): sudo apt-get install qt4-qmake

In order to produce the final debian package:

    cd src
    ant

If you want to produce a versioned build (the version is reported inthe final package):

    cd src
    ant -Dbuild-number=XX.XX

replace XX.XX with the version, e.g.:

    cd src
    ant -Dbuild.number=1.10




