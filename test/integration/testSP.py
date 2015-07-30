import PyTango
import numpy as np
import time
import re

libera = PyTango.DeviceProxy("kitslab/dia/bpm-01")
event = PyTango.DeviceProxy("LAB/TIM/EVG-01")


oldval = ""

def eventcallback(e):
    global oldval
    #print re.search(r'-?\d+',libera.read_attribute("XPosSP").value).group()
    val = str(libera.read_attribute("XPosSP").value)
    if oldval != str(val):
        oldval = val
        print event.read_attribute("Inj1Event").value, event.read_attribute("InjFreq").value, val

def testSPSignal():
    freqlist = np.arange(0.1,15.01,0.1)
    global oldval
    val = str(libera.read_attribute("XPosSP").value)
    for freq in freqlist:
        for ev in xrange(1,177):
            #print freq,ev
            event.write_attribute("InjFreq", freq)
            event.write_attribute("Inj1Event", ev)
            event.command_inout("Inject",1)
            time.sleep(0.01)
            #If Libera has event execute arg callback
            #libera.subscribe_event("XPosSP",PyTango.EventType.CHANGE_EVENT, arg)
            if oldval != str(val):
                oldval = val
                print event.read_attribute("Inj1Event").value, event.read_attribute("InjFreq").value, val