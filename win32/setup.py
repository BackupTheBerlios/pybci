
"""Copyright (c) <2009> <Benedikt Zoefel>

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE."""




#encoding: latin1

"""
setup.py file for PyBCI
"""

import ConfigParser
from distutils.core import setup

config = ConfigParser.RawConfigParser({'windows_path' : 'C:\\WINDOWS\system32'})

config.read('setup.cfg')

if config.has_section('system'):
  pass
else:
  config.add_section('system')

setup(  name="PyBCI",
        version = "0.4",
        author = "Benedikt Zoefel",
        description = "Python Brain Computer Interface for reading EEG data online, using Brain Vision Recorder",
        packages = ["PyBCI", "PyBCI.bci_source", "PyBCI.tools"],
        package_data = {'PyBCI.bci_source': ['_bci_source.pyd']},
        data_files = [(config.get('system', 'windows_path'), ['inpout32.dll', 'glut32.dll']),
                      ('Lib/site-packages/PyBCI/bci_source/data/images', ['PyBCI\\data\\images\\R.bmp', 'PyBCI\\data\\images\\L.bmp',
                                             'PyBCI\\data\\images\\sr.bmp', 'PyBCI\\data\\images\\sl.bmp',
                                             'PyBCI\\data\\images\\grey.bmp'])]
        )
