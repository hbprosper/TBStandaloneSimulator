#!/usr/bin/env python
#-----------------------------------------------------------------------------
# File: writePedestals.py
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
def nameonly(s):
    import posixpath
    return posixpath.splitext(posixpath.split(s)[1])[0]

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
#---------------------------------------------------------------------------
def main():
    argv = sys.argv[1:]
    argc = len(argv)
    if argc < 1:
        print '''
Usage:
   writePedestals.py <pedestal file> 
''' 
        sys.exit()


    filename = argv[0]
    if not os.path.exists(filename):
        sys.exit("** cannot open file %s" % filename)

    cellmap  = HGCCellMap()
    reader   = TBFileReader(filename)
    entries  = reader.entries()

    outfilename = replace(filename, ".root", "_Noise.root") 
    hfile= TFile(outfilename, "recreate") 
    vkey = vector("uint32_t")() 
    vadc = vector("uint16_t")() 
    tree = TTree("Pedestals", filename)
    tree.Branch("vkey", "vector<uint32_t>", vkey)    
    tree.Branch("vadc", "vector<uint16_t>", vadc)

    style  = TStyle("Pub", "Pub")
    style.SetOptTitle(0)
    style.SetOptDate(0)
    style.SetOptFile(0)
 
    canvas = TCanvas("fig_pedestals", "pedestals", 10, 10, 800, 400)
    canvas.Divide(2,1)
    hchan  = mkhist1("channels",  "channel", "", 128, 0, 128)
    hchan2 = mkhist1("c2", "channel", "", 128, 0, 128)
    hadc   = mkhist1("ADC distribution",   "ADC count", "", 200, 0, 2500)
    
    for entry in xrange(entries):
        reader.read(entry)
        skiroc = reader("SKIROC2DataFrame")
        if skiroc == None: sys.exit("\n** cannot find skiroc collection\n")

        vkey.clear()
        vadc.clear()

        for ii in xrange(skiroc.size()):
            digi   = SKIROC2DataFrame(skiroc[ii])
            nsamples = digi.samples()
            detid = digi.detid()
            u     = detid.iu()
            v     = detid.iv()
            layer = detid.layer()
            sen_u = detid.sensorIU()
            sen_v = detid.sensorIV()
            eid   = cellmap.uv2eid(layer, sen_u, sen_v, u, v)
            ski   = eid.first
            chan  = eid.second

            key   = detid.rawId()
            adc   = digi[0].adcHigh()

            binid = 64*(ski-1) + chan 

            hchan.Fill(binid+0.5, adc)
            hchan2.Fill(binid+0.5)
            hadc.Fill(adc)

            vkey.push_back(key)
            vadc.push_back(adc)

        hfile.cd()
        tree.Fill()

        if entry % 50 == 0:
            canvas.cd(1)
            gPad.SetLogy(0)
            hchan.Draw()

            canvas.cd(2)
            gPad.SetLogy()
            hadc.Draw()

            canvas.Update()

    canvas.cd(1)
    hchan.Divide(hchan2)
    style.SetOptStat("")
    gPad.SetLogy(0)
    hchan.Draw()

    canvas.cd(2)
    style.SetOptStat("mr") 
    gPad.SetLogy()
    hadc.Draw()

    canvas.Update()
    canvas.SaveAs(".png")

    tree.Write()
    hfile.Close()

    sleep(5)
#-----------------------------------------------------------------------------
if __name__ == "__main__": main()

