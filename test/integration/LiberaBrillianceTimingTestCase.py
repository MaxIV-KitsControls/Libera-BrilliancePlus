#!/usr/bin/python
import PyTango
from symbol import try_stmt
import unittest
import time
import exceptions
from fabric.api import *


# XXX Impose to run the test on the Libera platform
import subprocess

#env.hosts = ['194.47.254.61']

class LiberaBrillianceTestCase(unittest.TestCase):

    # TANGO_DEVICES = ["test/itech/libera-spe4", "test/itech/libera-spe5","test/itech/libera-spe6", "test/itech/libera-spe7"]
    TANGO_DEVICES = {"kitslab/dia/bpm-01":"kitslab-01-raf3"}
    #Tango Server executable Location on the remote Host
    execlocation = "/home/dojo/Libera-Test/LiberaBrilliancePlus"
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
        ('InterlockEnabled','boards.raf3.interlock.enabled', "true", "false"),
        ('T1EdgeRising','boards.evrx2.rtc.connectors.t1.edge.rising', "false", True),
        ('T1EdgeFalling','boards.evrx2.rtc.connectors.t1.edge.falling', "true", False),
        ('T2EdgeRising','boards.evrx2.rtc.connectors.t2.edge.rising', "false", True),
        ('T2EdgeFalling','boards.evrx2.rtc.connectors.t2.edge.falling', "true", False),
        ('PMNotified','boards.raf3.postmortem.capture', "true", "false")
        ]


    def setUp(self):
        # Reset Node Values
        resetNodeSettings()
        # remote run Server on the Host
        run('{0} {1}'.format())
        self.devices = []
        for device in LiberaBrillianceTestCase.TANGO_DEVICES :
            self.devices += [PyTango.DeviceProxy(device)]



    def propertyTests(self):

        for node in booleanAttr:
            nodevalue=get_node_value(node[1])
            self.assertEquals(nodevalue,node[2],
                              "Error on Attribute: {0} NodeValue:{1} != Property Value:{2}".format(node[0],
                                                                                                   nodevalue,
                                                                                                   node[2]))
        for node in enumAttrs:
            nodevalue=get_node_value(node[1])
            self.assertEquals(nodevalue, node[2].reverse_mapping[node[3]],
                              "Error on Attribute: {0} NodeValue:{1} != Property Value:{2}".format(node[0],
                                                                                                   nodevalue,
                                                                                                   node[2].reverse_mapping[node[3]]))
        for node in scalarAttrs:
            nodevalue=int(re.search(r'-?\d+', get_node_value(node[1])).group())
            self.assertEquals(nodevalue, node[2],
                              "Error on Attribute: {0} NodeValue:{1} != Property Value:{2}".format(node[0],
                                                                                                   nodevalue,
                                                                                                   node[2]))

            for attribute in attributes:
                print libera_ip, attribute[1]
                #expected = min( float(self.read_ireg_p(libera_ip, attribute[1])))
                expected = self.read_ireg_p(libera_ip,'boards.evrx2.connectors.t0.out_type')

                print 'EXPECTED == ',expected
                #XXX This test has a part of random : the fan speed is not expected to keep the same between the request with the low level library and the device !!!
                #read node value and attribute value and check consistency
                actual = device.read_attribute(attribute[0]).value

                print "ACTUAL == ", actual
                return

                ratio = actual/expected
                self.assertTrue( (ratio<1.1 and ratio>.9), "The temperature %s is not the expected ratio: %s (expected %s)" % (attribute[0], actual, expected))

#TEST UTILITIES
    def _write_read_test(self, device, attribute, expected ):
            "when:"
            print "%s %s %s" % (device.name(), attribute, expected)
            #setattr(device, attribute, expected)
            device.write_attribute(attribute, expected)
            #XXX We should expect to exit the write request with the value taken account in the low level
            #time.sleep(1)
            actual = getattr(device, attribute)
            print '----==={0}'.format(actual)
            #actual = device.read_attribute(attribute)

            "then:"
            self.assertEquals(expected, actual, "%s is not set correctly : %s (expected : %s)" % (attribute, actual, expected))

#UTILITIES

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

    def read_ireg(self, libera_ip, node):
        return self.ireg(libera_ip, node, '')

    def read_ireg_p(self, libera_ip, node):
        return self.ireg(libera_ip, node, '-P')

    def write_ireg(self, libera_ip, node, value):
        return self.ireg(libera_ip, "%s=%s" % (node,value), '' )

    def write_ireg_p(self, libera_ip, node, value):
        return self.ireg(libera_ip, "%s=%s" % (node,value), '-P' )

    def ireg(self, libera_ip, cmd, opts):
        result = ""

        li_args = [self.li, cmd, "-h", libera_ip, opts]
        print "cmd %s" % (li_args)
        out = subprocess.Popen(li_args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        node_value, err = out.communicate()


        if ( len(err) == 0 ):
            values = node_value.split(":")
            if( len(values) >= 2  ) :
                result = values[1].strip()
                #??? WHICH CASE ?
                if "gt" in result:
                    result=result.split("_")[1].strip()
        else:
            print "node_value %s, err %s" % (node_value, err)
        

        return result

    def num(s):
        try:
            return int(s)
        except exceptions.ValueError:
            return float(s)


if __name__ == '__main__':
    suiteFew = unittest.TestSuite()
    #suiteFew.addTest(LiberaBrillianceTestCase("testReadWriteUnsignedIntegerAttribute"))
    # unittest.TextTestRunner(verbosity=4).run(suiteFew)
    unittest.TextTestRunner(verbosity=4).run(unittest.makeSuite(LiberaBrillianceTestCase))
