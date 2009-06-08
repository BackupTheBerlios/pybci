
"""
Module to estimate the impact of eye movements and blinks
and to remove this impact from EEG data.
"""

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

__docformat__ = 'restructuredtext'


import numpy as N
import sys

def estimate_impact(baseline, artefact):
    """
    With this function, an impact array of eye movement artefacts is
    estimated. The difference between two conditions, namely a baseline
    or resting condition (without eye movements) and an artefact condition
    are used for calculating the impact of the latter condition on all
    channels of the baseline condition. It is assumed that the difference
    for one channel is composed of the influence of all the other channels
    (and herewith also of the EOG channels).

    The two conditions are reflected in the two arguments for this
    function, with the following structure:

    :param baseline: Baseline condition as a one-dimensional numpy array,
                    containing the mean samples of the baseline for
                    [numof_channels].

    :param artefact: Artefact conditions as a two-dimensional numpy array,
                     containing mean samples for each condition in a
                     separate array.

                     Example: The conditions
                     
                     1. horizontal eye movements
                     2. vertical eye movements
                     3. eye blinks
                     
                     would thus result in a numpy array
                     [[numof_channels(hor)],[numof_channels(ver)],
                     [numof_channels(blink)]].
    
    .. note:: It is necessary for a valid estimation that the channels
              in the two data arrays are both equal and in the same order.

    :returns: Another numpy array with the estimated impact.
              If you now take the dot product of this vector with a measured
              EEG signal vector, (f.i., calling :func:`remove_impact` you'll
              get a vector without any activity that had caused exactly
              this difference between the two conditions.
    """

    if artefact.ndim == 1:
        if baseline.shape[0] != artefact.shape[0]:
            print 'Error: The number of channels has be equal in both conditions.'
            sys.exit(-1)

        # this is just to avoid one-dimensionality when creating the pseudo-inverse    
        rest_cond = N.array([baseline, baseline]).T        
        art_cond = N.array([artefact, artefact]).T

    else:
        if baseline.shape[0] != artefact.shape[1]:
            print 'Error: The number of channels has be equal in both conditions.'
            sys.exit(-1)
            
        rest_cond = N.zeros((artefact.shape[1], artefact.shape[0]))
        for l in range(artefact.shape[1]):
            rest_cond[l] = baseline
        rest_cond = rest_cond.T
        art_cond = artefact.T

    diff = art_cond-rest_cond

    pinv = N.linalg.pinv(diff) # create pseudo-inverse

    impact = N.eye(pinv.shape[1])-N.dot(diff, pinv)

    return impact


def remove_impact(impact, signal):
    """
    Use this function to get a signal array without any activity, that had
    previously caused the difference between the two conditions used for
    calculating the impact array (usually evaluated by
    :func:`estimate_impact`). In other words, activity is removed from the
    data, that correlates with the difference of the two conditions, that
    had 'produced' the impact array.

    :param impact: Square matrix as a numpy array
                   [numof_channels][numof_channels]

    :param signal: 'Raw' EEG signal vector to be artefact-corrected using
                    the impact array. This vector has to be a numpy array
                    [channels][samples],
                    as the data is returned when calling the
                    :func:`data functions <BCI.get_data>` of the
                    :class:`BCI class <BCI>`.
                   
    .. note:: It is necessary for a valid estimation that the channels
              in the signal arrays are both equal and in the same order
              as in the data arrays used for estimating the impact array.

    :returns: Signal numpy array, containing the corrected data values.
    """

    return N.dot(impact, signal)




    






    
