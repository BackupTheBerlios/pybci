
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




"""
This is an example how to use the functions of the EyemoveCorrectur module
to estimate and remove artefacts caused by eye movements.
"""

import numpy as N
import time
from PyBCI import BCI
from PyBCI.tools.EyemoveCorrector import *


# We'll assume that you have already created a configuration file
example_bci = BCI('BCI.cfg')


# There are four condition we need for an estimation of eye movement artefacts:
conditions = ['resting', 'horizontal', 'vertical', 'blinks']

trials = 20
sec_per_trial = 2

# Make sure that the following matches with your config file.
numof_channels = 5
sample_rate = 500

condition_means = N.zeros((len(conditions), numof_channels))

for num, condition in zip(range(len(conditions)), conditions):
    condition_data = N.zeros((trials, numof_channels, sample_rate*sec_per_trial))
    for trial in range(trials): # For each trial in each condition...
        
        time.sleep(1) # ...we'll wait a second...
        
        # ...then give a sign by means of a triangle for the beginning of the trial, that is, the data
        # collection...
        example_bci.trigger_sign('triangle', 0.5, 300)

        # ...and store the data, separated for each trial.
        condition_data[trial] = example_bci.get_data(sec_per_trial)

    print 'Condition', condition, 'done.'

    # At last, we'll take the average of the respective condition, but keep
    # the channel structure.
    condition_data = N.mean(condition_data, axis=2) # average over the samples
    condition_data = N.mean(condition_data, axis=0) # average over the trials

    # The result is an array with the structure [condition][channels], with the
    # channel values averaged over all samples and trials of the respect condition.
    condition_means[num] = condition_data   

# The next thing we'll do is to estimate the eye movement impact array with the
# collected data:

impact = estimate_impact(condition_means[0], condition_means[1:4])

# That's it! The only thing we'll have to do now is to remove the calculated impact
# by calling the respective function. To finish this script, we'll have
# just a short trial.

for test in range(5):
    data = example_bci.get_data(1)
    print 'Original data:', data
    
    corr_data = remove_impact(impact, data)
    print 'Corrected data:', corr_data
    

example_bci.end_bci()    







    
        












