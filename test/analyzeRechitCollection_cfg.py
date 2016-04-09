import FWCore.ParameterSet.Config as cms

process = cms.Process("Plot")
process.load("FWCore.MessageService.MessageLogger_cfi")

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(-1) )
process.MessageLogger.cerr.FwkReport.reportEvery = 10

process.source = cms.Source("PoolSource",
                            fileNames = 
                            cms.untracked.
                            vstring('file:test_RecHits_OneLayer_TB.root'
                                    )
                            )

process.test = cms.EDAnalyzer("TestRecHit",
                              HGCALTBRECHITS = 
                              cms.InputTag("source", "simulator")
                              )

process.p = cms.Path(process.test)
