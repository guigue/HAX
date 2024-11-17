import pandas as pd
import numpy as np
from scipy.fft import fft

def v(file='test01.csv'):
    h = pd.read_csv(file,sep=',',header=0)

    adc = []
    t   = []
    for i in np.arange(h.adc.shape[0]):
        x = int(h.adc[i][4:],16)
        if x & (1<<15):
            x-=1<<16
            
        t.append( int(h.husec[i],16)/10)
        adc.append(x)

    return {'time':np.asarray(t), 'adc':np.asarray(adc)}

def toFile(d,file='test01.txt'):

    f = open(file,'w')

    f.write("Dt = 1\n")
    f.write("N = {0:d}\n\n".format(d['time'].shape[0]))

    for i in np.arange(d['time'].shape[0]):
        f.write("{0:5d}  {1:10.4f}\n".format(d['adc'][i],d['time'][i]))

    f.close()

    return
    
def getAmp(d,N=256):

    M = d['adc'].shape[0]

    A = []
    for i in np.arange(M-N):
        f = fft(d['adc'][i:i+N])
        A.append(np.abs(f[5])/(N//2))

    return np.asarray(A)
