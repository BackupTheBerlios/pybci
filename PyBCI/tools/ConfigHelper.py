
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






import ConfigParser

"""This module may help you to create a configuration file for the BCI.
Herefore you should use the function 'make_config'. See its documentation for usage."""

def make_config(outfile, sample_rate, numof_channels, mode, server = 'localhost', shape = 'None',
                security_mode = False, saving_mode = False, data_file = 'Nofile', format = 'binary',
                resolution = 0.1, returning_speed = 8, channels = 0,
                color_bg = 'white', color_trigger = 'black',
                size_window = (1000, 800)):
    """This is function to create a BCI configuration file automatically.
        You should call it with the following arguments:

    <outfile> : name of your configuration file
    
    <sample_rate> : Sample Rate

    <numof_channels> : Number of channels you want to get data from. Is not mandatory
                       the number of channels you are getting the data from.

    <mode> : Mode for giving signs in a seperate window.
             Possible modes are:
             'signs_enabled': OpenGL Python signing mode
             'signs_enabled_c: OpenGL C++ signing mode
             'signs_enabled_tk: Tkinter signing mode - this is the only signing mode including the possibility
             to show text signs yet, it may be pretty slow and partly unusuable though.
             'signs_disabled'.
             If you enable signs, you have to specify the <shape>.
             Then you have the possibility to give signs by calling BCI.trigger_signs(<time>).

    Optional arguments:

    <channels> : Labels of the <numof_channels> channels you want to get data
                 from as a list or tuple. The respective label is the number # in the Brain Recorder.
                 The default is 1 to <numof_channels>.


    <server> : Name of the server that is receiving Brain Recorder Data via TCP/IP-Port.
               Usually this is the same computer this software is running on - if this
               is the case, you may skip this arguments, otherwise it has to be specified.

    <resolution> : Resolution, that is declared in the Brain Recorder (usually either 0.1 (default)
                   or 10). This is used for conversion to microvolt.

    <returning_speed> : Returning speed of data arrays. Speed levels from -9 (very slow) to 9 (very fast)
                        are possible (default is 8), with possible exceptions of level -10
                        (slowest level that is possible) and 10 (as fast as possible).
                        If you are dependent on receiving the data as fast as possible,
                        you should choose a high level.

    <security_mode> : If True (False is default), a warning is raised if the number of returned
                       blocks is not equal to the read ones. That may be useful if you want
                       to be sure not to miss samples/data blocks or to avoid reading blocks
                       twice. Usually you will start the BCI first and start <security_mode>
                       not until you want to get the data (otherwise you probably will get a
                       warning message). See example.py for usage.

    <saving_mode> : If True (False is default), a <data_file> with format <format> is opened,
                    in which the data is written each time <get_data> is called.

    <data_file> : The file the data is written in when <saving_mode> is True.

    <format> : The file format the data is written in when <saving_mode> is True.
            Possible <format>s are 'plain'(ascii txt), 'pickle', 'binary' and 'mat' (MATLAB-file).

    <color_bg>: Background color of the signing window.

    <color_trigger> : Color of the sign, that is given.

    <size_window> : Size of the signing window as a tuple <width>, <heigth> in pixels.

    """


    config = ConfigParser.RawConfigParser()

    config.add_section('technics')
    config.add_section('visualization')
    config.add_section('security')
    config.add_section('data')

    config.set('technics', 'sample_rate', sample_rate)
    config.set('technics', 'numof_channels', numof_channels)
    config.set('technics', 'server', server)
    config.set('technics', 'resolution', resolution)
    config.set('technics', 'speed', returning_speed)
    if channels == 0:
        channels = range(numof_channels+1)[1:numof_channels+1]
    config.set('technics', 'channels', channels)

    config.set('visualization', 'mode', mode)
    config.set('visualization', 'color_bg', color_bg)
    config.set('visualization', 'color_trigger', color_trigger)
    config.set('visualization', 'size_window', size_window)

    config.set('security', 'security_mode', security_mode)

    config.set('data', 'saving_mode', saving_mode)
    config.set('data', 'file', data_file)
    config.set('data', 'format', format)

    config_file = open(outfile, 'w+')

    config.write(config_file)
