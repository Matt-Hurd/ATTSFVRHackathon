import flask
import wave
import struct
import sys
from bp import doAll
import numpy

def generateResponse(filename):
    w = wave.open("uploads/" + filename)
    count = w.getnframes()
    astr = w.readframes(w.getnframes())
    values = struct.unpack("%ih" % (w.getnframes()* w.getnchannels()), astr)
    values = [float(val) / pow(2, 15) for val in values]
    values = [values[b] for b in xrange(0, len(values), 2)]
    # envelope = test.getEnvelope(values).tolist()
    # bpass = bandpass(values)
    # bad = test.getVolumeInfo(envelope)
    bad = doAll(values)
    tooLow = 0.0
    tooLoud = 0.0
    for chunk in bad[0]:
    	tooLow += (chunk[1] - chunk[0])
    for chunk in bad[1]:
    	tooLoud += (chunk[1] - chunk[0])
    res = {'Quiet': tooLow / count * 100, 'Loud': tooLoud / count * 100}
    print res
    return res

if __name__ == '__main__':
	generateResponse("nohesitationwav.wav")