#!/usr/bin/python
import PyTango
import re
from fabric.api import run, local
import itertools
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

#Physical device to Test
device = PyTango.DeviceProxy("kitslab/dia/bpm-01")

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
	('T0inMask','boards.evrx2.rtc.sfp_2_connectors.t0.in_mask[0]', 51, 50),
	('T0inFunction','boards.evrx2.rtc.sfp_2_connectors.t0.in_function[0]', 255, 254),
	('T0Delay','boards.evrx2.rtc.sfp_2_connectors.t0.delay', 0, 1),
	('T0Duration','boards.evrx2.rtc.sfp_2_connectors.t0.duration', 100000000, 999999 ),
	('T1inMask','boards.evrx2.rtc.t1.in_mask[0]', 255, 254),
	('T1inFunction','boards.evrx2.rtc.t1.in_function[0]', 51, 50),
	('T1ID', 'boards.evrx2.rtc.connectors.t1.id', 21, 20),
	('T2inMask','boards.evrx2.rtc.t2.in_mask[0]', 255, 254),
	('T2inFunction','boards.evrx2.rtc.t2.in_function[0]', 80, 79),
	('T2ID','boards.evrx2.rtc.connectors.t2.id', 62, 61),
	('PMBufferSize','boards.raf3.postmortem.capacity', 524288, 500000),
	('PMOffset','boards.raf3.postmortem.offset', 0, 1),
	#('XHigh','boards.raf3.interlock.limits.position.max.x', 999936, 9999 ), #TODO, They fail on the ReadWrite Tests,
	#('XLow','boards.raf3.interlock.limits.position.min.x', -1000064, 9999),# TODO need to convert the tangovalue.
	#('YHigh','boards.raf3.interlock.limits.position.max.y', 999936, 9999), # TODO multiply it with *1e6
	#('YLow','boards.raf3.interlock.limits.position.min.y', -1000064, 9999) # TODO
	]

# AttributeName, Node, DefaultPropertyValue(used for testing and compare), RandomSetValue to write and compare
booleanAttr = [
	#('InterlockEnabled','boards.raf3.interlock.enabled', "true", "false"),
	('T1EdgeRising','boards.evrx2.rtc.connectors.t1.edge.rising', "false", True),
	#('T1EdgeFalling','boards.evrx2.rtc.connectors.t1.edge.falling', "true", False),
	#('T2EdgeRising','boards.evrx2.rtc.connectors.t2.edge.rising', "false", True),
	('T2EdgeFalling','boards.evrx2.rtc.connectors.t2.edge.falling', "true", False),
 	('PMNotified','boards.raf3.postmortem.capture', "true", "false")
 	]

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

# Read all attributes in order to find the Segmentation faults
def attributeTests():
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


def get_node_value(node):
    return run('/opt/libera/bin/libera-ireg {0}'.format(node)).split(' ')[1]


def write_node_value(node,value):
	run('/opt/libera/bin/libera-ireg {0}={1}'.format(node,value))

