# ---------------------------------------------------------------------------
# Read sim "digis" from sim root file and store in them in edm::Events 
# together with SKIROC2DataFrames made from the sim "digis" (HGCSSRecoHits)
# ---------------------------------------------------------------------------
import FWCore.ParameterSet.Config as cms

processName = "HGC"

process = cms.Process(processName)

process.load('FWCore.MessageService.MessageLogger_cfi')

# read file names from filelist
from string import strip
filelist = map(lambda x: "file:%s" % x, 
              map(strip, open("filelist").readlines()))

process.source = cms.Source ("HGCSimDigiSource",
                             runNumber  = cms.untracked.int32(101),
                             maxEvents  = cms.untracked.int32(-1),
                             minADCCount= cms.untracked.int32(1),
                             fileNames  = cms.untracked.vstring(filelist)
                             )

process.MessageLogger.cerr.FwkReport.reportEvery = 100

process.out = cms.OutputModule("PoolOutputModule",
                               fileName = cms.untracked.string
                               ("HGCal_digi_120GeV_protons.root")
                               )

process.outpath = cms.EndPath(process.out)
