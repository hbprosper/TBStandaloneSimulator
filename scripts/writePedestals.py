#!/usr/bin/env python
#-----------------------------------------------------------------------------
# File: writePedestals.py
# Description: Produce a file containing pedestal data in a format convenient
#              for adding noise to simulated HGC test beam events.
#              If the input file does not contain pedestal data for all layers
#              requested, randomly sample events from the input file and use
#              the pedestal data as noise for the missing layers.
# Created: 09-Apr-2016 Harrison B. Prosper
#-----------------------------------------------------------------------------
import os, sys, re
from time import sleep
from string import *
from ROOT import *
from HGCal.TBStandaloneSimulator.TBFileReader import *
#-----------------------------------------------------------------------------
gSystem.Load("libFWCoreFWLite.so")
FWLiteEnabler.enable()
#-----------------------------------------------------------------------------
def getarg(args, key, d):
    if args.has_key(key):
        return args[key]
    else:
        return d

def mkhist1(hname, xtitle, ytitle, nbins, xmin, xmax, **args):
    ymin   = getarg(args, 'ymin', None)
    ymax   = getarg(args, 'ymax', None)
    color  = getarg(args, 'color',   kBlack)
    lstyle = getarg(args, 'lstyle',  1)
    lwidth = getarg(args, 'lwidth',  1)
    ndivx  = getarg(args, 'ndivx',   505)
    ndivy  = getarg(args, 'ndivy',   510)

    h = TH1F(hname, "", nbins, xmin, xmax)		
    h.SetLineColor(color)
    h.SetLineStyle(lstyle)
    h.SetLineWidth(lwidth)

    h.SetMarkerSize(0.8)
    h.SetMarkerColor(color)
    h.SetMarkerStyle(20)

    h.GetXaxis().SetTitle(xtitle)
    h.GetXaxis().SetTitleOffset(1.2);
    h.GetXaxis().SetLimits(xmin, xmax)
    h.SetNdivisions(ndivx, "X")

    h.GetYaxis().SetTitle(ytitle)
    h.GetYaxis().SetTitleOffset(1.6)
    if ymin != None: h.SetMinimum(ymin)
    if ymax != None: h.SetMaximum(ymax)
    h.SetNdivisions(ndivy, "Y")
    return h

def checkForMissingLayers(checklayers, nlayers):
    layers = map(lambda x: x+1, range(nlayers))
    keys   = checklayers.keys()
    layers += keys
    layers.sort()
    missinglayers = []
    jj = 1
    while jj < len(layers):
        if layers[jj] == layers[jj-1]:
            jj += 2
            continue
        missinglayers.append(layers[jj-1])
        if jj == len(layers)-1:
            missinglayers.append(layers[jj])
        jj += 1
    return missinglayers
#---------------------------------------------------------------------------
def main():
    print "\n\n\t== writePedestals.py ==\n"

    argv = sys.argv[1:]
    argc = len(argv)
    if argc < 1:
        print '''
Usage:
   writePedestals.py <pedestal file> [layers=1]
''' 
        sys.exit()


    # get command line arguments
    filename = argv[0]
    if not os.path.exists(filename):
        sys.exit("** cannot open file %s" % filename)

    if argc > 1:
        nlayers = atoi(argv[1])
    else:
        nlayers = 1

    # need TRandom for random sampling of events
    random   = TRandom3()

    # need cell map to get list of cells per layer
    cellmap  = HGCCellMap()

    # read input pedestal file
    reader   = TBFileReader(filename)
    entries  = reader.entries()

    # create output noise file
    outfilename = replace(filename, ".root", "_Noise.root") 
    hfile= TFile(outfilename, "recreate")
    hfile.cd()

    # the noise file consists of a vector of rawIds and an
    # associated vector of ADC counts
    vkey = vector("uint32_t")() 
    vadc = vector("uint16_t")() 
    tree = TTree("Pedestals", filename)
    tree.Branch("vkey", "vector<uint32_t>", vkey)    
    tree.Branch("vadc", "vector<uint16_t>", vadc)

    # set up some plots
    gStyle.SetOptTitle(0)
    gStyle.SetOptDate(0)
    gStyle.SetOptFile(0)
 
    canvas = TCanvas("fig_pedestals", "pedestals", 10, 10, 800, 400)
    canvas.Divide(2,1)
    hchan  = mkhist1("channels",  "channel", "", 128, 0, 128)
    hchan2 = mkhist1("c2", "channel", "", 128, 0, 128)
    hadc   = mkhist1("ADC distribution",   "ADC count", "", 200, 0, 2500)
    

    # loop over pedestal events
    entries= min(2000, entries)
    a1 = 0.0
    a2 = 0.0
    nn = 0
    for entry in xrange(entries):
        # load pedestal data into memory
        reader.read(entry)
        skiroc = reader("SKIROC2DataFrame")
        if skiroc == None: sys.exit("\n** cannot find skiroc collection\n")

        vkey.clear()
        vadc.clear()

        checklayers = {}  # this will be used to determine missing layers
        keymap = {}       # this will be used to check for duplicate cells
        for ii in xrange(skiroc.size()):
            digi  = SKIROC2DataFrame(skiroc[ii])
            adc   = digi[0].adcHigh()
            a1 += adc
            a2 += adc*adc
            nn += 1
            detid = digi.detid()
            key   = detid.rawId()
            vkey.push_back(key)
            vadc.push_back(adc)
            layer = detid.layer()
            checklayers[layer] = 0
            if keymap.has_key(key):
                print "***this should not happen ***",
                print ii, key
                sys.exit()
            keymap[key] = layer

            # histogram channel data
            sen_u = detid.sensorIU()
            sen_v = detid.sensorIV()
            u     = detid.iu()
            v     = detid.iv()
            eid   = cellmap.uv2eid(layer, sen_u, sen_v, u, v)
            ski   = eid.first
            chan  = eid.second
            binid = 64*(ski-1) + chan 
            hchan.Fill(binid+0.5, adc)
            hchan2.Fill(binid+0.5)
            hadc.Fill(adc)


        # check that we have noise for all layers
        missinglayers = checkForMissingLayers(checklayers, nlayers)

        if entry == 0: 
            print "==> missing layers:", missinglayers

        # if we have missing layers, use noise
        # from randomly selected events to model noise
        # for those layers.
        for layer in missinglayers:
            ientry = random.Integer(entries-1)
            reader.read(ientry)
            skiroc = reader("SKIROC2DataFrame")
            if skiroc == None: sys.exit("\n** cannot find skiroc collection\n")
            data = {}
            for ii in xrange(skiroc.size()):
                digi  = SKIROC2DataFrame(skiroc[ii])
                adc   = digi[0].adcHigh()
                detid = digi.detid()
                u     = detid.iu()
                v     = detid.iv()
                data[(u,v)] = adc

            sen_u = 0  # hard code for now
            sen_v = 0
            cells = cellmap.cells(layer, sen_u, sen_v)
            for ii in xrange(cells.size()):
                cell = cells[ii]
                if data.has_key((cell.u, cell.v)):
                    adc = data[(cell.u, cell.v)]
                else:
                    adc = 0
                detid = HGCalTBDetId(layer, sen_u, sen_v, 
                                     cell.u, cell.v, cell.celltype)
                key   = detid.rawId()
                vkey.push_back(key)
                vadc.push_back(adc)

                if keymap.has_key(key):
                    print "*** duplicate key - this should not happen ***",
                    print ii, key
                    print layer, sen_u, sen_v, u, v, celltype
                    sys.exit()
                keymap[key] = layer

        hfile.cd()
        tree.Fill()

        if entry % 500 == 0:
            print entry, len(keymap)
            canvas.cd(1)
            gPad.SetLogy(0)
            hchan.Draw()

            canvas.cd(2)
            gPad.SetLogy()
            hadc.Draw()

            canvas.Update()

    from math import sqrt
    a1 /= nn
    a2 /= nn
    a2 = sqrt(a2-a1*a1)
    print "\n==> pedestal = %8.1f +/-%-8.1f ADC counts\n" % (a1, a2)

    canvas.cd(1)
    hchan.Divide(hchan2)
    gStyle.SetOptStat("")
    gPad.SetLogy(0)
    hchan.Draw()

    canvas.cd(2)
    gStyle.SetOptStat("mr") 
    gPad.SetLogy()
    hadc.Draw()

    canvas.Update()
    canvas.SaveAs(".png")

    tree.Write()
    hfile.Close()

    sleep(5)
#-----------------------------------------------------------------------------
if __name__ == "__main__": main()

