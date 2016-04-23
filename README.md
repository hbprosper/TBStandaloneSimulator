# TBStandaloneSimulator
Temporary home of test beam stand alone simulator (at least a piece of it!). This has been tested with CMSSW_8_0_1, slc6_amd64_gcc493, running within a CERNVM virtual machine on a mac. It should work on lxplus and cmslpc-sl6.

# Installation
```linux
  cd HGCal
  git clone https://github.com/hbprosper/TBStandaloneSimulator.git
  cd TBStandaloneSimulator
  cmsenv
  scram b
  scram b (do scram b a second time, if the first fails)
```
# Running producer
```linux
  cd test
  cmsRun produceSKIROCCollection_cfg.py
```
This reads the sim file 
```linux
digi_32GeV_electrons.root 
```
and copies its HGCSSRecoHit objects from to edm::Events, and creates and saves SKIROC dataframe objects to the same edm::Events. You should see the output
```linux
HGCal_digi_32GeV_electrons.root 
```
The list of input files to be read should be given in the file 
```linux
filelist
```
one file per line. 

