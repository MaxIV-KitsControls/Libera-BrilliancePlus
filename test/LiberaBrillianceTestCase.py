#!/usr/bin/python
import PyTango
from symbol import try_stmt
import unittest
import time
import exceptions

# XXX Impose to run the test on the Libera platform
import subprocess

class LiberaBrillianceTestCase(unittest.TestCase):

    # TANGO_DEVICES = ["test/itech/libera-spe4", "test/itech/libera-spe5","test/itech/libera-spe6", "test/itech/libera-spe7"]
    TANGO_DEVICES = ["test/libera/1"]

    ATTRIBUTES = {   'InterlockZNotified': '.interlock.status.il_status.y',
                     'InterlockXNotifier': '.interlock.status.il_status.x',
                     'InterlockAttnNotified': '.interlock.status.il_status.attenuator',
                     'InterlockADCPreFilterNotified': '.interlock.status.il_status.adc_overflow',
                     'InterlockADCPostFilterNotified': '.interlock.status.il_status.adc_overflow_filtered',
                     'InterlockEnabled': '.interlock.enabled',
                     'InterlockOverflowThreshold': '.interlock.limits.overflow.threshold',
                     'InterlockOverflowDuration': '.interlock.limits.overflow.duration',
                     'InterlockGainDependentEnabled': '.interlock.gain_dependent.enabled',
                     'InterlockGainDependentThreshold': '.interlock.gain_dependent.threshold',
                     'ZHigh': '.interlock.limits.position.max.y',
                     'XHigh': '.interlock.limits.position.max.x',
                     'ZLow': '.interlock.limits.position.min.y',
                     'XLow': '.interlock.limits.position.min.x',
                     'Kx': '.signal_processing.position.Kx',
                     'Ky': '.signal_processing.position.Ky',
                     'XOffset': '.signal_processing.position.off_x',
                     'YOffset': '.signal_processing.position.off_y',
                     'Fan1Speed': 'fans.left_',
                     'Fan2Speed': 'fans.right_'
                     }

    def setUp(self):
        self._tests = ["testTriggerReset"]
        self.li = "/opt/libera/bin/libera-ireg"
        self.devices = []
        for device in LiberaBrillianceTestCase.TANGO_DEVICES :
            self.devices += [PyTango.DeviceProxy(device)]


    def testTriggerReset(self):

        for device in self.devices :
            "when :"
            trigger_read1=device.read_attribute('TriggerCounter').value
            device.command_inout('ResetTrigger')
            trigger_read2=device.read_attribute('TriggerCounter').value

            "then :"
            self.assertTrue(trigger_read2<trigger_read1, "**ERROR** trigger counter before reset : %s  is not smaller then trigger counter after : %s" % (trigger_read1, trigger_read2) )

    def testReadWriteBooleanAttribute(self):
        # Absolute random values
        values = [True, False ]
        attributes = [
                'DDEnabled', 'ExternalTriggerEnabled', 
                'SAEnabled', 'ADCEnabled', 
                'InterlockEnabled', 
                'InterlockGainDependentEnabled',
                'AutoSwitchingEnabled', 'ExternalSwitching', 
                'CompensateTune', 'UseLiberaSAData', 
                ]
        for device in self.devices :
            for attribute in attributes :
                for expected in values :
                    self._write_read_test(device, attribute, expected)

    def testReadWriteUnsignedIntegerAttribute(self):
        # Absolute random values
        values = [10, 25, 0, 38, 10000]
        attributes = [
                'overflow_threshold',
                ]
        for device in self.devices :
            for attribute in attributes :
                for expected in values :
                    self._write_read_test(device, attribute, expected)

    def testReadWriteDoubleAttribute(self):
        # Absolute random valuess
        values = [-10.0, -25.45, 0.0, 38.56, 10000.0]
        # XXX Bad practice to test several attribute in one unit test
        # TODO Check if unittest accept the parametrized test
        attributes = [
                'XLow',
                'XHigh',
                'ZLow',
                'ZHigh',
                'XOffset',
                'ZOffset']

        for device in self.devices :
            for attribute in attributes :
                for expected in values :
                    self._write_read_test(device, attribute, expected)


    def testWrongBufferSize(self):
        sizes = [1, 8193, 1000000]
        attributes = ["DDBufferSize", "SAStatNumSamples"]
        for device in self.devices :
            for attribute in attributes :
                for wrong in sizes :
                    "when:"
                    self.assertRaises(PyTango.DevFailed, lambda : device.write_attribute(attribute, wrong) )


    def testSignalDDSize(self):

        signals = ["VaDD", "VbDD", "VcDD", "VdDD"]
        sizes = [10, 55, 100, 5400, 7300, 8192]

        for device in self.devices :
            for expected in sizes :
                "when:"
                device.DDBufferSize = expected
                #XXX We should expect to exit the write request with the value taken account in the low level
                time.sleep(1)
                actual = device.DDBufferSize

                "then:"
                self.assertEquals(expected, actual, "DDBufferSize is not set correctly : %s (expected : %s)" % (actual, expected))

                "then:"
                for signal in signals :
                    value = device.read_attribute(signal).value
                    actual = len(value)
                    self.assertEquals(expected, actual, "The signal %s has not the expected length : %s (expected %s)" % (signal, actual, expected))

    def testSignalPMSize(self):

        signals = ["VaPM", "VbPM", "VcPM", "VdPM"]
        sizes = [10, 55, 100, 5400, 7300, 8192]

        for device in self.devices :
            for expected in sizes :
                "when:"
                device.SAStatNumSamples = expected
                #XXX We should expect to exit the write request with the value taken account in the low level
                time.sleep(1)
                actual = device.SAStatNumSamples

                "then:"
                self.assertEquals(expected, actual, "SAStatNumSamples is not set correctly : %s (expected : %s)" % (actual, expected))

                "then:"
                for signal in signals :
                    value = device.read_attribute(signal).value
                    actual = len(value)
                    self.assertEquals(expected, actual, "The signal %s has not the expected length : %s (expected %s)" % (signal, actual, expected))


    def testPlatformTemperature(self):

        for device in self.devices :
            libera_board=device.get_property('LiberaBoard')['LiberaBoard'][0]
            libera_ip=device.get_property('LiberaIpAddr')['LiberaIpAddr'][0]
            attributes = (
                ("Temp1", 'boards.'+ libera_board + '.sensors.ID_2.value'),
                ("Temp2", 'boards.icb0.sensors.ID_1.value'),
                ("Temp3", 'boards.evrx2.sensors.ID_6.value'))
                
            for attribute in attributes:
                li_args = [self.li, attribute[1], "-h", libera_ip, '-P']

                out=subprocess.Popen(li_args,stdout=subprocess.PIPE)
                node_value, err = out.communicate()
                #XXX This test has a part of random : the temperature is not expected to keep the same between the request with the low level library and the device !!!
                expected = float(node_value.split(":")[1].strip())
                #read node value and attribute value and check consistency
                actual = device.read_attribute(attribute[0]).value

                ratio = actual/expected
                self.assertTrue( (ratio<1.1 and ratio>.9), "The temperature %s is not the expected ratio: %s (expected %s)" % (attribute[0], actual, expected))


    def testPlatformFanSpeed(self):

        for device in self.devices :
            libera_board=device.get_property('LiberaBoard')['LiberaBoard'][0]
            libera_ip=device.get_property('LiberaIpAddr')['LiberaIpAddr'][0]
            attributes = (
                ("Fan1Speed", LiberaBrillianceTestCase.ATTRIBUTES["Fan1Speed"]),
                ("Fan2Speed", LiberaBrillianceTestCase.ATTRIBUTES["Fan2Speed"]))

            for attribute in attributes:
                expected = min( float(self.read_ireg_p(libera_ip, attribute[1]+"_rear")),
                                float(self.read_ireg_p(libera_ip, attribute[1]+"_front")),
                                float(self.read_ireg_p(libera_ip, attribute[1]+"_middle")) )

                #XXX This test has a part of random : the fan speed is not expected to keep the same between the request with the low level library and the device !!!
                #read node value and attribute value and check consistency
                actual = device.read_attribute(attribute[0]).value

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
            #actual = device.read_attribute(attribute)

            "then:"
            self.assertEquals(expected, actual, "%s is not set correctly : %s (expected : %s)" % (attribute, actual, expected))

#UTILITIES
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
        #print "cmd %s" % (li_args)
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
    #suiteFew.addTest(LiberaBrillianceTestCase("testPlatformFanSpeed"))
    #suiteFew.addTest(LiberaBrillianceTestCase("testPlatformTemperature"))
    suiteFew.addTest(LiberaBrillianceTestCase("testReadWriteBooleanAttribute"))
    suiteFew.addTest(LiberaBrillianceTestCase("testReadWriteUnsignedIntegerAttribute"))
    suiteFew.addTest(LiberaBrillianceTestCase("testReadWriteDoubleAttribute"))
    suiteFew.addTest(LiberaBrillianceTestCase("testWrongBufferSize"))
    suiteFew.addTest(LiberaBrillianceTestCase("testSignalPMSize"))
    suiteFew.addTest(LiberaBrillianceTestCase("testSignalDDSize"))
    unittest.TextTestRunner(verbosity=4).run(suiteFew)
    #unittest.TextTestRunner(verbosity=2).run(unittest.makeSuite(LiberaBrillianceTestCase))
