#!/usr/bin/python
import PyTango
import re
from fabric.api import run, local
import itertools
import numpy as np
import time

def enum(*sequential, **named):
    enums = dict(zip(sequential, range(len(sequential))), **named)
    reverse = dict((value, key) for key, value in enums.iteritems())
    enums['reverse_mapping'] = reverse
    return type('Enum', (), enums)

#Enum custom Types from some nodes
Source = enum(Off=0,External=1,Internal=2,Pulse=3,LXI=4,RTC=5)
DecoderSwitch = enum(off=0, on=1, debug=1)
Direction = enum(Input=0, Output=1)
ConnectionState = enum(low=0, high=1)
OutType = enum(Off=0, Trigger=1, T3=2, SFP=3)
MgtOut = enum(off=0,sfp_in=1,debug=2,connectors=3)
PMSource = enum(external=0,interlock=1,limits=2)
#myBoolean = enum(False=0,True=1)



#used for SP test
oldval = ""
#Physical device to Test
#device = PyTango.DeviceProxy("kitslab/dia/bpm-01")
device = PyTango.DeviceProxy("test/libera/1")
#event = PyTango.DeviceProxy("LAB/TIM/EVG-01")

# AttributeName, Node, EnumType, DefaultPropertyValue(used for testing and compare), RandomSetValue(used for test write)
enumAttrs = [
	('RtcDecoderSwitch','boards.evrx2.rtc.decoder_switch', DecoderSwitch , 1, 0 ),
	('McSource','boards.evrx2.triggers.mc.source', Source, 5, 1),
	('T0Direction','boards.evrx2.connectors.t0.direction', Direction, 1, 0),
	('T0OutputType','boards.evrx2.connectors.t0.out_type', OutType, 3, 1),
	('T0State','boards.evrx2.rtc.sfp_2_connectors.t0.state', ConnectionState, 1, 0),
	('T1Source','boards.evrx2.triggers.t1.source', Source, 5, 4),
	('T2Source','boards.evrx2.triggers.t2.source', Source, 5, 4),
	('MgtOut','boards.evrx2.rtc.mgt_out', MgtOut, 3, 2),
	('PMSource','boards.raf3.postmortem.source_select', PMSource,0,1)
	]

# AttributeName, Node, DefaultPropertyValue(used for testing and compare), RandomSetValue to write and compare
scalarAttrs = [
	('MCinMask','boards.evrx2.rtc.mc.in_mask[0]',256, 255), #TODO Exception when I write in ArrayNodes
	('MCinFunction','boards.evrx2.rtc.mc.in_function[0]',256, 255),
	('T0inMask','boards.evrx2.rtc.sfp_2_connectors.t0.in_mask[0]', 255, 50),
	('T0idOut','boards.evrx2.rtc.sfp_2_connectors.t0.in_function[0]', 144, 254),
	('T0Delay','boards.evrx2.rtc.sfp_2_connectors.t0.delay', 0, 1),
	('T0Duration','boards.evrx2.rtc.sfp_2_connectors.t0.duration', 100000000, 999999 ),
	('T1inMask','boards.evrx2.rtc.t1.in_mask[0]', 255, 254),
	('T1inFunction','boards.evrx2.rtc.t1.in_function[0]', 145, 50),
	('T1ID', 'boards.evrx2.rtc.connectors.t1.id', 21, 20),
	('T2inMask','boards.evrx2.rtc.t2.in_mask[0]', 255, 254),
	('T2inFunction','boards.evrx2.rtc.t2.in_function[0]', 161, 79), # property 81 for the commisioning else 80
	('T2ID','boards.evrx2.rtc.connectors.t2.id', 62, 61)
	#('PMBufferSize','boards.raf3.postmortem.capacity', 524288, 500000),
	#('PMOffset','boards.raf3.postmortem.offset', 0, 1),
	#('Gain','boards.raf3.conditioning.tuning.agc.power_level', -80, -60),
	#('SPThreshold','boards.raf3.single_pass.threshold', 200, 50),
	#('SPnBefore','boards.raf3.single_pass.n_before', 1, 10),
	#('SPnAfter','boards.raf3.single_pass.n_after', 100, 60),
	#('ExternalTriggerDelay','boards.raf3.local_timing.trigger_delay', 9800, 9700)
	#('XHigh','boards.raf3.interlock.limits.position.max.x', 999936, 9999 ), #TODO, They fail on the ReadWrite Tests,
	#('XLow','boards.raf3.interlock.limits.position.min.x', -1000064, 9999),# TODO need to convert the tangovalue.
	#('YHigh','boards.raf3.interlock.limits.position.max.y', 999936, 9999), # TODO multiply it with *1e6
	#('YLow','boards.raf3.interlock.limits.position.min.y', -1000064, 9999) # TODO
	]

# AttributeName, Node, DefaultPropertyValue(used for testing and compare), RandomSetValue to write and compare
booleanAttr = [
	('InterlockEnabled','boards.raf3.interlock.enabled', "true", False),
	('AutoSwitchingEnabled','boards.raf3.conditioning.switching', "false", True),
	('T1EdgeRising','boards.evrx2.rtc.connectors.t1.edge.rising', "false", True),
	('T1EdgeFalling','boards.evrx2.rtc.connectors.t1.edge.falling', "true", False),
	('T2EdgeRising','boards.evrx2.rtc.connectors.t2.edge.rising', "false", True),
	('T2EdgeFalling','boards.evrx2.rtc.connectors.t2.edge.falling', "true", False),
 	#('PMNotified','boards.raf3.postmortem.capture', "true", False),
	('AutoSwitchingEnabled','boards.raf3.conditioning.switching', "false", True),
	('AGCEnabled','boards.raf3.conditioning.tuning.agc.enabled', "false", True)
 	]


def readWriteScalar():
	for attr in scalarAttrs:
		#write value from tango
		device.write_attribute(attr[0],attr[3])
		#read_from tango
		tangovalue=device.read_attribute(attr[0]).value
		#Assert read/write Attributes
		assert (attr[3]==tangovalue),\
			"Error on Attribute: {0} WriteValue:{1} != TangoValue:{2}".format(attr[0],
																			 attr[3],
																			 tangovalue)
		time.sleep(3)
		print "MCPLL: {0}".format(device.read_attribute('MCPLLStatus').value)
		print 'Success!! on Attribute: {0} WriteValue:{1} == TangoValue:{2}'.format(attr[0],
																			 attr[3],
																			 tangovalue)
		# #Assert tangovalue=nodeValue
		# nodevalue=int(re.search(r'-?\d+', get_node_value(attr[1])).group())
		# #Assert tango/node Attributes
		# assert (nodevalue==int(tangovalue)),\
		# 	"Error on Attribute: {0} NodeValue:{1} != TangoValue:{2}".format(attr[0],
		# 																	 nodevalue,
		# 																	 tangovalue)




#Test if the nodevalue == tangoAttr then RandomSetValue and check again.
def enumTests():
	for node in enumAttrs:
		print "[[ Read Test ]]"
		#read value from the node
		nodevalue=get_node_value(node[1])
		#read value from tango
		tangovalue=device.read_attribute(node[0]).value
		#assert Values
		assert (nodevalue==node[2].reverse_mapping[tangovalue]),\
			"Error on Attribute: {0} NodeValue:{1} != TangoValue:{2}".format(node[0],
																			 nodevalue,
																			 node[2].reverse_mapping[tangovalue])
		print "[[ Write Test ]]"
		#write Values
		write_node_value(node[1],1)
		#read value from tango
		nodevalue=get_node_value(node[1])
		#Check
		assert (nodevalue==node[2].reverse_mapping[1]),\
			"Error on Attribute: {0} NodeValue:{1} != TangoValue:{2}".format(node[0],
																			 nodevalue,
																			 node[2].reverse_mapping[1])

#Test if the nodevalue == tangoAttr then RandomSetValue and check again.
def scalarTests():
	for node in scalarAttrs:
		print "[[ Read Test ]]"
		#read value from the node
		#extract any int from the string
		nodevalue=int(re.search(r'-?\d+', get_node_value(node[1])).group())
		#print type(nodevalue)
		#read value from tango
		tangovalue=device.read_attribute(node[0]).value
		#assert Values
		assert (nodevalue==tangovalue),\
			"Error on Attribute: {0} NodeValue:{1} != TangoValue:{2}".format(node[0],
																			 nodevalue,
																			 tangovalue)
		# #write Values
		# print "[[ Write Test ]]"
		# #write the value to the node THROUGH tango
		# device.write_attribute(node[0],node[3])
		# time.sleep(1) #TODO later write with wait
		# #read it from node
		# nodevalue = int(get_node_value(node[1]))
		# #compare new written with TANGO
		# assert (nodevalue==node[3]),\
		# 	"Error on Attribute: {0} SetValue:{1} != NodeValue:{2}".format(node[0],
		# 																	node[3],
		# 																	nodevalue)

#Test if the nodevalue == tangoAttr then RandomSetValue and check again.
def booleanTests():
	for node in booleanAttr:
		print "[[ Read Test ]]"
		#Get Value from node
		nodevalue=get_node_value(node[1])
		#Get Value from Tango
		tangovalue=str(device.read_attribute(node[0]).value).lower()
		#assert Values
		assert (nodevalue==tangovalue),\
			"Error on Attribute: {0} NodeValue:{1} != TangoValue:{2}".format(node[0],
																			 nodevalue,
																			 tangovalue)
		#write Values
		print "[[ Write Test ]]"
		#write the value to the node THROUGH tango
		device.write_attribute(node[0],bool(node[3]))
		time.sleep(1) #TODO later write with wait
		#read it from node
		nodevalue = str(get_node_value(node[1])).lower()
		#compare new written with TANGO
		assert (nodevalue==str(node[3]).lower()),\
			"Error on Attribute: {0} SetValue:{1} != TangoValue:{2}".format(node[0],
																			str(node[3]).lower(),
																			nodevalue)

#Test by connecting to physical device and test if the properties(values given in the pair) set correctly (prevents the accidental change of the important attributes)
def propertyTests():
	for node in booleanAttr:
		nodevalue=get_node_value(node[1])
		assert (nodevalue==node[2]),\
			"Error on Attribute: {0} NodeValue:{1} != Property Value:{2}".format(node[0],
																				 nodevalue,
																				 node[2])
	for node in enumAttrs:
		nodevalue=get_node_value(node[1])
		assert (nodevalue==node[2].reverse_mapping[node[3]]),\
			"Error on Attribute: {0} NodeValue:{1} != Property Value:{2}".format(node[0],
																				 nodevalue,
																				 node[2].reverse_mapping[node[3]])
	for node in scalarAttrs:
		nodevalue=int(re.search(r'-?\d+', get_node_value(node[1])).group())
		assert (nodevalue==node[2]),\
			"Error on Attribute: {0} NodeValue:{1} != Property Value:{2}".format(node[0],
																				 nodevalue,
																					 																		 																															 node[2])
#Write all attributes
def attributeWriteTests():
	for attr in itertools.chain(enumAttrs,scalarAttrs,booleanAttr):
		print device.write_attribute(attr[0],bool(node[3]))

# Read all attributes in order to find the Segmentation faults
def attributeReadTests():
	for attr in itertools.chain(enumAttrs,scalarAttrs,booleanAttr):
		print device.read_attribute(attr[0]).value

def allReadWriteTests():
	scalarTests()
	enumTests()
	booleanTests()

def resetNodeSettings():
	for attr in enumAttrs:
		write_node_value(attr[1],attr[4])
	for attr in scalarAttrs:
		write_node_value(attr[1],attr[3])
	for attr in booleanAttr:
		write_node_value(attr[1],str(attr[3]).lower())



def eventcallback(e):
	global oldval
	val=str(device.read_attribute("XPosSP").value)
	if oldval!=str(val):
		oldval = val
		print "Injection Id: {0}, Injection Frequency: {1}, XPosSP value: {2}, YPosSP value{3}, SumPosSP value{4}".format(
			event.read_attribute("Inj1Event").value,
			event.read_attribute("InjFreq").value,
			str(device.read_attribute("XPosSP").value),
			str(device.read_attribute("YPosSP").value),
			str(device.read_attribute("SumSP").value))

def testSPSignal():
    freqlist = np.arange(0.1,15.01,0.1)
    #global oldval
    val = str(device.read_attribute("XPosSP").value)
    for freq in freqlist:
        for ev in xrange(1,177):
            #print freq,ev
            event.write_attribute("InjFreq", freq)
            event.write_attribute("Inj1Event", ev)
            event.command_inout("Inject",1)
            time.sleep(0.01)
            #If Libera has event execute arg callback
            device.subscribe_event("XPosSP",PyTango.EventType.CHANGE_EVENT, eventcallback)
            #if oldval != str(val):
            #    oldval = val
            #    print event.read_attribute("Inj1Event").value, event.read_attribute("InjFreq").value, val


def get_node_value(node):
    return run('/opt/libera/bin/libera-ireg {0}'.format(node)).split(' ')[1]


def write_node_value(node,value):
	run('/opt/libera/bin/libera-ireg {0}={1}'.format(node,value))

