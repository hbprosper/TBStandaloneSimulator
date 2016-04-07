# TBStandaloneSimulator
Temporary home of test beam stand alone simulator (at least a piece of it!)

# Installation
  cd HGCal
  git clone https://github.com/hbprosper/TBStandaloneSimulator.git
  cd TBStandaloneSimulator
  cmsenv
  scram b
  scram b (do scram b a second time, if the first fails)

# Running producer
  cd test
  cmsRun produceRechitCollection_cfg.py

This should produce the fike test_RecHits_OneLayer_TB.root from the input sim file PFcal.root
