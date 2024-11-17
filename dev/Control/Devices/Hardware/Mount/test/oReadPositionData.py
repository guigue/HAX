# External methods
import sys, string, os, struct, glob
import numpy as np

class Position_Data(object):

    def __init__(self):
        self.Data   = {}

    def read(self,fname):
        self.fname=fname
        fmt = 'Qdddddddd'
        if os.path.exists(self.fname) :
            fd         = os.open(self.fname,os.O_RDONLY)
            nrec       = os.fstat(fd).st_size // struct.calcsize(fmt)
            self.Data.update( { 'time'       : np.array(np.empty([nrec], np.uint64))  } )
            self.Data.update( { 'JD'         : np.array(np.empty([nrec], np.float64)) } )
            self.Data.update( { 'sid_time'   : np.array(np.empty([nrec], np.float64)) } ) 
            self.Data.update( { 'elevation'  : np.array(np.empty([nrec], np.float64)) } )
            self.Data.update( { 'azimuth'    : np.array(np.empty([nrec], np.float64)) } )
            self.Data.update( { 'rascension' : np.array(np.empty([nrec], np.float64)) } )
            self.Data.update( { 'declination': np.array(np.empty([nrec], np.float64)) } )
            self.Data.update( { 'ra_rate'    : np.array(np.empty([nrec], np.float64)) } )
            self.Data.update( {'dec_rate'    : np.array(np.empty([nrec], np.float64)) } )
            
        for irec in range(nrec):
            one_record  = os.read(fd,struct.calcsize(fmt))
            ur          = struct.unpack(fmt,one_record)
            self.Data['time'][irec]       = ur[0]
            self.Data['JD'][irec]         = ur[1]
            self.Data['sid_time'][irec]   = ur[2]
            self.Data['elevation'][irec]  = ur[3]
            self.Data['azimuth'][irec]    = ur[4]
            self.Data['rascension'][irec] = ur[5]
            self.Data['declination'][irec]= ur[6]
            self.Data['ra_rate'][irec]    = ur[7]
            self.Data['dec_rate'][irec]   = ur[8]

        os.close(fd)

        return




