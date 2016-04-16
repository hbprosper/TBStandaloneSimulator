#!/usr/bin/env python
#-----------------------------------------------------------------------------
# File: TBFileReader.py
# Description: File reader for HGC Test Beam 2016
# Created: 09-Apr-2016 Harrison B. Prosper
#-----------------------------------------------------------------------------
import os, sys, re
from string import *
from ROOT import *
from DataFormats.FWLite import Events, Handle
#-----------------------------------------------------------------------------
gSystem.Load("libFWCoreFWLite.so")
FWLiteEnabler.enable()

simplify = re.compile(",edm::Strict.*[>] [>]")
extract  = re.compile("(?<=[<]).*(?=[>])")
dequote  = re.compile("\"")
#-----------------------------------------------------------------------------
class TBFileReader:
    def __init__(self, filename, **optional):
        
        if not os.path.exists(filename):
            sys.exit("** file not found: %s" % filename)

        # cache input variables 
        self.filename = filename

        # create options by scanning file
        print "reading file %s ..." % filename
        os.system("edmDumpEventContent %s >& .dumpevent" % filename)
        records = open(".dumpevent").readlines()
        for ii, record in enumerate(records):
            if record[:4] != "----":   continue
            records = records[ii+1:]
            break

        objects = {}
        for ii, record in enumerate(records):
            record = simplify.sub(">", strip(record))
            t = split(record)
            name = extract.findall(record)[0]
            objects[name] = (t[0], dequote.sub("",t[1]), dequote.sub("",t[2]))

        # create event buffer, get event iterator,
        # and create handles
        self.events = Events(filename)
        self.iter   = self.events.__iter__()
        self.event  = self.iter.next()
        self.numberOfEvents = self.event.size()
        print "\n  Number of events: %d" % self.numberOfEvents
        self.handles= {}

        print "  %-20s %s" % ("Key", "Type")
        print "-"*78
        keys = objects.keys()
        keys.sort()
        for key in keys:
            edmtype, label1, label2 = objects[key]
            h = Handle(edmtype)
            self.handles[key] = h
            print "  %-20s %s" % (key, edmtype)

        self.entry   = 0
        self.buffer  = {}
        self.objects = objects
        self.keys    = keys

    def __del__(self):
        pass

    # read event at specified index in TFile and 
    # cache requested objects
    def read(self, index):
        if index < 0: return False
        if index > self.numberOfEvents-1: return False
  
        self.event.to(index)

        for key in self.keys:
            handle = self.handles[key]
            if handle == None: continue
            edmtype, module, label = self.objects[key]
            self.event.getByLabel(module, label, handle)
            self.buffer[key] = handle.product()
        self.entry += 1
        return True

    # get next event and cache requested objects
    def next(self):
        try:
            self.event = self.iter.next()
        except StopIteration:
            return False

        for key in self.keys:
            handle = self.handles[key]
            if handle == None: continue
            edmtype, module, label = self.objects[key]
            self.event.getByLabel(module, label, handle)
            self.buffer[key] = handle.product()
        self.entry += 1
        return True

    # return object by key
    def __call__(self, key):
        try:
            return self.buffer[key]
        except:
            return None

    def entries(self):
        return self.numberOfEvents

    def names(self):
        return self.keys()

    def __len__(self):
        return self.entries()

#-----------------------------------------------------------------------------
def main():
    print "\n\t<== TBFileReader ==>\n"

    filename = "HGCal_digi_32GeV_electrons.root"

    reader = TBFileReader(filename)

    index = 0
    while reader.read(index):
        rechits  = reader("HGCSSRecoHit")
        if rechits == None:
            sys.exit("\n** cannot find collection\n")
        print "%d\tnumber of rechits: %d" % (index, rechits.size())
        for ii in xrange(rechits.size()):
            energy = rechits[ii].energy()
            adc    = rechits[ii].adcCounts()
            x      = rechits[ii].get_x()
            y      = rechits[ii].get_y()
            z      = rechits[ii].get_z()
            print "\t%5d: cell(%6.2f,%6.2f,%6.2f): %8.0f\t%8.3f MeV" % \
                (ii, x, y, z, adc, energy)
        print
        index += 1
        break
#-----------------------------------------------------------------------------
if __name__ == "__main__": main()

