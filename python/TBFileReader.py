#!/usr/bin/env python
#-----------------------------------------------------------------------------
# File: TBFileReader.py
# Description: File reader for HGC Test Beam 2016
# Created: 09-Apr-2016 Harrison B. Prosper
#-----------------------------------------------------------------------------
import os, sys
from string import find
from ROOT import *
from DataFormats.FWLite import Events, Handle
#-----------------------------------------------------------------------------
gSystem.Load("libFWCoreFWLite.so")
FWLiteEnabler.enable()
#-----------------------------------------------------------------------------
class TBFileReader:
    def __init__(self, filename, options, **optional):
        # cache input variables 
        self.filename = filename

        # check for required options
        self.options  = options
        if not self.options.has_key("label"):
            sys.exit("** need label(s) for getByLabel")

        if type(self.options["label"]) != type(()):
            sys.exit("** label format: (first, second); "\
                         "second could be an empty string")

        if not self.options.has_key("objects"):
            sys.exit("** need list of objects to be retrieved\n"\
                         "*** specified as a list of"\
                         " (key, object-name) 2-tuples")

        # create event buffer, get event iterator,
        # and create handles
        self.events = Events(filename)
        self.iter   = self.events.__iter__()
        self.handles= {}

        print "object list:"
        print "\t%-16s: %s" % ("key", "objects")
        for key, name in self.options["objects"]:
            h = Handle(name)
            print "\t%-16s: %s" % (key, name)
            self.handles[key] = h

        self.entry = 0
        self.buffer = {}

    def __del__(self):
        pass


    # get next event and cache requested objects
    def next(self):
        opt = self.options
        try:
            self.event = self.iter.next()
        except StopIteration:
            return False

        for (key, handle) in self.handles.items():
            if handle == None: continue
            self.event.getByLabel(opt["label"][0],
                                  opt["label"][1],
                                  handle)
            self.buffer[key] = handle.product()
        self.entry += 1
        return True

    # return object by key
    def __call__(self, key):
        try:
            return self.buffer[key]
        except:
            return None
#-----------------------------------------------------------------------------
def main():
    print "\n\t<== TBFileReader ==>\n"

    filename = "test_RecHits_OneLayer_TB.root"

    options = {"label"  : ("source", "simulator"),
               "objects": 
               [("rechits", "edm::SortedCollection<HGCalTBRecHit>"),
                ("sampling", "edm::SortedCollection<HGCSSSamplingSection>"),
                ("particles", "edm::SortedCollection<HGCSSGenParticle>")]}

    get = TBFileReader(filename, options)

    while get.next():
        rechits  = get("rechits")
        sampling = get("sampling")
        print len(rechits), rechits[0].energy()
#-----------------------------------------------------------------------------
if len(sys.argv) > 1: main()

