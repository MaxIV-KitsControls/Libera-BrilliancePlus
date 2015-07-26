LiberaBrilliancePlus Test
==============
***

Integration Test for LiberaBrilliancePlus, used for testing directly the libera device kitslab/dia/bpm-01 in the lab.

Requirement
-----------

Software:

- [PyTango][pytango] >= 8.1.1
- [Fabric] [fabric] => 1.3

[pytango]: https://pypi.python.org/pypi/PyTango
[fabric]: http://docs.fabfile.org/en/latest/tutorial.html

Testing
-------

For testing, run (in the ﻿test/integration directory):

    $ ﻿fab testType -H liberaIp -u liberaSshUsername -p liberaPassword

Examples:

   * Test if the properties set correctly during the init of the device:(In case fail,restart Tango Device and try again)

        $ ﻿fab ﻿propertyTests -H liberaIp -u liberaSshUsername -p liberaPassword

   * Reset Libera properties: (Add a random value(different than the default) to the libera settings)

        $ ﻿fab ﻿resetNodeSettings -H liberaIp -u liberaSshUsername -p liberaPassword

   * Check which Attribute causes segmentation fault. (happens when a given node in AddScalar function is wrong)

        $ ﻿fab attributeTests -H liberaIp -u liberaSshUsername -p liberaPassword

   * Read - Write test (assert node with tango values then write a new value to node and compare again with Tango (Always restart TangoDevice before perform).

        $ ﻿fab ﻿allReadWriteTests -H liberaIp -u liberaSshUsername -p liberaPassword


Known Issues
------------

* Exception when write in array Type nodes i.e boards.evrx2.rtc.mc.in_mask[0]


Contact
-------

Vasileios Martos: vasileios.martos@maxlab.lu.se