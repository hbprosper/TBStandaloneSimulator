# ---------------------------------------------------------------------------
# Read sim "digis" from sim root file and store in them in edm::Events 
# together with SKIROC2DataFrames made from the sim "digis" (HGCSSRecoHits)
# ---------------------------------------------------------------------------
import FWCore.ParameterSet.Config as cms

processName = "HGC"

process = cms.Process(processName)

process.load('FWCore.MessageService.MessageLogger_cfi')

process.source = cms.Source ("HGCSimDigiSource",
                             runNumber = cms.untracked.int32(101),
                             maxEvents = cms.untracked.int32(-1),
                             fileNames = 
                             cms.untracked.vstring("file:digi_32GeV_electrons.root")
                             )

process.MessageLogger.cerr.FwkReport.reportEvery = 10

process.out = cms.OutputModule("PoolOutputModule",
                               fileName = cms.untracked.string
                               ("HGCal_digi_32GeV_electrons.root")
                               )

process.outpath = cms.EndPath(process.out)
