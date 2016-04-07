# TBStandaloneSimulator
Temporary home of test beam stand alone simulator (at least a piece of it!)

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
  cmsRun produceRechitCollection_cfg.py
```
This reads the sim file PFcal.root, copies its objects to edm::Events, and creates HGCalTBRecHits.
