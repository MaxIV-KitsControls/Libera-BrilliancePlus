#!/usr/bin/python
import PyTango
from symbol import try_stmt
import unittest
import time
import exceptions
from fabric.api import run


# XXX Impose to run the test on the Libera platform
import subprocess

#env.hosts = ['194.47.254.61']

def host_type():
    run('/opt/libera/bin/libera-ireg -h boards.evrx2.connectors.t0.out_type')
