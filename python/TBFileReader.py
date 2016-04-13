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
stripaway= re.compile("HGCSS|HGCal")
#-----------------------------------------------------------------------------
class TBFileReader:
    def __init__(self, filename, **optional):
        
        if not os.path.exists(filename):
            sys.exit("** file not found: %s" % filename)

        # cache input variables 
        self.filename = filename

        # create options by scanning file
        print "scanning file %s ..." % filename
        os.system("edmDumpEventContent %s >& .dumpevent" % filename)
        records = open(".dumpevent").readlines()
        record = "%-48s %-10s %-10s %-7s" % ("Type","Module","Label","Process")
        print record
        print "-"*78
        for ii, record in enumerate(records):
            t = split(record)
            if t[0] != "Type":   continue
            if t[1] != "Module": continue
            records = records[ii+2:]
            break

        objects = {}
        for ii, record in enumerate(records):
            record = simplify.sub(">", strip(record))
            record = replace(record, '"', '')
            t = split(record)
            if len(t) == 3: t.insert(-1, "")
            record = "%-48s %-10s %-10s %-7s" % tuple(t)
            print record

            name = extract.findall(record)[0]
            name = stripaway.sub("",name)
            objects[name] = (t[0], t[1], t[2])

        # create event buffer, get event iterator,
        # and create handles
        self.events = Events(filename)
        self.iter   = self.events.__iter__()
        self.event  = self.iter.next()
        self.numberOfEvents = self.event.size()
        print "\nNumber of events: %d" % self.numberOfEvents
        self.handles= {}

        print "\nKey/Type List:"
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

    filename = "test_RecHits_OneLayer_TB.root"

    reader = TBFileReader(filename)

    index = 0
    while reader.read(index):
        rechits  = reader("TBRecHit")
        print "%d\tnumber of rechits: %d" % (index, rechits.size())
        for ii in xrange(rechits.size()):
            energy = rechits[ii].energy()
            cellid = rechits[ii].id()
            l = cellid.layer()
            u = cellid.iu()
            v = cellid.iv()
            print "\t%5d: cell(%3d,%3d,%3d): %8.3f GeV" % (ii, l, u ,v, energy)
        print
        index += 1
        break
#-----------------------------------------------------------------------------
if __name__ == "__main__": main()

