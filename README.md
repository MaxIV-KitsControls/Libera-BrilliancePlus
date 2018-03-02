Libera Brilliance Plus
======================

Tango DeviceServer of the Libera Brilliance Plus.

Cloning this repository
-----------------------

This repository has a dependency on itech proprietary library 

Building LiberaBrilliancePlus
-----------------------------

### Prerequisites
- ITech Libera Brilliance Plus Libraries 2.9 or 3.1
	libliberamci.so
	libliberaistd.so
	libliberaisig.so
	libliberainet.so
- Tango 8 or Tango 9
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
LIBERA_API=2.9 make
or 
LIBERA_API=3.1 make
```
