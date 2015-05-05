#!/bin/sh
set -e

if [ "x$1" != "x" ] && [ "x$2" != "x" ]; then
    sed "s/TANGO_HOST_PLACEHOLDER/$1/g" /usr/local/bin/LiberaBrilliancePlus/LiberaBrilliancePlus.sh > /etc/init.d/LiberaBrilliancePlus
    sed -i "s/TANGO_INSTANCE_PLACEHOLDER/$2/g" /etc/init.d/LiberaBrilliancePlus
else
    echo "Run the following script to configure the startup script:"
    echo "    sudo /usr/local/bin/LiberaBrilliancePlus/configure.sh TANGOHOST TANGOINSTANCE"
    echo "e.g.:"
    echo "    sudo /usr/local/bin/LiberaBrilliancePlus/configure.sh 10.0.0.1:10000 1"
    echo "Then start the service with:"
    echo "    service \"LiberaBrilliancePlus\" start"
fi
